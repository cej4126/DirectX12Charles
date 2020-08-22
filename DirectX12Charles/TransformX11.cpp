#include "TransformX11.h"

TransformX11::TransformX11(Graphics &gfx, const DrawX11 &parent)
   :
   gfx(gfx),
   parentTransform(parent)
{
}

void TransformX11::Bind(Graphics &gfx) noexcept
{
   XMMATRIX consts = XMMatrixTranspose(
      parentTransform.GetTransformXM() * gfx.GetProjectionX11());

   D3D11_MAPPED_SUBRESOURCE msr;
   ThrowIfFailed(GetContext(gfx)->Map(
      pTransformConstantBuffer.Get(), 0u,
      D3D11_MAP_WRITE_DISCARD, 0u,
      &msr
   ));
   memcpy(msr.pData, &consts, sizeof(consts));
   GetContext(gfx)->Unmap(pTransformConstantBuffer.Get(), 0u);

   GetContext(gfx)->VSSetConstantBuffers(0u, 1u, pTransformConstantBuffer.GetAddressOf());

   GetContext(gfx)->DrawIndexed(indicesCount, indicesStart, 0u);
}
