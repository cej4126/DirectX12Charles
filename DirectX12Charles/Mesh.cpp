#include "Mesh.h"
#include "imgui/imgui.h"
#include <unordered_map>

Mesh::Mesh(Graphics &gfx, std::vector<std::shared_ptr<Bindable>> bindPtrs, int indicesCount, int &MaterialIndex)
   :
   MaterialIndex(MaterialIndex)
{
   for (auto &pb : bindPtrs)
   {
      AddBind(std::move(pb));
   }

   std::unique_ptr < Transform > trans = std::make_unique<Transform>(gfx, *this);
   trans->setIndices(0, indicesCount);
   AddBind(std::move(trans));
}

void Mesh::Draw(Graphics &gfx, FXMMATRIX acculatedTransform, int index) const noexcept
{
   XMStoreFloat4x4(&transform, acculatedTransform);
   DrawFunction::Draw(gfx, index);
}

Node::Node(int id, const std::string &name, std::vector<Mesh * > meshPtrs, const XMMATRIX &transformIn) noexcept
   :
   id(id),
   meshPtrs(std::move(meshPtrs)),
   name(name)
{
   XMStoreFloat4x4(&transform, transformIn);
   XMStoreFloat4x4(&appliedTransform, XMMatrixIdentity());
}

void Node::Draw(Graphics &gfx, FXMMATRIX accumulatedTransform, int &index)
{
   const auto built =
      XMLoadFloat4x4(&appliedTransform) *
      XMLoadFloat4x4(&transform) *
      accumulatedTransform;

   for (const auto pm : meshPtrs)
   {
      pm->Draw(gfx, built, index);
      ++index;
   }

   for (const auto &pc : childPtrs)
   {
      pc->Draw(gfx, built, index);
   }
}

void Node::AddChild(std::unique_ptr<Node> pChild) noexcept
{
   childPtrs.push_back(std::move(pChild));
}

void Node::ShowTree(Node *&pSelectedNode) const noexcept
{
   // If there is not selection node, selectedId to an invalid id
   const int selectedId = (pSelectedNode == nullptr) ? -1 : pSelectedNode->GetId();
   // build up flags for current node
   const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow
      | ((GetId() == selectedId) ? ImGuiTreeNodeFlags_Selected : 0)
      | ((childPtrs.size() == 0) ? ImGuiTreeNodeFlags_Leaf : 0);

   // Render this node
   const auto expanded = ImGui::TreeNodeEx((void *)(intptr_t)GetId(), node_flags, name.c_str());

   // Processing for selecting node
   if (ImGui::IsItemClicked())
   {
      pSelectedNode = const_cast<Node *>(this);
   }

   // Recursive rendering of open node's children
   if (expanded)
   {
      for (const auto &pChild : childPtrs)
      {
         pChild->ShowTree(pSelectedNode);
      }
      ImGui::TreePop();
   }
}

void Node::SetAppliedTransform(FXMMATRIX transform) noexcept
{
   XMStoreFloat4x4(&appliedTransform, transform);
}

int Node::GetId() const noexcept
{
   return id;
}

class ModelWindow
{
public:

   void Show(const char *windowName, const Node &root) noexcept
   {
      windowName = windowName ? windowName : "Model";

      // need an ints to track node indices and selected node
      int nodeIndexTracker = 0;

      if (ImGui::Begin(windowName))
      {
         ImGui::Columns(2, nullptr, true);
         root.ShowTree(pSelectedNode);

         ImGui::NextColumn();
         if (pSelectedNode != nullptr)
         {
            auto &tranform = transforms[pSelectedNode->GetId()];
            ImGui::Text("Orientation");
            ImGui::SliderAngle("Roll", &tranform.roll, -180.0f, 180.0f);
            ImGui::SliderAngle("Pitch", &tranform.pitch, -180.0f, 180.0f);
            ImGui::SliderAngle("Yaw", &tranform.yaw, -180.0f, 180.0f);

            ImGui::Text("Position");
            ImGui::SliderFloat("X", &tranform.x, -20.0f, 20.0f);
            ImGui::SliderFloat("Y", &tranform.y, -20.0f, 20.0f);
            ImGui::SliderFloat("Z", &tranform.z, -20.0f, 20.0f);
         }
      }
      ImGui::End();
   }

   XMMATRIX GetTransform() const noexcept
   {
      assert(pSelectedNode != nullptr);
      const auto &transform = transforms.at(pSelectedNode->GetId());
      return XMMatrixRotationRollPitchYaw(transform.pitch, transform.yaw, transform.roll) *
         XMMatrixTranslation(transform.x, transform.y, transform.z);
   }

   Node *GetSelectedNode() const noexcept
   {
      return pSelectedNode;
   }

private:
   static constexpr float PI = 3.14159265f;
   Node *pSelectedNode;
   struct TransformParameters
   {
      float roll = 0.0f;
      float pitch = 0.0f;
      float yaw = 0.0f;
      float x = 0.0f;
      float y = 0.0f;
      float z = 0.0f;
   };
   std::unordered_map<int, TransformParameters> transforms;
};

Model::Model(Graphics &gfx, const std::string fileName, ID3D12Resource *lightView, int &MaterialIndex, int &index)
   :
   gfx(gfx),
   lightView(lightView),
   pWindow(std::make_unique<ModelWindow>())
{
   Assimp::Importer imp;

   const auto pScene = imp.ReadFile(fileName.c_str(),
      aiProcess_Triangulate |
      aiProcess_JoinIdenticalVertices |
      aiProcess_ConvertToLeftHanded |
      aiProcess_GenNormals);

   m_materialIndex = MaterialIndex;

   for (size_t i = 0; i < pScene->mNumMeshes; i++)
   {
      meshPtrs.push_back(ParseMesh(*pScene->mMeshes[i], pScene->mMaterials));
      ++index;
      MaterialIndex = m_materialIndex;
   }

   int nextId = 0;
   pRoot = ParseNode(nextId, *pScene->mRootNode);
}

Model::~Model() noexcept
{}

std::unique_ptr<Node> Model::ParseNode(int &nextId, const aiNode &node) noexcept
{
   const auto transform = XMMatrixTranspose(XMLoadFloat4x4(
      reinterpret_cast<const XMFLOAT4X4 *>(&node.mTransformation)));

   std::vector<Mesh *> curMeshPtrs;
   curMeshPtrs.reserve(node.mNumMeshes);
   for (size_t i = 0; i < node.mNumMeshes; i++)
   {
      const auto meshIndex = node.mMeshes[i];
      curMeshPtrs.push_back(meshPtrs.at(meshIndex).get());
   }

   auto pNode = std::make_unique<Node>(nextId++, node.mName.C_Str(), std::move(curMeshPtrs), transform);
   for (size_t i = 0; i < node.mNumChildren; i++)
   {
      pNode->AddChild(ParseNode(nextId, *node.mChildren[i]));
   }
   return pNode;
}

void Model::FirstCommand()
{
   if (m_materialIndex != -1)
   {
      gfx.CopyMaterialConstant(m_materialIndex, m_material);
   }
}

std::unique_ptr<Mesh> Model::ParseMesh(const aiMesh &mesh, const aiMaterial *const *pMaterials)
{
   using hw3dexp::VertexLayout;
   hw3dexp::VertexBuffer vbuf(std::move(
      VertexLayout{}
      .Append(VertexLayout::Position3D)
      .Append(VertexLayout::Normal)
      .Append(VertexLayout::Texture2D)
   ));

   for (unsigned int i = 0; i < mesh.mNumVertices; i++)
   {
      vbuf.EmplaceBack(
         *reinterpret_cast<XMFLOAT3 *>(&mesh.mVertices[i]),
         *reinterpret_cast<XMFLOAT3 *>(&mesh.mNormals[i]),
         *reinterpret_cast<XMFLOAT2 *>(&mesh.mTextureCoords[0][i])
      );
   }

   std::vector<unsigned short> indices;
   indices.reserve((size_t)mesh.mNumFaces * 3);
   for (unsigned int i = 0; i < mesh.mNumFaces; i++)
   {
      const auto &face = mesh.mFaces[i];
      assert(face.mNumIndices == 3);
      indices.push_back(face.mIndices[0]);
      indices.push_back(face.mIndices[1]);
      indices.push_back(face.mIndices[2]);
   }

   std::shared_ptr<ModelObject> object = std::make_shared<ModelObject>(gfx);

   bool specular = false;
   float shininess = 35.0f;
   if (mesh.mMaterialIndex >= 0)
   {
      using namespace std::string_literals;
      auto &material = *pMaterials[mesh.mMaterialIndex];
      const auto path = "..\\..\\DirectX12Charles\\Models\\nano_textured\\"s;
      aiString texFileName;
      material.GetTexture(aiTextureType_DIFFUSE, 0, &texFileName);
      object->CreateTexture(Surface::FromFile(path + texFileName.C_Str()), 0);

      if (material.GetTexture(aiTextureType_SPECULAR, 0, &texFileName) == aiReturn_SUCCESS)
      {
         object->CreateTexture(Surface::FromFile(path + texFileName.C_Str()), 1);
         specular = true;
      }
      else
      {
         material.Get(AI_MATKEY_SHININESS, shininess);
      }
   }

   object->LoadVerticesBuffer(vbuf);

   object->LoadIndicesBuffer(indices);
   if (specular)
   {
      object->CreateShader(L"ModelVS.cso", L"ModelSpecularPS.cso");
   }
   else
   {
      object->CreateShader(L"ModelVS.cso", L"ModelPS.cso");
      // copy at FirstCommand
      m_material.specularInensity = 0.8f;
      m_material.specularPower = shininess;
   }

   XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
   object->CreateConstant(position);

   // Create Root Signature after constants
   object->CreateRootSignature();

   object->CreatePipelineState(vbuf.GetLayout().GetD3DLayout(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

   object->SetLightView(lightView);

   bindablePtrs.push_back(std::move(object));
   return std::make_unique<Mesh>(gfx, std::move(bindablePtrs), (UINT)indices.size(), m_materialIndex);
}

void Model::Draw(Graphics &gfx, int &index) const
{
   if (auto node = pWindow->GetSelectedNode())
   {
      node->SetAppliedTransform(pWindow->GetTransform());
   }
   pRoot->Draw(gfx, XMMatrixIdentity(), index);
   ++index;
}

void Model::ShowWindow(const char *windowName) noexcept
{
   pWindow->Show(windowName, *pRoot);
}
