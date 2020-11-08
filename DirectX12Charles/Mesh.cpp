#include "Mesh.h"
#include "imgui/imgui.h"
#include <unordered_map>

Mesh::Mesh(Graphics &gfx, std::vector<std::unique_ptr<Bindable>> bindPtrs, int indicesCount, int MaterialIndex)
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

Node::Node(const std::string &name, std::vector<Mesh * > meshPtrs, const DirectX::XMMATRIX &transformIn) noexcept
   :
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

void Node::ShowTree(int &nodeIndexTracked, std::optional<int> &selectedIndex, Node *&pSelectedNode) const noexcept
{
   //std::optional<int> mySelectedIndex = *selectedIndex;
   // nodeIndex serves as the uid for gui tree nodes, incremented throughout recursion
   const int currentNodeIndex = nodeIndexTracked;
   nodeIndexTracked++;

   // build up flags for current node
   const auto node_flags = ImGuiTreeNodeFlags_OpenOnArrow
      | ((currentNodeIndex == selectedIndex.value_or(-1)) ? ImGuiTreeNodeFlags_Selected : 0)
      | ((childPtrs.size() == 0) ? ImGuiTreeNodeFlags_Leaf : 0);

   // Render this node
   const auto expanded = ImGui::TreeNodeEx((void *)(intptr_t)currentNodeIndex, node_flags, name.c_str());

   // Processing for selecting node
   if (ImGui::IsItemClicked())
   {
      selectedIndex = currentNodeIndex;
      pSelectedNode = const_cast<Node *>(this);
   }

   // Recursive rendering of open node's children
   if (expanded)
   {
      for (const auto &pChild : childPtrs)
      {
         pChild->ShowTree(nodeIndexTracked, selectedIndex, pSelectedNode);
      }
      ImGui::TreePop();
   }
}

void Node::SetAppliedTransform(FXMMATRIX transform) noexcept
{
   XMStoreFloat4x4(&appliedTransform, transform);
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
         root.ShowTree(nodeIndexTracker, selectedIndex, pSelectedNode);

         ImGui::NextColumn();
         if (pSelectedNode != nullptr)
         {
            auto &tranform = transforms[*selectedIndex];
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

   DirectX::XMMATRIX GetTransform() const noexcept
   {
      const auto &transform = transforms.at(*selectedIndex);
      return DirectX::XMMatrixRotationRollPitchYaw(transform.pitch, transform.yaw, transform.roll) *
         DirectX::XMMatrixTranslation(transform.x, transform.y, transform.z);
   }

   Node *GetSelectedNode() const noexcept
   {
      return pSelectedNode;
   }

private:
   static constexpr float PI = 3.14159265f;
   std::optional<int> selectedIndex;
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

Model::Model(Graphics &gfx, const std::string fileName, ID3D12Resource *lightView, int MaterialIndex, int &index)
   :
   gfx(gfx),
   MaterialIndex(MaterialIndex),
   lightView(lightView),
   pWindow(std::make_unique<ModelWindow>())
{
   Assimp::Importer imp;

   //const auto pScene = imp.ReadFile(fileName.c_str(),
   //   aiProcess_Triangulate |
   //   aiProcess_JoinIdenticalVertices);
   const auto pScene = imp.ReadFile(fileName.c_str(),
      aiProcess_Triangulate |
      aiProcess_JoinIdenticalVertices |
      aiProcess_ConvertToLeftHanded |
      aiProcess_GenNormals);

   for (size_t i = 0; i < pScene->mNumMeshes; i++)
   {
      meshPtrs.push_back(ParseMesh(*pScene->mMeshes[i]));
      ++index;
   }
   material.materialColor = XMFLOAT3(1.0f, 0.4f, 0.2f);

   pRoot = ParseNode(*pScene->mRootNode);
}

Model::~Model() noexcept
{}

std::unique_ptr<Node> Model::ParseNode(const aiNode &node) noexcept
{
   const auto transform = XMMatrixTranspose(DirectX::XMLoadFloat4x4(
      reinterpret_cast<const XMFLOAT4X4 *>(&node.mTransformation)));

   std::vector<Mesh *> curMeshPtrs;
   curMeshPtrs.reserve(node.mNumMeshes);
   for (size_t i = 0; i < node.mNumMeshes; i++)
   {
      const auto meshIndex = node.mMeshes[i];
      curMeshPtrs.push_back(meshPtrs.at(meshIndex).get());
   }

   auto pNode = std::make_unique<Node>(node.mName.C_Str(), std::move(curMeshPtrs), transform);
   for (size_t i = 0; i < node.mNumChildren; i++)
   {
      pNode->AddChild(ParseNode(*node.mChildren[i]));
   }
   return pNode;
}

void Model::FirstCommand()
{
   if (MaterialIndex != -1)
   {
      gfx.CopyMaterialConstant(MaterialIndex, material);
   }
}

std::unique_ptr<Mesh> Model::ParseMesh(const aiMesh &mesh)
{
   using hw3dexp::VertexLayout;
   hw3dexp::VertexBuffer vbuf(std::move(
      VertexLayout{}
      .Append(VertexLayout::Position3D)
      .Append(VertexLayout::Normal)
   ));

   for (unsigned int i = 0; i < mesh.mNumVertices; i++)
   {
      vbuf.EmplaceBack(
         *reinterpret_cast<XMFLOAT3 *>(&mesh.mVertices[i]),
         *reinterpret_cast<XMFLOAT3 *>(&mesh.mNormals[i])
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

   std::unique_ptr<ModelObject> object = std::make_unique<ModelObject>(gfx);

   object->LoadVerticesBuffer(vbuf);

   object->LoadIndicesBuffer(indices);
   object->CreateShader(L"LightedVS.cso", L"LightedPS.cso");

   XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
   object->CreateConstant(position);

   // Create Root Signature after constants
   object->CreateRootSignature(true, false);

   object->CreatePipelineState(vbuf.GetLayout().GetD3DLayout(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

   object->SetLightView(lightView);

   bindablePtrs.push_back(std::move(object));
   return std::make_unique<Mesh>(gfx, std::move(bindablePtrs), (UINT)indices.size(), MaterialIndex);
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
