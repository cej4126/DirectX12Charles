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
   const auto modelView = parentTransform.GetTransformXM() * gfx.GetCamera();
   const Graphics::TransformMatrix contantMatrix =
   {
      XMMatrixTranspose(modelView),
      XMMatrixTranspose(
      modelView *
      gfx.GetProjection())
   };

   gfx.SetMatrixConstant(index, contantMatrix);

   int materialIndex = parentTransform.getMaterialIndex();
   if (materialIndex != -1)
   {
      gfx.SetMaterialConstant(materialIndex);
   }

   commandList->DrawIndexedInstanced(indicesCount, 1u, indicesStart, 0u, 0u);

   if (lightBufferActive)
   {
      auto dataCopy = gfx.lightData;
      const auto pos = XMLoadFloat3(&dataCopy.position);
      XMMATRIX cam = gfx.GetCamera();
      XMStoreFloat3(&dataCopy.position, XMVector3Transform(pos, cam));
      memcpy(lightBufferGPUAddress, &dataCopy, sizeof(dataCopy));
   }
}
