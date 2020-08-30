#include "Transform.h"
#include "Graphics.h"

Transform::Transform(Graphics &gfx, const DrawX12 &parent)
   :
   gfx(gfx),
   commandList(gfx.GetCommandList()),
   parentTransform(parent)
{
}

void Transform::Bind(Graphics &gfx, int index) noexcept
{
   XMMATRIX matrixBuffer = XMMatrixTranspose(
      parentTransform.GetTransformXM() *
      gfx.GetCamera() *
      gfx.GetProjection());

   gfx.SetMatrixConstant(index, matrixBuffer);

   commandList->DrawIndexedInstanced(indicesCount, 1u, indicesStart, 0u, 0u);
}
