#include "Transform.h"
#include "Graphics.h"

Transform::Transform(Graphics &gfx, const DrawFunction &parent)
   :
   gfx(gfx),
   device(gfx.GetDevice()),
   commandList(gfx.GetCommandList()),
   parentTransform(parent)
{
}

void Transform::Bind(Graphics &gfx) noexcept
{
   if (textureActive)
   {
      // set the descriptor heap
      ID3D12DescriptorHeap *descriptorHeaps[] = { mainDescriptorHeap.Get() };
      commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

      // set the descriptor table to the descriptor heap (parameter 1, as constant buffer root descriptor is parameter index 0)
      commandList->SetGraphicsRootDescriptorTable(1, mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
   }

   const auto modelView = parentTransform.GetTransformXM() * gfx.GetCamera();
   const Graphics::TransformMatrix contantMatrix =
   {
      XMMatrixTranspose(modelView),
      XMMatrixTranspose(
      modelView *
      gfx.GetProjection())
   };

   int index = getIndex();
   if (index == -1)
   {
      assert(false);
   }

   //tempindex = index;
//   else if (tempindex != index)
//   {
//      tempindex = index;
//   }

   gfx.SetMatrixConstant(index, contantMatrix);

   int materialIndex = parentTransform.getMaterialIndex();
   if (materialIndex != -1)
   {
      gfx.SetMaterialConstant(materialIndex);
   }

   commandList->DrawIndexedInstanced(indicesCount, 1u, indicesStart, 0u, 0u);

   if (lightBufferActive)
   {
      auto dataCopy = gfx.lightData;
      const auto pos = XMLoadFloat3(&dataCopy.position);
      XMMATRIX cam = gfx.GetCamera();
      XMStoreFloat3(&dataCopy.position, XMVector3Transform(pos, cam));
      memcpy(lightBufferGPUAddress, &dataCopy, sizeof(dataCopy));
   }
}

void Transform::CreateTexture(const Surface &surface)
{
   textureActive = true;
   //const UINT indicesBufferSize = (UINT)(sizeof(unsigned short) * indices.size());

   D3D12_HEAP_PROPERTIES heapProps;
   ZeroMemory(&heapProps, sizeof(heapProps));
   heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
   heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
   heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
   heapProps.CreationNodeMask = 1;
   heapProps.VisibleNodeMask = 1;

   D3D12_RESOURCE_DESC resourceDesc;
   ZeroMemory(&resourceDesc, sizeof(resourceDesc));
   resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
   //resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
   resourceDesc.Alignment = 0;
   resourceDesc.Width = surface.GetWidth();
   resourceDesc.Height = surface.GetHeight();
   resourceDesc.DepthOrArraySize = 1;
   resourceDesc.MipLevels = 1;
   //resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
   resourceDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
   resourceDesc.SampleDesc.Count = 1;
   resourceDesc.SampleDesc.Quality = 0;
   resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
   //resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
   resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

   ThrowIfFailed(device->CreateCommittedResource(
      &heapProps,
      D3D12_HEAP_FLAG_NONE,
      &resourceDesc,
      D3D12_RESOURCE_STATE_COPY_DEST,
      nullptr,
      IID_PPV_ARGS(&textureBuffer)));
   textureBuffer->SetName(L"Texture Default Buffer");

   // Upload heap
   heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
   UINT64 textureUploadBufferSize;
   device->GetCopyableFootprints(&resourceDesc, 0, 1, 0, nullptr, nullptr, nullptr, &textureUploadBufferSize);

   resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
   resourceDesc.Width = textureUploadBufferSize;
   resourceDesc.Height = 1;
   resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
   resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

   ThrowIfFailed(device->CreateCommittedResource(
      &heapProps,
      D3D12_HEAP_FLAG_NONE,
      &resourceDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&textureBufferUploadHeap)));
   textureBufferUploadHeap->SetName(L"Texture Upload Buffer");

   // copy data to the upload heap
   D3D12_SUBRESOURCE_DATA TextureData = {};
   TextureData.pData = surface.GetBufferPtr();
   TextureData.RowPitch = surface.GetWidth() * sizeof(Surface::Color);
   TextureData.SlicePitch = surface.GetWidth() * sizeof(Surface::Color) * surface.GetHeight();

   gfx.UpdateSubresource(
      textureBuffer.Get(),
      textureBufferUploadHeap.Get(),
      &TextureData); // pSrcData

   D3D12_RESOURCE_BARRIER resourceBarrier;
   resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
   resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
   resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
   resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
   resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
   resourceBarrier.Transition.pResource = textureBuffer.Get();
   commandList->ResourceBarrier(1, &resourceBarrier);

   // create the descriptor heap that will store our srv
   D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
   heapDesc.NumDescriptors = 1;
   heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
   heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
   ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(&mainDescriptorHeap)));

   // now we create a shader resource view (descriptor that points to the texture and describes it)
   D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
   srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
   srvDesc.Format = resourceDesc.Format;
   srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
   srvDesc.Texture2D.MipLevels = 1;
   device->CreateShaderResourceView(textureBuffer.Get(), &srvDesc, mainDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

