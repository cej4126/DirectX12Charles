#include "Transform.h"
#include "Graphics.h"

Transform::Transform(Graphics &gfx, const DrawFunction &parent)
   :
   gfx(gfx),
   device(gfx.GetDevice()),
   commandList(gfx.GetCommandList()),
   parentTransform(parent)
{
}

void Transform::Bind(Graphics &gfx, int index) noexcept
{
   const auto model = parentTransform.GetTransformXM();
   const Graphics::TransformMatrix contantMatrix =
   {
      XMMatrixTranspose(model),
      XMMatrixTranspose(
      model *
      gfx.GetCamera() *
      gfx.GetProjection())
   };

   gfx.SetMatrixConstant(index, contantMatrix);

   commandList->DrawIndexedInstanced(indicesCount, 1u, indicesStart, 0u, 0u);

   if (lightBufferActive)
   {
      struct Graphics::lightDataType buffer = parentTransform.GetLightData();
      memcpy(lightBufferGPUAddress, &buffer, sizeof(buffer));
   }
}
