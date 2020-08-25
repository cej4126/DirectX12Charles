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

void ObjectX11::AddTexture(const Surface &surface)
{
   textureActive = true;

   D3D11_TEXTURE2D_DESC textureDesc = {};
   textureDesc.Height = surface.GetHeight();
   textureDesc.Width = surface.GetWidth();
   textureDesc.MipLevels = 1;
   textureDesc.ArraySize = 1;
   textureDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
   textureDesc.SampleDesc.Count = 1;
   textureDesc.SampleDesc.Quality = 0;
   textureDesc.Usage = D3D11_USAGE_DEFAULT;
   textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
   textureDesc.CPUAccessFlags = 0;
   textureDesc.MiscFlags = 0;

   D3D11_SUBRESOURCE_DATA subresource = {};
   subresource.pSysMem = surface.GetBufferPtr();
   subresource.SysMemPitch = surface.GetWidth() * sizeof(Surface::Color);


   ThrowIfFailed(GetDevice(gfx)->CreateTexture2D(
      &textureDesc, &subresource, &pTexture));
   
   D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
   srvDesc.Format = textureDesc.Format;
   srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
   srvDesc.Texture2D.MostDetailedMip = 0;
   srvDesc.Texture2D.MipLevels = 1;

   ThrowIfFailed(GetDevice(gfx)->CreateShaderResourceView(
      pTexture.Get(), &srvDesc, &pTextureView));
}

void ObjectX11::AddSampler()
{
   SamplerActive = true;

   D3D11_SAMPLER_DESC samplerDesc = {};
   samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
   samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
   samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
   samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;

   ThrowIfFailed(GetDevice(gfx)->CreateSamplerState(&samplerDesc, &pSampler));
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
   // Vertex Buffer
   const UINT offset = 0u;
   GetContext(gfx)->IASetVertexBuffers(0u, 1u, pVertexBuffer.GetAddressOf(), &vertexStride, &offset);

   // Index Buffer
   GetContext(gfx)->IASetIndexBuffer(pIndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);

   // Shaders
   GetContext(gfx)->VSSetShader(pVertexShader.Get(), nullptr, 0u);
   GetContext(gfx)->PSSetShader(pPixelShader.Get(), nullptr, 0u);

   // Layout
   GetContext(gfx)->IASetInputLayout(pInputLayout.Get());

   // Topology
   GetContext(gfx)->IASetPrimitiveTopology(topologyType);

   if (pixelConstantBufferActive)
   {
      GetContext(gfx)->PSSetConstantBuffers(0u, 1u, pPixelConstantBuffer.GetAddressOf());
   }

   if (SamplerActive)
   {
      GetContext(gfx)->PSSetSamplers(0u, 1u, pSampler.GetAddressOf());
   }

   if (textureActive)
   {
      GetContext(gfx)->PSSetShaderResources(0u, 1u, pTextureView.GetAddressOf());
   }

   //GetContext(gfx)->DrawIndexed(indexCount, 0u, 0u);
}