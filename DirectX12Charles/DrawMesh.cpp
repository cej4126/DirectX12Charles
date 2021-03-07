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
