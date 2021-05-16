#pragma once
#include "Bindable.h"
#include "Graphics.h"
#include "DrawFunction.h"
#include "Surface.h"

class Transform : public Bind::Bindable
{
public:
   Transform(Graphics &gfx, const DrawFunction &parent, int rootVS = 0, int rootPS = -1);
   void Bind(Graphics &gfx) noexcept override;

   void setIndices(int index, UINT start, UINT count)
   {
      setIndex(index);
      indicesStart = start;
      indicesCount = count;
   }
   ID3D12Resource *getLightView() { return lightBufferUploadHeaps.Get(); }

public:
   Graphics &gfx;
   ID3D12Device *device;
   ID3D12GraphicsCommandList *commandList;

   template<class V>
   ID3D12Resource *CreateLightPosition(V &buffer)
   {
      lightBufferActive = true;

      D3D12_RESOURCE_DESC constantHeapDesc = {};
      constantHeapDesc.Alignment = 0;
      constantHeapDesc.DepthOrArraySize = 1;
      constantHeapDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      constantHeapDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
      constantHeapDesc.Format = DXGI_FORMAT_UNKNOWN;
      constantHeapDesc.Height = 1;
      constantHeapDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      constantHeapDesc.SampleDesc.Count = 1;
      constantHeapDesc.SampleDesc.Quality = 0;
      constantHeapDesc.Width = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
      constantHeapDesc.MipLevels = 1;

      D3D12_HEAP_PROPERTIES constantHeapUpload = {};
      constantHeapUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
      constantHeapUpload.CreationNodeMask = 1;
      constantHeapUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
      constantHeapUpload.Type = D3D12_HEAP_TYPE_UPLOAD;
      constantHeapUpload.VisibleNodeMask = 1;

      ThrowIfFailed(device->CreateCommittedResource(
         &constantHeapUpload,
         D3D12_HEAP_FLAG_NONE,
         &constantHeapDesc,
         D3D12_RESOURCE_STATE_GENERIC_READ,
         nullptr,
         IID_PPV_ARGS(&lightBufferUploadHeaps)));

      D3D12_RANGE readRange;
      readRange.Begin = 1;
      readRange.End = 0;
      ThrowIfFailed(lightBufferUploadHeaps->Map(0, &readRange, reinterpret_cast<void **>(&lightBufferGPUAddress)));

      int ConstantBufferPerObjectAlignedSize = (sizeof(buffer) + 255) & ~255;

      memcpy(lightBufferGPUAddress + 0 * ConstantBufferPerObjectAlignedSize, &buffer, sizeof(buffer));

      return lightBufferUploadHeaps.Get();
   }

private:
   // Vetrix Constant Buffer
   //Microsoft::WRL::ComPtr<ID3D11Buffer> pTransformConstantBuffer;
   const DrawFunction &parentTransform;
   UINT indicesStart = 0;
   UINT indicesCount = 0;

   bool lightBufferActive = false;
   Microsoft::WRL::ComPtr <ID3D12Resource> lightBufferUploadHeaps;
   UINT8 *lightBufferGPUAddress = nullptr;

   int m_rootVS;
   int m_rootPS;

};
