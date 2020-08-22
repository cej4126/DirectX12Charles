#pragma once
#include "Graphics.h"
#include "Shape.h"

class OneBoxX11
{
public:
   OneBoxX11(Graphics &gfx);
   void Update(float dt);
   void Draw();

private:
   Graphics &gfx;
   ID3D11Device *device;
   ID3D11DeviceContext *context;

   struct MatrixBufferType
   {
      XMMATRIX transform;
   };
   MatrixBufferType matrixBuffer;
   int ConstantBufferPerObjectAlignedSize = (sizeof(matrixBuffer) + 255) & ~255;

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
   struct ConstantBufferColor colorBuffer = { 0.0 };

   float angle = 0.0f;

   Microsoft::WRL::ComPtr<ID3D11Buffer> x11VertexBuffer;
   Microsoft::WRL::ComPtr<ID3D11Buffer> x11IndexBuffer;
   Microsoft::WRL::ComPtr<ID3D11Buffer> x11MatrixBuffer;
   Microsoft::WRL::ComPtr<ID3D11Buffer> x11ColorBuffer;
   Microsoft::WRL::ComPtr<ID3D11PixelShader> x11PixelShader;
   Microsoft::WRL::ComPtr<ID3D11VertexShader> x11VertexShader;
   Microsoft::WRL::ComPtr<ID3D11InputLayout> x11InputLayout;

   UINT indicesCount;
   UINT indicesStart;
   UINT vertexStride;
};

