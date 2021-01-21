#pragma once
#include "stdafx.h"
#include "Bindable.h"
#include "Graphics.h"
#include "Surface.h"
#include "Vertex.h"

class Object : public Bind::Bindable
{
public:
   Object(Graphics &gfx, std::string tag);

   static std::shared_ptr<Bind::Bindable> Resolve(Graphics &gfx, const std::string &tag);
   static std::string GenerateUID(const std::string &path);
   std::string GetUID() const noexcept override;

   void CreateTexture(const Surface &surface, int slot);
   void CreateRootSignature(bool constantFlag, bool materialFlag, bool textureFlag);
   void CreateShader(const std::wstring &vertexPath, const std::wstring &pixelPath);
   void SetLightView(ID3D12Resource *mylightView);
   void CreatePipelineState(const std::vector<D3D12_INPUT_ELEMENT_DESC> &inputElementDescs, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType);

   void LoadVerticesBuffer(const hw3dexp::VertexBuffer &vertices);

   void LoadIndicesBuffer(const std::vector<unsigned short> &indices);

   void CreateConstant(const XMFLOAT3 &colorBuffer, int size);

   void Bind(Graphics &gfx, int drawStep) noexcept override;

private:
   Graphics &gfx;
   ID3D12Device *device;
   ID3D12GraphicsCommandList *commandList;

   bool colorBufferActive = false;
   Microsoft::WRL::ComPtr <ID3D12Resource> colorBufferUploadHeaps;
   UINT8 *colorBufferGPUAddress;

   std::string tag;

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

   bool lightActive = false;
   ID3D12Resource *lightView = nullptr;
};

