#pragma once
#include "BindableX12.h"
#include "Graphics.h"
#include "DrawX12.h"

class TransformX12 : public BindableX12
{
public:
   TransformX12(Graphics &gfx, const DrawX12 &parent);
   void Bind(Graphics &gfx, int index) noexcept override;
   void setIndices(UINT start, UINT count)
   {
      indicesStart = start;
      indicesCount = count;
   }

public:
   Graphics &gfx;
   ID3D12GraphicsCommandList *commandList;

private:
   // Vetrix Constant Buffer
   Microsoft::WRL::ComPtr<ID3D11Buffer> pTransformConstantBuffer;
   const DrawX12 &parentTransform;
   UINT8 *matrixBufferGPUAddress;
   UINT indicesStart = 0;
   UINT indicesCount = 0;
};
