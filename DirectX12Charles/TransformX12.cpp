#include "TransformX12.h"
#include "Graphics.h"

TransformX12::TransformX12(Graphics &gfx, const DrawX12 &parent)
   :
   gfx(gfx),
   commandList(gfx.GetCommandListX12()),
   parentTransform(parent)
{
}

void TransformX12::Bind(Graphics &gfx, int index) noexcept
{
   XMMATRIX matrixBuffer = XMMatrixTranspose(
      parentTransform.GetTransformXM() *
      gfx.GetCamera() *
      gfx.GetProjection());

   gfx.SetMatrixConstantX12(index, matrixBuffer);

   commandList->DrawIndexedInstanced(indicesCount, 1u, indicesStart, 0u, 0u);
}
