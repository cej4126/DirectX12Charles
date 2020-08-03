#pragma once
#include "stdafx.h"
#include "Graphics.h"

class OneBoxX12
{
public:
   OneBoxX12(Graphics &gfx);
   void Update(float dt);
   void Draw();
   void LoadConstant();
   void LoadDepend();

private:
   Graphics &gfx;
   ID3D12GraphicsCommandList *commandList;
   ID3D12Device *device;

   struct MatrixBufferType
   {
      XMMATRIX transform;
   };
   MatrixBufferType matrixBuffer;

   //struct VertexX11
   //{
   //   struct
   //   {
   //      float x;
   //      float y;
   //      float z;
   //   } pos;
   //};

   struct Vertex
   {
      Vertex(float x, float y, float z) : pos(x, y, z) {}
      DirectX::XMFLOAT3 pos;
   };

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

   int ConstantBufferPerObjectAlignedSize = (sizeof(matrixBuffer) + 255) & ~255;

   Microsoft::WRL::ComPtr <ID3D12Resource> matrixBufferUploadHeaps;
   UINT8 *matrixBufferGPUAddress;

   Microsoft::WRL::ComPtr <ID3D12Resource> colorBufferUploadHeaps;
   UINT8 *colorBufferGPUAddress;
   float angle = 0.0f;

   void CreateRootSignature();
   void LoadVertexBuffer();
   void LoadIndexBuffer();
   void CreateShader();
   void CreateConstant();
   void CreatePipelineState();

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

};

