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

void Transform::Bind(Graphics &gfx, int index) noexcept
{
   const auto model = parentTransform.GetTransformXM();
   const Graphics::TransformMatrix contantMatrix =
   {
      XMMatrixTranspose(model),
      XMMatrixTranspose(
      model *
      gfx.GetCamera() *
      gfx.GetProjection())
   };

   gfx.SetMatrixConstant(index, contantMatrix);

   commandList->DrawIndexedInstanced(indicesCount, 1u, indicesStart, 0u, 0u);

   if (lightBufferActive)
   {
      XMFLOAT3 buffer = { model.r[3].m128_f32[0], model.r[3].m128_f32[1], model.r[3].m128_f32[2] };
      int ConstantBufferPerObjectAlignedSize = (sizeof(buffer) + 255) & ~255;
      memcpy(lightBufferGPUAddress + 0 * ConstantBufferPerObjectAlignedSize, &buffer, sizeof(buffer));
   }
}

ID3D12Resource *Transform::CreateLightPosition(XMFLOAT3 &buffer)
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
