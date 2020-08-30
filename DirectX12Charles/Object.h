#pragma once
#include "stdafx.h"
#include "Bindable.h"
#include "Graphics.h"
#include "Surface.h"

class Object : public Bindable
{
public:
   Object(Graphics &gfx);

   void CreateTexture(const Surface &surface);
   void CreateRootSignature(bool constantFlag, bool textureFlag);
   void CreateShader(const std::wstring &vertexPath, const std::wstring &pixelPath);
   void CreateConstant();
   void CreatePipelineState(const std::vector<D3D12_INPUT_ELEMENT_DESC> &inputElementDescs, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType);

   template<class V>
   void LoadVerticesBuffer(const std::vector<V> &vertices)
   {
      const UINT vertexBufferSize = (UINT)(sizeof(V) * vertices.size());

      D3D12_HEAP_PROPERTIES heapProps;
      ZeroMemory(&heapProps, sizeof(heapProps));
      heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
      heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
      heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
      heapProps.CreationNodeMask = 1;
      heapProps.VisibleNodeMask = 1;

      D3D12_RESOURCE_DESC resourceDesc;
      ZeroMemory(&resourceDesc, sizeof(resourceDesc));
      resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
      resourceDesc.Alignment = 0;
      resourceDesc.Width = vertexBufferSize;
      resourceDesc.Height = 1;
      resourceDesc.DepthOrArraySize = 1;
      resourceDesc.MipLevels = 1;
      resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
      resourceDesc.SampleDesc.Count = 1;
      resourceDesc.SampleDesc.Quality = 0;
      resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
      resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

      ThrowIfFailed(device->CreateCommittedResource(
         &heapProps,
         D3D12_HEAP_FLAG_NONE,
         &resourceDesc,
         D3D12_RESOURCE_STATE_COPY_DEST,
         nullptr,
         IID_PPV_ARGS(&vertexDefaultBuffer)));

      // Upload heap
      heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

      ThrowIfFailed(device->CreateCommittedResource(
         &heapProps,
         D3D12_HEAP_FLAG_NONE,
         &resourceDesc,
         D3D12_RESOURCE_STATE_GENERIC_READ,
         nullptr,
         IID_PPV_ARGS(&vertexUploadBuffer)));

      // copy data to the upload heap
      D3D12_SUBRESOURCE_DATA vertexData = {};
      vertexData.pData = vertices.data(); //reinterpret_cast<BYTE *>(
      vertexData.RowPitch = vertexBufferSize;
      vertexData.SlicePitch = vertexBufferSize;

      // Add the copy to the command list
      gfx.UpdateSubresource(
         vertexDefaultBuffer.Get(),
         vertexUploadBuffer.Get(),
         &vertexData); // pSrcData

      D3D12_RESOURCE_BARRIER resourceBarrier;
      resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
      resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
      resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
      resourceBarrier.Transition.pResource = vertexDefaultBuffer.Get();
      commandList->ResourceBarrier(1, &resourceBarrier);

      // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
      vertexBufferView.BufferLocation = vertexDefaultBuffer->GetGPUVirtualAddress();
      vertexBufferView.StrideInBytes = sizeof(V);
      vertexBufferView.SizeInBytes = vertexBufferSize;

   }

   void LoadIndicesBuffer(const std::vector<unsigned short> &indices);


   void Bind(Graphics &gfx, int drawStep) noexcept override;
   //void Bind(Graphics &gfx) noexcept override;

private:
   Graphics &gfx;
   ID3D12Device *device;
   ID3D12GraphicsCommandList *commandList;

   struct ConstantBufferColor
   {
      struct
      {
         float r;
         float g;
         float b;
         float a;
      } face_colors[6];
   };
   struct ConstantBufferColor colorBuffer;

   bool colorBufferActive = false;
   Microsoft::WRL::ComPtr <ID3D12Resource> colorBufferUploadHeaps;
   UINT8 *colorBufferGPUAddress;

   bool textureActive = false;
   Microsoft::WRL::ComPtr < ID3D12Resource > textureBuffer;
   Microsoft::WRL::ComPtr < ID3D12DescriptorHeap >mainDescriptorHeap;
   Microsoft::WRL::ComPtr < ID3D12Resource > textureBufferUploadHeap;

   Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature;
   D3D12_INPUT_LAYOUT_DESC inputLayoutDesc;
   Microsoft::WRL::ComPtr <ID3DBlob> vertexShaderBlob;
   Microsoft::WRL::ComPtr <ID3DBlob> pixelShaderBlob;
   Microsoft::WRL::ComPtr <ID3D12PipelineState> pipelineState;

   Microsoft::WRL::ComPtr <ID3D12Resource> vertexDefaultBuffer;
   Microsoft::WRL::ComPtr <ID3D12Resource> vertexUploadBuffer;
   D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
   Microsoft::WRL::ComPtr <ID3D12Resource> indexDefaultBuffer;
   Microsoft::WRL::ComPtr <ID3D12Resource> indexUploadBuffer;
   D3D12_INDEX_BUFFER_VIEW indexBufferView;
   UINT indicesCount;
   D3D12_PRIMITIVE_TOPOLOGY_TYPE topology = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
};

