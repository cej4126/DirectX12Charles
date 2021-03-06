#pragma once
#include "stdafx.h"
#include "Bindable.h"
#include "Graphics.h"
#include "Surface.h"
#include "Vertex.h"

class ModelSpec : public Bind::Bindable
{
public:
   ModelSpec(Graphics &gfx, std::string tag);

   static std::shared_ptr<ModelSpec> Resolve(Graphics &gfx, const std::string &tag);
   static std::string GenerateUID(const std::string &tag);
   std::string GetUID() const noexcept override;

   //void CreateTexture(std::string path, int slot, bool alphaGloss = false);
   //bool getAlphaGloss() { return m_alphaGloss; }

   enum class Model_Type
   {
      MODEL_DIFF_NORMAL_SPEC,
      MODEL_DIFF_NORMAL,
      MODEL_DIFF,
      MODEL_NONE
   };

   void CreateRootSignature(Model_Type type);
   void CreateShader(const std::wstring &vertexPath, const std::wstring &pixelPath);
   void SetLightView(ID3D12Resource *mylightView);
   void CreatePipelineState(const std::vector<D3D12_INPUT_ELEMENT_DESC> &inputElementDescs, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType);

   void LoadVerticesBuffer(const hw3dexp::VertexBuffer &vertices);

   void LoadIndicesBuffer(const std::vector<unsigned short> &indices);

   void CreateConstant(const XMFLOAT3 &colorBuffer, int size);

   void Bind(Graphics &gfx) noexcept override;
   void setSize(int size) { m_size = size; };
   int getSize() { return m_size; };

private:
   enum
   {
      VIEW_CB = 0,
      LIGHT_CB,
      MATERIAL_CB,
      TEXTURE_CB,
      COUNT_CB
   };
   static const int NUMBER_OF_VIEW = 3;

   Graphics &gfx;
   ID3D12Device *device;
   ID3D12GraphicsCommandList *commandList;

   std::string tag;

   bool colorBufferActive = false;
   Microsoft::WRL::ComPtr <ID3D12Resource> colorBufferUploadHeaps;
   UINT8 *colorBufferGPUAddress = 0;
   int m_size;

   //bool textureActive = false;
   //bool m_alphaGloss = false;
   //bool specularActive = false;
   //Microsoft::WRL::ComPtr < ID3D12Resource > textureBuffer[NUMBER_OF_VIEW];
   //Microsoft::WRL::ComPtr < ID3D12DescriptorHeap >mainDescriptorHeap;
   //Microsoft::WRL::ComPtr < ID3D12Resource > textureBufferUploadHeap[NUMBER_OF_VIEW];

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

   ID3D12Resource *lightView = nullptr;
};

