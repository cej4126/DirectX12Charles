#include "Object.h"
using namespace Microsoft::WRL;


Object::Object(Graphics &gfx)
   :
   gfx(gfx),
   device(gfx.GetDevice()),
   commandList(gfx.GetCommandList())
{
}

void Object::Bind(Graphics &gfx, int drawStep) noexcept
{
   //int drawStep = 0;
   if (drawStep == 0)
   {
      commandList->SetGraphicsRootSignature(rootSignature.Get());
   }
   else if (drawStep == 2)
   {
      commandList->SetPipelineState(pipelineState.Get());

      commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
      commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
      commandList->IASetIndexBuffer(&indexBufferView);

      if (colorBufferActive)
      {
         commandList->SetGraphicsRootConstantBufferView(1, colorBufferUploadHeaps->GetGPUVirtualAddress());
      }

      if (textureActive)
      {
         // set the descriptor heap
         ID3D12DescriptorHeap *descriptorHeaps[] = { mainDescriptorHeap.Get() };
         commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);

         // set the descriptor table to the descriptor heap (parameter 1, as constant buffer root descriptor is parameter index 0)
         commandList->SetGraphicsRootDescriptorTable(1, mainDescriptorHeap->GetGPUDescriptorHandleForHeapStart());
      }
   }
}

void Object::CreateRootSignature(bool constantFlag, bool textureFlag)
{
   int rootCount = 1;

   D3D12_ROOT_DESCRIPTOR rootCBVDescriptor;
   rootCBVDescriptor.RegisterSpace = 0;
   rootCBVDescriptor.ShaderRegister = 0;

   D3D12_ROOT_PARAMETER  rootParameters[2];
   // constant buffer for matrix
   rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
   rootParameters[0].Descriptor = rootCBVDescriptor;
   rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

   if (constantFlag)
   {
      ++rootCount;
      // Constant buffer for the color
      rootCBVDescriptor.RegisterSpace = 0;
      rootCBVDescriptor.ShaderRegister = 1;
      rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
      rootParameters[1].Descriptor = rootCBVDescriptor;
      rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
   }

   if (textureFlag)
   {
      ++rootCount;

      D3D12_ROOT_DESCRIPTOR rootCBVDescriptor;
      rootCBVDescriptor.RegisterSpace = 0;
      rootCBVDescriptor.ShaderRegister = 0;

      // create a descriptor range (descriptor table) and fill it out
      // this is a range of descriptors inside a descriptor heap
      D3D12_DESCRIPTOR_RANGE  descriptorTableRanges[1]; // only one range right now
      descriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // this is a range of shader resource views (descriptors)
      descriptorTableRanges[0].NumDescriptors = 1; // we only have one texture right now, so the range is only 1
      descriptorTableRanges[0].BaseShaderRegister = 0; // start index of the shader registers in the range
      descriptorTableRanges[0].RegisterSpace = 0; // space 0. can usually be zero
      descriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

      // create a descriptor table
      D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
      descriptorTable.NumDescriptorRanges = _countof(descriptorTableRanges); // we only have one range
      descriptorTable.pDescriptorRanges = &descriptorTableRanges[0]; // the pointer to the beginning of our ranges array

      // fill out the parameter for our descriptor table. Remember it's a good idea to sort parameters by frequency of change. Our constant
      // buffer will be changed multiple times per frame, while our descriptor table will not be changed at all (in this tutorial)
      rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // this is a descriptor table
      rootParameters[1].DescriptorTable = descriptorTable; // this is our descriptor table for this root parameter
      rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // our pixel shader will be the only shader accessing this parameter for now
   }

   // create a static sampler
   D3D12_STATIC_SAMPLER_DESC sampler = {};
   sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_POINT;
   sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
   sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
   sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_BORDER;
   sampler.MipLODBias = 0;
   sampler.MaxAnisotropy = 0;
   sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
   sampler.BorderColor = D3D12_STATIC_BORDER_COLOR_TRANSPARENT_BLACK;
   sampler.MinLOD = 0.0f;
   sampler.MaxLOD = D3D12_FLOAT32_MAX;
   sampler.ShaderRegister = 0;
   sampler.RegisterSpace = 0;
   sampler.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

   // Create an basic root signature.
   D3D12_ROOT_SIGNATURE_DESC rsDesc;
   rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
      D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
      D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
      D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
   rsDesc.NumParameters = rootCount;
   rsDesc.pParameters = rootParameters;
   rsDesc.NumStaticSamplers = 1;
   rsDesc.pStaticSamplers = &sampler;

   ComPtr<ID3DBlob> signature;
   ComPtr<ID3DBlob> error;
   ThrowIfFailed(D3D12SerializeRootSignature(&rsDesc,
      D3D_ROOT_SIGNATURE_VERSION_1,
      &signature,
      &error));
   ThrowIfFailed(device->CreateRootSignature(0,
      signature->GetBufferPointer(),
      signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
}

void Object::CreateShader(const std::wstring &vertexPath, const std::wstring &pixelPath)
{
   ThrowIfFailed(D3DReadFileToBlob(vertexPath.c_str(), vertexShaderBlob.ReleaseAndGetAddressOf()));

   // create pixel shader
   ThrowIfFailed(D3DReadFileToBlob(pixelPath.c_str(), pixelShaderBlob.ReleaseAndGetAddressOf()));
}

void Object::LoadIndicesBuffer(const std::vector<unsigned short> &indices)
{
   indicesCount = (UINT)indices.size();
   const UINT indicesBufferSize = (UINT)(sizeof(unsigned short) * indices.size());

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
   resourceDesc.Width = indicesBufferSize;
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
      IID_PPV_ARGS(&indexDefaultBuffer)));
   indexDefaultBuffer->SetName(L"index Default Buffer");

   // Upload heap
   heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

   ThrowIfFailed(device->CreateCommittedResource(
      &heapProps,
      D3D12_HEAP_FLAG_NONE,
      &resourceDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&indexUploadBuffer)));
   indexUploadBuffer->SetName(L"index Upload Buffer");

   // copy data to the upload heap
   D3D12_SUBRESOURCE_DATA indexData = {};
   indexData.pData = indices.data(); // reinterpret_cast<BYTE *>(indicesX12);
   indexData.RowPitch = indicesBufferSize;
   indexData.SlicePitch = indicesBufferSize;

   gfx.UpdateSubresource(
      indexDefaultBuffer.Get(),
      indexUploadBuffer.Get(),
      &indexData); // pSrcData

   D3D12_RESOURCE_BARRIER resourceBarrier;
   resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
   resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
   resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
   resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
   resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
   resourceBarrier.Transition.pResource = indexDefaultBuffer.Get();
   commandList->ResourceBarrier(1, &resourceBarrier);

   indexBufferView.BufferLocation = indexDefaultBuffer->GetGPUVirtualAddress();
   indexBufferView.Format = DXGI_FORMAT_R16_UINT;
   indexBufferView.SizeInBytes = indicesBufferSize;
}

void Object::CreateTexture(const Surface &surface)
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

void Object::CreatePipelineState(const std::vector<D3D12_INPUT_ELEMENT_DESC> &inputElementDescs, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType)
{
   topology = topologyType;

   D3D12_RASTERIZER_DESC rasterizerDesc;
   ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));
   rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
   rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
   rasterizerDesc.FrontCounterClockwise = FALSE;
   rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
   rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
   rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
   rasterizerDesc.DepthClipEnable = TRUE;
   rasterizerDesc.MultisampleEnable = FALSE;
   rasterizerDesc.AntialiasedLineEnable = FALSE;
   rasterizerDesc.ForcedSampleCount = 0;
   rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

   D3D12_BLEND_DESC blendDesc;
   ZeroMemory(&blendDesc, sizeof(blendDesc));
   blendDesc.AlphaToCoverageEnable = FALSE;
   blendDesc.IndependentBlendEnable = FALSE;
   blendDesc.RenderTarget[0] = {
      FALSE,FALSE,
      D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
      D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
      D3D12_LOGIC_OP_NOOP,
      D3D12_COLOR_WRITE_ENABLE_ALL
   };

   // Depth
   D3D12_DEPTH_STENCIL_DESC depthDesc = {};
   depthDesc.DepthEnable = TRUE;
   depthDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
   depthDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
   depthDesc.StencilEnable = FALSE;
   depthDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
   depthDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
   const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp =
   { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
   depthDesc.FrontFace = defaultStencilOp;
   depthDesc.BackFace = defaultStencilOp;


   // Describe and create the graphics pipeline state object (PSO).
   D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
   psoDesc.InputLayout = { inputElementDescs.data(), (UINT)inputElementDescs.size() };
   psoDesc.pRootSignature = rootSignature.Get();
   psoDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
   psoDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
   psoDesc.RasterizerState = rasterizerDesc;
   psoDesc.BlendState = blendDesc;
   psoDesc.SampleMask = UINT_MAX;
   psoDesc.PrimitiveTopologyType = topology;
   psoDesc.NumRenderTargets = 1;
   psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
   psoDesc.SampleDesc.Count = 1;
   psoDesc.DepthStencilState = depthDesc;
   psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
   ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
}