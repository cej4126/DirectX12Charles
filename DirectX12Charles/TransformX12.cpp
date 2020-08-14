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
      parentTransform.GetTransformXM() * gfx.GetProjectionX12());

   gfx.SetMatrixConstantX12(index, matrixBuffer);

}
