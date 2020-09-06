#pragma once
#include "Bindable.h"
#include "Graphics.h"
#include "DrawFunction.h"

class Transform : public Bindable
{
public:

public:
   Transform(Graphics &gfx, const DrawFunction &parent);
   void Bind(Graphics &gfx, int index) noexcept override;
   void setIndices(UINT start, UINT count)
   {
      indicesStart = start;
      indicesCount = count;
   }
   ID3D12Resource *getLightView() { return lightBufferUploadHeaps.Get(); }

public:
   Graphics &gfx;
   ID3D12Device *device;
   ID3D12GraphicsCommandList *commandList;

   ID3D12Resource *CreateLightPosition(XMFLOAT3 &Buffer);

private:
   // Vetrix Constant Buffer
   Microsoft::WRL::ComPtr<ID3D11Buffer> pTransformConstantBuffer;
   const DrawFunction &parentTransform;
   UINT8 *matrixBufferGPUAddress;
   UINT indicesStart = 0;
   UINT indicesCount = 0;

   bool lightBufferActive = false;
   Microsoft::WRL::ComPtr <ID3D12Resource> lightBufferUploadHeaps;
   UINT8 *lightBufferGPUAddress;
};
