#include "stdafx.h"
#include "OneBoxX11.h"
//#include "Geometry.h"
using namespace Microsoft::WRL;

OneBoxX11::OneBoxX11(Graphics &gfx)
   :
   gfx(gfx),
   device(gfx.GetDeviceX11()),
   context(gfx.GetContextX11())
{
   struct Vertex
   {
      XMFLOAT3 pos;
   };
   auto model = gfx.shape.GetShapeData<Vertex>();
   //auto model = CubeTemp::Make<Vertex>();

   vertexStride = sizeof(Vertex);
   indicesStart = gfx.shape.getIndiceStart(Shape::Cube);
   indicesCount = gfx.shape.getIndiceCount(Shape::Cube);

   D3D11_BUFFER_DESC verticesX11Desc = {};
   verticesX11Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
   verticesX11Desc.Usage = D3D11_USAGE_DEFAULT;
   verticesX11Desc.CPUAccessFlags = 0u;
   verticesX11Desc.MiscFlags = 0u;
   verticesX11Desc.ByteWidth = UINT(sizeof(Vertex) * model.vertices.size());
   verticesX11Desc.StructureByteStride = sizeof(Vertex);
   D3D11_SUBRESOURCE_DATA verticeX11Data = {};
   verticeX11Data.pSysMem = model.vertices.data();
   ThrowIfFailed(device->CreateBuffer(&verticesX11Desc, &verticeX11Data, &x11VertexBuffer));

   //indiceX11Count = (UINT)model.indices.size();

   D3D11_BUFFER_DESC indicesX11Desc = {};
   indicesX11Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
   indicesX11Desc.Usage = D3D11_USAGE_DEFAULT;
   indicesX11Desc.CPUAccessFlags = 0u;
   indicesX11Desc.MiscFlags = 0u;
   indicesX11Desc.ByteWidth = model.indices.size() * sizeof(unsigned short);
   indicesX11Desc.StructureByteStride = sizeof(unsigned short);
   D3D11_SUBRESOURCE_DATA indiceX11Data = {};
   indiceX11Data.pSysMem = model.indices.data();
   ThrowIfFailed(device->CreateBuffer(&indicesX11Desc, &indiceX11Data, &x11IndexBuffer));

   // Constant Buffer Matrix
   matrixBuffer.transform = XMMatrixIdentity();
   D3D11_BUFFER_DESC cbd;
   cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
   cbd.Usage = D3D11_USAGE_DYNAMIC;
   cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
   cbd.MiscFlags = 0u;
   cbd.ByteWidth = sizeof(matrixBuffer);
   cbd.StructureByteStride = 0u;
   D3D11_SUBRESOURCE_DATA csd = {};
   csd.pSysMem = &matrixBuffer;
   ThrowIfFailed(device->CreateBuffer(&cbd, &csd, &x11MatrixBuffer));

   // lookup table for cube face colors
   ConstantBufferColor cb =
   {
      {
         {1.0f, 0.0f, 1.0f, 1.0f},
         {1.0f, 0.0f, 0.0f, 1.0f},
         {0.0f, 1.0f, 0.0f, 1.0f},
         {0.0f, 0.0f, 1.0f, 1.0f},
         {1.0f, 1.0f, 0.0f, 1.0f},
         {0.0f, 1.0f, 1.0f, 1.0f},
      }
   };

   for (int i = 0; i < 6; i++)
   {
      colorBuffer.face_colors[i].r = cb.face_colors[i].r;
      colorBuffer.face_colors[i].g = cb.face_colors[i].g;
      colorBuffer.face_colors[i].b = cb.face_colors[i].b;
      colorBuffer.face_colors[i].a = cb.face_colors[i].a;
   }

   // Constant Buffer color
   D3D11_BUFFER_DESC colorDesc;
   colorDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
   colorDesc.Usage = D3D11_USAGE_DYNAMIC;
   colorDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
   colorDesc.MiscFlags = 0u;
   colorDesc.ByteWidth = sizeof(colorBuffer);
   colorDesc.StructureByteStride = 0u;
   D3D11_SUBRESOURCE_DATA colorSub = {};
   colorSub.pSysMem = &colorBuffer;
   ThrowIfFailed(device->CreateBuffer(&colorDesc, &colorSub, &x11ColorBuffer));

   // create pixel shader
   ComPtr<ID3DBlob> pBlobPixel;
   ThrowIfFailed(D3DReadFileToBlob(L"ColorIndexPSX11.cso", &pBlobPixel));
   ThrowIfFailed(device->CreatePixelShader(pBlobPixel->GetBufferPointer(), pBlobPixel->GetBufferSize(), nullptr, &x11PixelShader));

   // create vertex shader
   ComPtr<ID3DBlob> pBlobVertex;
   ThrowIfFailed(D3DReadFileToBlob(L"ColorIndexVSX11.cso", &pBlobVertex));
   ThrowIfFailed(device->CreateVertexShader(pBlobVertex->GetBufferPointer(), pBlobVertex->GetBufferSize(), nullptr, &x11VertexShader));

   // input (vertex) layout
   const D3D11_INPUT_ELEMENT_DESC ied[] =
   {
      { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA, 0 },
   };

   ThrowIfFailed(device->CreateInputLayout(
      ied, (UINT)std::size(ied),
      pBlobVertex->GetBufferPointer(),
      pBlobVertex->GetBufferSize(),
      &x11InputLayout));
}

void OneBoxX11::Update(float dt)
{
   angle += 2.0f * dt;
}

void OneBoxX11::Draw()
{
   // Bind vertex buffer to pipeline
   const UINT offset = 0u;
   context->IASetVertexBuffers(0u, 1u, x11VertexBuffer.GetAddressOf(), &vertexStride, &offset);

   context->IASetIndexBuffer(x11IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);

   float offsetx = 4.5f;
   float offsety = 3.0f;
   matrixBuffer.transform = XMMatrixTranspose(
      XMMatrixRotationZ(angle) *
      XMMatrixRotationY(angle) *
      XMMatrixScaling(0.5f, 0.5f, 0.5f) *
      XMMatrixTranslation(offsetx, offsety, 8.0f) *
      XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 10.0f));

   D3D11_MAPPED_SUBRESOURCE msr;
   ThrowIfFailed(context->Map(
      x11MatrixBuffer.Get(), 0u,
      D3D11_MAP_WRITE_DISCARD, 0u,
      &msr
   ));
   memcpy(msr.pData, &matrixBuffer, sizeof(matrixBuffer));
   context->Unmap(x11MatrixBuffer.Get(), 0u);
   context->VSSetConstantBuffers(0u, 1u, x11MatrixBuffer.GetAddressOf());

   D3D11_MAPPED_SUBRESOURCE msrColor;
   ThrowIfFailed(context->Map(
      x11ColorBuffer.Get(), 0u,
      D3D11_MAP_WRITE_DISCARD, 0u,
      &msrColor
   ));
   memcpy(msrColor.pData, &colorBuffer, sizeof(colorBuffer));
   context->Unmap(x11ColorBuffer.Get(), 0u);
   context->PSSetConstantBuffers(0u, 1u, x11ColorBuffer.GetAddressOf());

   // bind pixel shader
   context->PSSetShader(x11PixelShader.Get(), nullptr, 0u);

   // bind vertex shader
   context->VSSetShader(x11VertexShader.Get(), nullptr, 0u);

   context->IASetInputLayout(x11InputLayout.Get());

   // Set primitive topology to triangle list (groups of 3 vertices)
   context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   context->DrawIndexed(indicesCount, indicesStart, 0u);
}
