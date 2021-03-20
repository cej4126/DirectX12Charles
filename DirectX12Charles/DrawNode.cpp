#include "DrawNode.h"
#include "imgui/imgui.h"
#include <unordered_map>

DrawNode::DrawNode(int id, const std::string &name, std::vector<DrawMesh * > DrawMeshPtrs, const XMMATRIX &transformIn) noexcept
   :
   id(id),
   MeshPtrs(std::move(DrawMeshPtrs)),
   name(name)
{
   XMStoreFloat4x4(&transform, transformIn);
   XMStoreFloat4x4(&appliedTransform, XMMatrixIdentity());
}

void DrawNode::Draw(Graphics &gfx, FXMMATRIX accumulatedTransform)
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

void DrawNode::AddChild(std::unique_ptr<DrawNode> pChild) noexcept
{
   childPtrs.push_back(std::move(pChild));
}

void DrawNode::ShowTree(DrawNode *&pSelectedNode) const noexcept
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
      pSelectedNode = const_cast<DrawNode *>(this);
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

void DrawNode::SetAppliedTransform(FXMMATRIX transform) noexcept
{
   XMStoreFloat4x4(&appliedTransform, transform);
}

const XMFLOAT4X4 &DrawNode::GetAppliedTransform() const noexcept
{
   return appliedTransform;
}

int DrawNode::GetId() const noexcept
{
   return id;
}
