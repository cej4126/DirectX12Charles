#pragma once
#include "stdafx.h"
#include "BindableX11.h"
#include "Graphics.h"
#include "DrawX11.h"

class TransformX11 : public BindableX11
{
public:
   TransformX11(Graphics &gfx, const DrawX11 &parent);
   void setIndices(UINT start, UINT count)
   {
      indicesStart = start;
      indicesCount = count;
   }

   // Vertex Constant Buffer
   void AddTransformConstantBuffer()
   {
      XMMATRIX Vertexcbuf;

      D3D11_BUFFER_DESC cbd;
      cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
      cbd.Usage = D3D11_USAGE_DYNAMIC;
      cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
      cbd.MiscFlags = 0u;
      cbd.ByteWidth = sizeof(Vertexcbuf);
      cbd.StructureByteStride = 0u;

      D3D11_SUBRESOURCE_DATA csd = {};
      csd.pSysMem = &Vertexcbuf;
      ThrowIfFailed(GetDevice(gfx)->CreateBuffer(&cbd, &csd, &pTransformConstantBuffer));
   }
   void Bind(Graphics &gfx) noexcept override;

public:
   Graphics &gfx;

private:
   // Vetrix Constant Buffer
   Microsoft::WRL::ComPtr<ID3D11Buffer> pTransformConstantBuffer;
   const DrawX11 &parentTransform;
   UINT indicesStart = 0;
   UINT indicesCount = 0;
};
