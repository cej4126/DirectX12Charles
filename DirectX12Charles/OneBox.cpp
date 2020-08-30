#include "OneBox.h"
using namespace Microsoft::WRL;

OneBox::OneBox(Graphics &gfx)
   :
   gfx(gfx),
   device(gfx.GetDevice()),
   commandList(gfx.GetCommandList())
{
   LoadDepend();
}

void OneBox::LoadDepend()
{
   CreateRootSignature();
   CreateShader();
   LoadDrawBuffer();
   CreatePipelineState();
   CreateConstant();
}

void OneBox::Update(float dt)
{
   angle += 2.0f * dt;

   float offsetx = -4.5f;
   float offsety = 3.0f;

   matrixBuffer.transform = XMMatrixTranspose(
      XMMatrixRotationZ(angle) *
      XMMatrixRotationY(angle) *
      XMMatrixScaling(0.5f, 0.5f, 0.5f) *
      XMMatrixTranslation(offsetx, offsety, 8.0f) *
      XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 10.0f));
}

void OneBox::Draw()
{
   commandList->SetGraphicsRootSignature(rootSignature.Get());
   commandList->SetPipelineState(pipelineState.Get());

   commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
   commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
   commandList->IASetIndexBuffer(&indexBufferView);

   int ConstantBufferPerObjectAlignedSize = (sizeof(XMMATRIX) + 255) & ~255;

   commandList->SetGraphicsRootConstantBufferView(0,
      matrixBufferUploadHeaps->GetGPUVirtualAddress() + 0 * ConstantBufferPerObjectAlignedSize);

   commandList->SetGraphicsRootConstantBufferView(1,
      colorBufferUploadHeaps->GetGPUVirtualAddress() + 0 * ConstantBufferPerObjectAlignedSize);

   commandList->DrawIndexedInstanced(indicesCount, 1u, indicesStart, 0u, 0u);
}

void OneBox::LoadConstant()
{
   int ConstantBufferPerObjectAlignedSize = (sizeof(matrixBuffer) + 255) & ~255;
   memcpy(matrixBufferGPUAddress + 0 * ConstantBufferPerObjectAlignedSize, &matrixBuffer, sizeof(matrixBuffer));
}

void OneBox::CreateRootSignature()
{
   D3D12_ROOT_DESCRIPTOR rootCBVDescriptor;
   rootCBVDescriptor.RegisterSpace = 0;
   rootCBVDescriptor.ShaderRegister = 0;

   D3D12_ROOT_PARAMETER  rootParameters[2];
   // constant buffer for matrix
   rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
   rootParameters[0].Descriptor = rootCBVDescriptor;
   rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

   // Constant buffer for the color
   rootCBVDescriptor.RegisterSpace = 0;
   rootCBVDescriptor.ShaderRegister = 1;
   rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
   rootParameters[1].Descriptor = rootCBVDescriptor;
   rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

   // Create an basic root signature.
   D3D12_ROOT_SIGNATURE_DESC rsDesc;
   rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
      D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
      D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
      D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
   rsDesc.NumParameters = _countof(rootParameters);
   rsDesc.pParameters = rootParameters;
   rsDesc.NumStaticSamplers = 0;
   rsDesc.pStaticSamplers = nullptr;

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

void OneBox::LoadDrawBuffer()
{
   struct Vertex
   {
      XMFLOAT3 pos;
   };
   auto model = gfx.shape.GetShapeData<Vertex>();
   indicesStart = gfx.shape.getIndiceStart(Shape::Cube);
   indicesCount = gfx.shape.getIndiceCount(Shape::Cube);

   const UINT vertexBufferSize = (UINT)(sizeof(Vertex) * model.vertices.size());

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
   resourceDesc.Width = vertexBufferSize;
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
      IID_PPV_ARGS(&vertexDefaultBuffer)));

   // Upload heap
   heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

   ThrowIfFailed(device->CreateCommittedResource(
      &heapProps,
      D3D12_HEAP_FLAG_NONE,
      &resourceDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&vertexUploadBuffer)));

   // copy data to the upload heap
   D3D12_SUBRESOURCE_DATA vertexData = {};
   vertexData.pData = model.vertices.data(); //reinterpret_cast<BYTE *>(
   vertexData.RowPitch = vertexBufferSize;
   vertexData.SlicePitch = vertexBufferSize;

   // Add the copy to the command list
   gfx.UpdateSubresource(
      vertexDefaultBuffer.Get(),
      vertexUploadBuffer.Get(),
      &vertexData); // pSrcData

   D3D12_RESOURCE_BARRIER resourceBarrier;
   resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
   resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
   resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
   resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
   resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
   resourceBarrier.Transition.pResource = vertexDefaultBuffer.Get();
   commandList->ResourceBarrier(1, &resourceBarrier);

   // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
   vertexBufferView.BufferLocation = vertexDefaultBuffer->GetGPUVirtualAddress();
   vertexBufferView.StrideInBytes = sizeof(Vertex);
   vertexBufferView.SizeInBytes = vertexBufferSize;

   const UINT indicesBufferSize = (UINT)(sizeof(unsigned short) * model.indices.size());

   ZeroMemory(&heapProps, sizeof(heapProps));
   heapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
   heapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
   heapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
   heapProps.CreationNodeMask = 1;
   heapProps.VisibleNodeMask = 1;

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
   indexData.pData = model.indices.data(); // reinterpret_cast<BYTE *>(indicesX12);
   indexData.RowPitch = indicesBufferSize;
   indexData.SlicePitch = indicesBufferSize;

   gfx.UpdateSubresource(
      indexDefaultBuffer.Get(),
      indexUploadBuffer.Get(),
      &indexData); // pSrcData

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

void OneBox::CreateShader()
{
   ThrowIfFailed(D3DReadFileToBlob(L"ColorIndexVS.cso", vertexShaderBlob.ReleaseAndGetAddressOf()));

   // create pixel shader
   ThrowIfFailed(D3DReadFileToBlob(L"ColorIndexPS.cso", pixelShaderBlob.ReleaseAndGetAddressOf()));
}

void OneBox::CreateConstant()
{
   // Matrix Constant buffer
   D3D12_HEAP_PROPERTIES constantHeapUpload = {};
   constantHeapUpload.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
   constantHeapUpload.CreationNodeMask = 1;
   constantHeapUpload.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
   constantHeapUpload.Type = D3D12_HEAP_TYPE_UPLOAD;
   constantHeapUpload.VisibleNodeMask = 1;

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

   ThrowIfFailed(device->CreateCommittedResource(
      &constantHeapUpload,
      D3D12_HEAP_FLAG_NONE,
      &constantHeapDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&matrixBufferUploadHeaps)));

   D3D12_RANGE readRange;
   readRange.Begin = 0;
   readRange.End = 0;
   ThrowIfFailed(matrixBufferUploadHeaps->Map(0, &readRange, reinterpret_cast<void **>(&matrixBufferGPUAddress)));

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

   // Color Constant Buffer
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

   ThrowIfFailed(device->CreateCommittedResource(
      &constantHeapUpload,
      D3D12_HEAP_FLAG_NONE,
      &constantHeapDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&colorBufferUploadHeaps)));

   readRange.Begin = 1;
   readRange.End = 0;
   ThrowIfFailed(colorBufferUploadHeaps->Map(0, &readRange, reinterpret_cast<void **>(&colorBufferGPUAddress)));
   int ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBufferColor) + 255) & ~255;
   memcpy(colorBufferGPUAddress + 0 * ConstantBufferPerObjectAlignedSize, &colorBuffer, sizeof(colorBuffer));
}

void OneBox::CreatePipelineState()
{
   // Define the vertex input layout.
   D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
   {
       { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
   };

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
   psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
   psoDesc.pRootSignature = rootSignature.Get();
   psoDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
   psoDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
   psoDesc.RasterizerState = rasterizerDesc;
   psoDesc.BlendState = blendDesc;
   psoDesc.SampleMask = UINT_MAX;
   psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
   psoDesc.NumRenderTargets = 1;
   psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
   psoDesc.SampleDesc.Count = 1;
   psoDesc.DepthStencilState = depthDesc;
   psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
   ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
}

