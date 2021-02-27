#include "DrawMesh.h"
#include "imgui/imgui.h"
#include <unordered_map>

DrawMesh::DrawMesh(Graphics &gfx, int index, std::vector<std::shared_ptr<Bind::Bindable>> bindPtrs, int indicesCount, int &MaterialIndex)
   :
   MaterialIndex(MaterialIndex)
{
   for (auto &pb : bindPtrs)
   {
      AddBind(std::move(pb));
   }

   std::unique_ptr < Transform > trans = std::make_unique<Transform>(gfx, *this, 0, -1);
   trans->setIndices(index, 0, indicesCount);
   AddBind(std::move(trans));
}

void DrawMesh::Draw(Graphics &gfx, FXMMATRIX acculatedTransform) const noexcept
{
   XMStoreFloat4x4(&transform, acculatedTransform);
   DrawFunction::Draw(gfx);
}

Node::Node(int id, const std::string &name, std::vector<DrawMesh * > DrawMeshPtrs, const XMMATRIX &transformIn) noexcept
   :
   id(id),
   MeshPtrs(std::move(DrawMeshPtrs)),
   name(name)
{
   XMStoreFloat4x4(&transform, transformIn);
   XMStoreFloat4x4(&appliedTransform, XMMatrixIdentity());
}

void Node::Draw(Graphics &gfx, FXMMATRIX accumulatedTransform)
{
   const auto built =
      XMLoadFloat4x4(&appliedTransform) *
      XMLoadFloat4x4(&transform) *
      accumulatedTransform;

   for (const auto pm : MeshPtrs)
   {
      pm->Draw(gfx, built);
   }

   for (const auto &pc : childPtrs)
   {
      pc->Draw(gfx, built);
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

Model::Model(Graphics &gfx, int &index, const std::string fileName, ID3D12Resource *lightView, int &MaterialIndex)
   :
   gfx(gfx),
   filename(fileName),
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
      MeshPtrs.push_back(ParseMesh(index, *pScene->mMeshes[i], pScene->mMaterials));
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

   std::vector<DrawMesh *> curMeshPtrs;
   curMeshPtrs.reserve(node.mNumMeshes);
   for (size_t i = 0; i < node.mNumMeshes; i++)
   {
      const auto MeshIndex = node.mMeshes[i];
      curMeshPtrs.push_back(MeshPtrs.at(MeshIndex).get());
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

std::unique_ptr<DrawMesh> Model::ParseMesh(int index, const aiMesh &mesh, const aiMaterial *const *pMaterials)
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

   // create the tag and path
   std::size_t pos = filename.find_last_of("/\\");
   std::string path = filename.substr(0, pos) + std::string("\\");
   pos = path.find_last_of("/\\");
   std::string tag = path.substr(pos + 1);
   aiString diffuseName;
   aiString specularName;

   bool diffuse = false;
   bool specular = false;
   auto &material = *pMaterials[mesh.mMaterialIndex];
   if (mesh.mMaterialIndex >= 0)
   {
      if (material.GetTexture(aiTextureType_DIFFUSE, 0, &diffuseName) == aiReturn_SUCCESS)
      {
         diffuse = true;
         tag += std::string("#") + diffuseName.C_Str();
      }
   }

   if (mesh.mMaterialIndex >= 0)
   {
      if (material.GetTexture(aiTextureType_SPECULAR, 0, &specularName) == aiReturn_SUCCESS)
      {
         specular = true;
         tag += std::string("#") + specularName.C_Str();
      }
   }

   auto object = ModelObject::Resolve(gfx, tag);

   if (!object->isInitialized())
   {
      object->setInitialized();

      float shininess = 35.0f;
      if (diffuse)
      {
         object->CreateTexture(Surface::FromFile(path + diffuseName.C_Str()), 0);
      }

      object->LoadVerticesBuffer(vbuf);
      object->LoadIndicesBuffer(indices);

      if (specular)
      {
         object->CreateTexture(Surface::FromFile(path + specularName.C_Str()), 1);
      }
      else
      {
         material.Get(AI_MATKEY_SHININESS, shininess);
      }

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

      //XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
      //object->CreateConstant(position);

      // Create Root Signature after constants
      object->CreateRootSignature(false, false, false);

      object->CreatePipelineState(vbuf.GetLayout().GetD3DLayout(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

      object->SetLightView(lightView);
   }

   bindablePtrs.push_back(std::move(object));
   return std::make_unique<DrawMesh>(gfx, index, std::move(bindablePtrs), (UINT)indices.size(), m_materialIndex);
}

void Model::Draw(Graphics &gfx) const
{
   if (auto node = pWindow->GetSelectedNode())
   {
      node->SetAppliedTransform(pWindow->GetTransform());
   }
   pRoot->Draw(gfx, XMMatrixIdentity());
}

void Model::ShowWindow(const char *windowName) noexcept
{
   pWindow->Show(windowName, *pRoot);
}
