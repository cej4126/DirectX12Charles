#include "Transform.h"
#include "Graphics.h"

Transform::Transform(Graphics &gfx, const DrawFunction &parent, int rootVS, int rootPS)
   :
   gfx(gfx),
   device(gfx.GetDevice()),
   commandList(gfx.GetCommandList()),
   parentTransform(parent),
   m_rootVS(rootVS),
   m_rootPS(rootPS)
{
}

void Transform::Bind(Graphics &gfx) noexcept
{
   const auto modelView = parentTransform.GetTransformXM() * gfx.GetCamera();
   const Graphics::TransformMatrix contantMatrix =
   {
      XMMatrixTranspose(modelView),
      XMMatrixTranspose(
      modelView *
      gfx.GetProjection())
   };

   int index = getIndex();
   if (index == -1)
   {
      assert(false);
   }

   gfx.SetMatrixConstant(index, contantMatrix, m_rootVS, m_rootPS);

   int materialIndex = parentTransform.getMaterialIndex();
   if (materialIndex != -1)
   {
      gfx.SetMaterialConstant(materialIndex);
   }

   if (indicesCount > 0)
   {
      commandList->DrawIndexedInstanced(indicesCount, 1u, indicesStart, 0u, 0u);
   }

   if (lightBufferActive)
   {
      auto dataCopy = gfx.lightData;
      const auto pos = XMLoadFloat3(&dataCopy.viewLightPos);
      XMMATRIX cam = gfx.GetCamera();
      XMStoreFloat3(&dataCopy.viewLightPos, XMVector3Transform(pos, cam));
      memcpy(lightBufferGPUAddress, &dataCopy, sizeof(dataCopy));
   }
}
