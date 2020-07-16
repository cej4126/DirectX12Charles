#include "ObjectX11.h"

ObjectX11::ObjectX11(Graphics &gfx)
   :
   gfx(gfx)
{
}

void ObjectX11::AddIndexBuffer(const std::vector<unsigned short> &indices)
{
   indexCount = (UINT)indices.size();

   D3D11_BUFFER_DESC ibd = {};
   ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
   ibd.Usage = D3D11_USAGE_DEFAULT;
   ibd.CPUAccessFlags = 0u;
   ibd.MiscFlags = 0u;
   ibd.ByteWidth = UINT(indexCount * sizeof(unsigned short));
   ibd.StructureByteStride = sizeof(unsigned short);
   D3D11_SUBRESOURCE_DATA isd = {};
   isd.pSysMem = indices.data();
   ThrowIfFailed(GetDevice(gfx)->CreateBuffer(&ibd, &isd, &pIndexBuffer));

}

void ObjectX11::AddShaders(const std::wstring &vertexPath, const std::wstring &pixelPath)
{
   ThrowIfFailed(D3DReadFileToBlob(vertexPath.c_str(), &pVertexBytecodeBlob));
   ThrowIfFailed(GetDevice(gfx)->CreateVertexShader(
      pVertexBytecodeBlob->GetBufferPointer(),
      pVertexBytecodeBlob->GetBufferSize(),
      nullptr,
      &pVertexShader
   ));

   ThrowIfFailed(D3DReadFileToBlob(pixelPath.c_str(), &pPixelBytecodeBlob));
   ThrowIfFailed(GetDevice(gfx)->CreatePixelShader(
      pPixelBytecodeBlob->GetBufferPointer(),
      pPixelBytecodeBlob->GetBufferSize(),
      nullptr,
      &pPixelShader));

}

void ObjectX11::AddInputLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC> &layout)
{
   ThrowIfFailed(GetDevice(gfx)->CreateInputLayout(
      layout.data(), (UINT)layout.size(),
      pVertexBytecodeBlob->GetBufferPointer(),
      pVertexBytecodeBlob->GetBufferSize(),
      &pInputLayout));
}

void ObjectX11::AddTopology(D3D11_PRIMITIVE_TOPOLOGY type)
{
   topologyType = type;
}

void ObjectX11::Bind(Graphics &gfx) noexcept
{

}