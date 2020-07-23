#include "Graphics.h"

using namespace Microsoft::WRL;
using namespace DirectX;

Graphics::Graphics(HWND hWnd, int width, int height)
   :
   width(width),
   height(height),
   hWnd(hWnd)
{

   aspectRatio = static_cast<float>(width) / static_cast<float>(height);

   // debug 3d
#if defined(_DEBUG) 
   ComPtr <ID3D12Debug> m_debugInterface;
   ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugInterface)));
   m_debugInterface->EnableDebugLayer();
#endif

   LoadDriveX12();
   LoadBaseX12();

   if (DirectX11OnlyFlag)
   {
      LoadDriveX11Only();
   }
   else
   {
      LoadBaseX11();
   }

   if (DWriteFlag)
   {
      LoadBase2D();
   }

   LoadDepentX11();
}

void Graphics::LoadDriveX12()
{
   // Dwrite
   UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG) 
   dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
   // end Dwrite
#endif

   // factory
   ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_DxgiFactory4)));

   // adapter
   int adapterIndex = 0;
   bool adapterFound = false;
   ComPtr<IDXGIAdapter1> adapterTemp;
   HRESULT hr;
   while (m_DxgiFactory4->EnumAdapters1(adapterIndex, adapterTemp.ReleaseAndGetAddressOf()) != DXGI_ERROR_NOT_FOUND)
   {
      DXGI_ADAPTER_DESC1 desc1;
      adapterTemp->GetDesc1(&desc1);
      if (desc1.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)
      {
         continue;
      }

      hr = D3D12CreateDevice(adapterTemp.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr);
      if (SUCCEEDED(hr))
      {
         adapterFound = true;
         break;
      }
      ++adapterIndex;
   }

   if (!adapterFound)
   {
      throw;
   }

   // device
   hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
   ThrowIfFailed(hr);

   // Create Command Queue
   D3D12_COMMAND_QUEUE_DESC queueDesc;
   ZeroMemory(&queueDesc, sizeof(queueDesc));
   queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
   queueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
   queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
   queueDesc.NodeMask = 0;

   ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(commandQueue.ReleaseAndGetAddressOf())));
   commandQueue->SetName(L"Command Queue");


   // Create Swap Chains
   DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
   ZeroMemory(&swapChainDesc, sizeof(swapChainDesc));
   swapChainDesc.Width = width;
   swapChainDesc.Height = height;
   swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
   swapChainDesc.Stereo = FALSE;
   swapChainDesc.SampleDesc = { 1, 0 };
   swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   swapChainDesc.BufferCount = bufferCount;
   swapChainDesc.Scaling = DXGI_SCALING_NONE;
   swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
   swapChainDesc.Flags = 0;

   ThrowIfFailed(m_DxgiFactory4->CreateSwapChainForHwnd(commandQueue.Get(),
      hWnd,
      &swapChainDesc,
      nullptr,
      nullptr,
      swapChain1.ReleaseAndGetAddressOf()));

   ThrowIfFailed(swapChain1.As(&swapChain));

   // Get Swap Chain Buffers
   for (UINT i{ 0 }; i < bufferCount; i++)
   {
      ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(swapChainBuffers[i].ReleaseAndGetAddressOf())));
   }
   frameIndex = swapChain->GetCurrentBackBufferIndex();
}

void Graphics::LoadDriveX11Only()
{
   DXGI_SWAP_CHAIN_DESC sd = {};
   sd.BufferDesc.Width = 0;
   sd.BufferDesc.Height = 0;
   sd.BufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
   sd.BufferDesc.RefreshRate.Numerator = 0;
   sd.BufferDesc.RefreshRate.Denominator = 0;
   sd.BufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
   sd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
   sd.SampleDesc.Count = 1;
   sd.SampleDesc.Quality = 0;
   sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   sd.BufferCount = bufferCount;
   sd.OutputWindow = hWnd;
   sd.Windowed = TRUE;
   sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
   sd.Flags = 0;

   UINT swapCreateFlags = 0u;
#ifndef NDEBUG
   swapCreateFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

   // create device and front/back buffers, and swap chain and rendering context
   ThrowIfFailed(D3D11CreateDeviceAndSwapChain(
      nullptr,
      D3D_DRIVER_TYPE_HARDWARE,
      nullptr,
      swapCreateFlags,
      nullptr,
      0,
      D3D11_SDK_VERSION,
      &sd,
      &pSwap,
      &x11Device,
      nullptr,
      &x11DeviceContext
   ));

   ThrowIfFailed(pSwap.As(&swapChain));

   // gain access to texture subresource in swap chain (back buffer)
   for (int i = 0; i < bufferCount; i++)
   {
      ThrowIfFailed(pSwap->GetBuffer(0, __uuidof(ID3D11Resource), &x11BackBuffer[i]));
      ThrowIfFailed(x11Device->CreateRenderTargetView(x11BackBuffer[i].Get(), nullptr, &x11Target[i]));

   }
}

void Graphics::LoadBaseX12()
{
   // Create Descriptopr Heap RTV
   D3D12_DESCRIPTOR_HEAP_DESC heapDesc;
   ZeroMemory(&heapDesc, sizeof(heapDesc));
   heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
   heapDesc.NumDescriptors = bufferCount;
   heapDesc.NodeMask = 0;
   heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

   ThrowIfFailed(device->CreateDescriptorHeap(&heapDesc, IID_PPV_ARGS(m_rtvHeap.ReleaseAndGetAddressOf())));

   rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
   for (UINT i{ 0 }; i < bufferCount; i++)
   {
      D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
      rtvHandle.ptr += (INT64)i * (UINT64)rtvDescriptorSize;
      device->CreateRenderTargetView(swapChainBuffers[i].Get(), nullptr, rtvHandle);
   }

   // Create Command Allocators
   for (UINT i{ 0 }; i < bufferCount; i++)
   {
      ComPtr<ID3D12CommandAllocator> commandAllocator;
      ThrowIfFailed(device->CreateCommandAllocator(
         D3D12_COMMAND_LIST_TYPE_DIRECT,
         IID_PPV_ARGS(commandAllocator.ReleaseAndGetAddressOf())));

      commandAllocators.push_back(commandAllocator);
   }

   // Create Command List
   ThrowIfFailed(device->CreateCommandList(
      0,
      D3D12_COMMAND_LIST_TYPE_DIRECT,
      commandAllocators[0].Get(),
      nullptr, IID_PPV_ARGS(commandList.ReleaseAndGetAddressOf())));

   CreateFence();

   rootSignatureX12();

   CreateShaderX12();

   LoadDepentX12();

   viewport.TopLeftX = 0.0f;
   viewport.TopLeftY = 0.0f;
   viewport.Width = (float)width;
   viewport.Height = (float)height;
   viewport.MinDepth = D3D12_MIN_DEPTH;
   viewport.MaxDepth = D3D12_MAX_DEPTH;

   scissorRect.left = 0;
   scissorRect.top = 0;
   scissorRect.right = width;
   scissorRect.bottom = height;
}

void Graphics::CreateFence()
{
   // Create Fences
   for (UINT i = 0; i < bufferCount; i++)
   {
      ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence[i].ReleaseAndGetAddressOf())));
      fenceValue[i] = 0;
   }

   // Create Fence Event Handle
   fenceEventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
   if (fenceEventHandle == NULL)
   {
      throw;
   }
}

void Graphics::rootSignatureX12()
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

void Graphics::CreateShaderX12()
{
   ThrowIfFailed(D3DReadFileToBlob(L"VertexShader.cso", vertexShaderBlob.ReleaseAndGetAddressOf()));

   // create pixel shader
   ThrowIfFailed(D3DReadFileToBlob(L"PixelShader.cso", pixelShaderBlob.ReleaseAndGetAddressOf()));
}

void Graphics::LoadDepentX12()
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

   LoadVertexBuffer();
   LoadIndexBuffer();

   // create a depth stencil descriptor heap
   D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
   dsvHeapDesc.NumDescriptors = 1;
   dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
   dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
   ThrowIfFailed(device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&dsDescriptorHeap)));

   D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
   depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
   depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
   depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

   // Create Depth Stencil Buffer
   D3D12_CLEAR_VALUE depthOptimizedClearValue;
   ZeroMemory(&depthOptimizedClearValue, sizeof(depthOptimizedClearValue));
   depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
   depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
   depthOptimizedClearValue.DepthStencil.Stencil = 0;

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
   resourceDesc.Alignment = 0;
   resourceDesc.Width = width;
   resourceDesc.Height = height;
   resourceDesc.DepthOrArraySize = 1;
   resourceDesc.MipLevels = 0;
   resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
   resourceDesc.SampleDesc.Count = 1;
   resourceDesc.SampleDesc.Quality = 0;
   resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
   resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

   ThrowIfFailed(device->CreateCommittedResource(
      &heapProps,
      D3D12_HEAP_FLAG_NONE,
      &resourceDesc,
      D3D12_RESOURCE_STATE_DEPTH_WRITE,
      &depthOptimizedClearValue,
      IID_PPV_ARGS(depthStencilBuffer.ReleaseAndGetAddressOf())));
   depthStencilBuffer->SetName(L"depth stencil buffer");

   device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilDesc, dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

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

   // Now we execute the command list to upload the initial assets (triangle data)
   commandList->Close();
   ID3D12CommandList *ppCommandLists[] = { commandList.Get() };
   commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

   fenceValue[frameIndex]++;
   ThrowIfFailed(commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]));

}

void Graphics::LoadVertexBuffer()
{
   // Define the geometry for a triangle.
   Vertex verticesX12[] =
   {
      { -1.0f, -1.0f, -1.0f },
      {  1.0f, -1.0f, -1.0f },
      { -1.0f,  1.0f, -1.0f },
      {  1.0f,  1.0f, -1.0f },

      { -1.0f, -1.0f,  1.0f },
      {  1.0f, -1.0f,  1.0f },
      { -1.0f,  1.0f,  1.0f },
      {  1.0f,  1.0f,  1.0f },
   };
   const UINT vertexCount = (UINT)std::size(verticesX12);
   const UINT vertexBufferSize = sizeof(verticesX12);

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
      IID_PPV_ARGS(&vertexBuffer)));
   vertexBuffer->SetName(L"vertexBuffer");

   // Upload heap
   heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

   ThrowIfFailed(device->CreateCommittedResource(
      &heapProps,
      D3D12_HEAP_FLAG_NONE,
      &resourceDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&vertexUpload)));
   vertexUpload->SetName(L"vertexUpload");

   // copy data to the upload heap
   D3D12_SUBRESOURCE_DATA vertexData = {};
   vertexData.pData = reinterpret_cast<BYTE *>(verticesX12);
   vertexData.RowPitch = vertexBufferSize;
   vertexData.SlicePitch = vertexBufferSize;

   // Add the copy to the command list
   UpdateSubresources(commandList.Get(),
      vertexBuffer.Get(),
      vertexUpload.Get(),
      0,  // IntermediateOffset
      0,  // FirstSubresource
      1,  // NumSubresources
      &vertexData); // pSrcData

   D3D12_RESOURCE_BARRIER resourceBarrier;
   resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
   resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
   resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
   resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
   resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
   resourceBarrier.Transition.pResource = vertexBuffer.Get();
   commandList->ResourceBarrier(1, &resourceBarrier);

   // create a vertex buffer view for the triangle. We get the GPU memory address to the vertex pointer using the GetGPUVirtualAddress() method
   vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
   vertexBufferView.StrideInBytes = sizeof(Vertex);
   vertexBufferView.SizeInBytes = vertexBufferSize;
}

void Graphics::LoadIndexBuffer()
{
   const unsigned short indicesX12Right[] =
   {
      0,2,1, 2,3,1,  // Back Face
      1,3,5, 3,7,5,  // Left Face
      2,6,3, 3,6,7,  // Top Face
      4,5,7, 4,7,6,  // Front Face
      0,4,2, 2,4,6,  // Right Face
      0,1,4, 1,5,4   // Bottom Face
   };

   unsigned short indicesX12[36];
   for (int i = 0; i < 36; i++)
   {
      indicesX12[i] = indicesX12Right[i];
   }

   indicesCount = (UINT)std::size(indicesX12);
   const UINT indicesBufferSize = sizeof(indicesX12);

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
      IID_PPV_ARGS(&indexBuffer)));
   indexBuffer->SetName(L"indexBuffer");

   // Upload heap
   heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;

   ThrowIfFailed(device->CreateCommittedResource(
      &heapProps,
      D3D12_HEAP_FLAG_NONE,
      &resourceDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&indexUpload)));
   indexUpload->SetName(L"indexUpload");

   // copy data to the upload heap
   D3D12_SUBRESOURCE_DATA indexData = {};
   indexData.pData = reinterpret_cast<BYTE *>(indicesX12);
   indexData.RowPitch = indicesBufferSize;
   indexData.SlicePitch = indicesBufferSize;

   // Add the copy to the command list
   UpdateSubresources(commandList.Get(),
      indexBuffer.Get(),
      indexUpload.Get(),
      0,  // IntermediateOffset
      0,  // FirstSubresource
      1,  // NumSubresources
      &indexData); // pSrcData

   D3D12_RESOURCE_BARRIER resourceBarrier;
   resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
   resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
   resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
   resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
   resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
   resourceBarrier.Transition.pResource = indexBuffer.Get();
   commandList->ResourceBarrier(1, &resourceBarrier);

   indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
   //indexBufferView.Format = DXGI_FORMAT_R32_UINT;
   indexBufferView.Format = DXGI_FORMAT_R16_UINT;
   indexBufferView.SizeInBytes = indicesBufferSize;
}


//------------------------------------------------------------------------------------------------
// All arrays must be populated (e.g. by calling GetCopyableFootprints)
inline UINT64 Graphics::UpdateSubresources(
   _In_ ID3D12GraphicsCommandList *pCmdList,
   _In_ ID3D12Resource *pDestinationResource,
   _In_ ID3D12Resource *pIntermediate,
   _In_range_(0, D3D12_REQ_SUBRESOURCES) UINT FirstSubresource,
   _In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) UINT NumSubresources,
   UINT64 RequiredSize,
   _In_reads_(NumSubresources) const D3D12_PLACED_SUBRESOURCE_FOOTPRINT *pLayouts,
   _In_reads_(NumSubresources) const UINT *pNumRows,
   _In_reads_(NumSubresources) const UINT64 *pRowSizesInBytes,
   _In_reads_(NumSubresources) const D3D12_SUBRESOURCE_DATA *pSrcData)
{
   // part 1

    // Minor validation
   D3D12_RESOURCE_DESC IntermediateDesc = pIntermediate->GetDesc();
   D3D12_RESOURCE_DESC DestinationDesc = pDestinationResource->GetDesc();
   if (IntermediateDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER ||
      IntermediateDesc.Width < RequiredSize + pLayouts[0].Offset ||
      RequiredSize >(SIZE_T) - 1 ||
      (DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER &&
         (FirstSubresource != 0 || NumSubresources != 1)))
   {
      return 0;
   }

   BYTE *pData;
   HRESULT hr = pIntermediate->Map(0, NULL, reinterpret_cast<void **>(&pData));
   if (FAILED(hr))
   {
      return 0;
   }

   for (UINT i = 0; i < NumSubresources; ++i)
   {
      if (pRowSizesInBytes[i] > (SIZE_T)-1) return 0;
      D3D12_MEMCPY_DEST DestData = { pData + pLayouts[i].Offset, pLayouts[i].Footprint.RowPitch, pLayouts[i].Footprint.RowPitch * pNumRows[i] };
      MemcpySubresource(&DestData, &pSrcData[i], (SIZE_T)pRowSizesInBytes[i], pNumRows[i], pLayouts[i].Footprint.Depth);
   }
   pIntermediate->Unmap(0, NULL);

   if (DestinationDesc.Dimension == D3D12_RESOURCE_DIMENSION_BUFFER)
   {
      // CD3DX12_BOX SrcBox(UINT(pLayouts[0].Offset), UINT(pLayouts[0].Offset + pLayouts[0].Footprint.Width));
      D3D12_BOX SrcBox = {};
      SrcBox.top = 0;
      SrcBox.front = 0;
      SrcBox.left = UINT(pLayouts[0].Offset);
      SrcBox.right = UINT(pLayouts[0].Offset + pLayouts[0].Footprint.Width);
      SrcBox.bottom = 1;
      SrcBox.back = 1;

      pCmdList->CopyBufferRegion(
         pDestinationResource, 0, pIntermediate, pLayouts[0].Offset, pLayouts[0].Footprint.Width);
   }
   else
   {
      for (UINT i = 0; i < NumSubresources; ++i)
      {
         //CD3DX12_TEXTURE_COPY_LOCATION Dst(pDestinationResource, i + FirstSubresource);
         D3D12_TEXTURE_COPY_LOCATION Dst = {};
         Dst.pResource = pDestinationResource;
         Dst.SubresourceIndex = i + FirstSubresource;
         //CD3DX12_TEXTURE_COPY_LOCATION Src(pIntermediate, pLayouts[i]);
         D3D12_TEXTURE_COPY_LOCATION Src = {};
         Src.pResource = pIntermediate;
         Src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
         Src.PlacedFootprint = pLayouts[i];
         pCmdList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
      }
   }
   return RequiredSize;
}

//------------------------------------------------------------------------------------------------
// Heap-allocating UpdateSubresources implementation
inline UINT64 Graphics::UpdateSubresources(
   _In_ ID3D12GraphicsCommandList *pCmdList,
   _In_ ID3D12Resource *pDestinationResource,
   _In_ ID3D12Resource *pIntermediate,
   UINT64 IntermediateOffset,
   _In_range_(0, D3D12_REQ_SUBRESOURCES) UINT FirstSubresource,
   _In_range_(0, D3D12_REQ_SUBRESOURCES - FirstSubresource) UINT NumSubresources,
   _In_reads_(NumSubresources) D3D12_SUBRESOURCE_DATA *pSrcData)
{
   // part 2

   UINT64 RequiredSize = 0;
   UINT64 MemToAlloc = static_cast<UINT64>(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64)) * NumSubresources;
   if (MemToAlloc > SIZE_MAX)
   {
      return 0;
   }
   void *pMem = HeapAlloc(GetProcessHeap(), 0, static_cast<SIZE_T>(MemToAlloc));
   if (pMem == NULL)
   {
      return 0;
   }
   D3D12_PLACED_SUBRESOURCE_FOOTPRINT *pLayouts = reinterpret_cast<D3D12_PLACED_SUBRESOURCE_FOOTPRINT *>(pMem);
   UINT64 *pRowSizesInBytes = reinterpret_cast<UINT64 *>(pLayouts + NumSubresources);
   UINT *pNumRows = reinterpret_cast<UINT *>(pRowSizesInBytes + NumSubresources);

   D3D12_RESOURCE_DESC Desc = pDestinationResource->GetDesc();
   ID3D12Device *pDevice;
   pDestinationResource->GetDevice(__uuidof(*pDevice), reinterpret_cast<void **>(&pDevice));
   pDevice->GetCopyableFootprints(&Desc, FirstSubresource, NumSubresources, IntermediateOffset, pLayouts, pNumRows, pRowSizesInBytes, &RequiredSize);
   pDevice->Release();

   UINT64 Result = UpdateSubresources(pCmdList, pDestinationResource, pIntermediate, FirstSubresource, NumSubresources, RequiredSize, pLayouts, pNumRows, pRowSizesInBytes, pSrcData);
   HeapFree(GetProcessHeap(), 0, pMem);
   return Result;
}

// Row-by-row memcpy
inline void Graphics::MemcpySubresource(
   _In_ const D3D12_MEMCPY_DEST *pDest,
   _In_ const D3D12_SUBRESOURCE_DATA *pSrc,
   SIZE_T RowSizeInBytes,
   UINT NumRows,
   UINT NumSlices)
{
   for (UINT z = 0; z < NumSlices; ++z)
   {
      BYTE *pDestSlice = reinterpret_cast<BYTE *>(pDest->pData) + pDest->SlicePitch * z;
      const BYTE *pSrcSlice = reinterpret_cast<const BYTE *>(pSrc->pData) + pSrc->SlicePitch * z;
      for (UINT y = 0; y < NumRows; ++y)
      {
         memcpy(pDestSlice + pDest->RowPitch * y,
            pSrcSlice + pSrc->RowPitch * y,
            RowSizeInBytes);
      }
   }
}

void Graphics::LoadBaseX11()
{
   UINT x11DeviceFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;
#if defined(_DEBUG)
   x11DeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

   // Create an 11 device wrapped around the 12 device and share 12's command queue.
   ThrowIfFailed(D3D11On12CreateDevice(
      device.Get(),
      x11DeviceFlags,
      nullptr,
      0,
      reinterpret_cast<IUnknown **>(commandQueue.GetAddressOf()),
      1,
      0,
      &x11Device,
      &x11DeviceContext,
      nullptr
   ));

   // Query the 11On12 device from the 11 device.
   ThrowIfFailed(x11Device.As(&x11On12Device));

   // DWrite
   D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};

   ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &m_d2dFactory));
   ThrowIfFailed(x11On12Device.As(&dxgiDevice));

   // Create a RTV
   for (UINT i{ 0 }; i < bufferCount; i++)
   {
      D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
      ThrowIfFailed(x11On12Device->CreateWrappedResource(
         swapChainBuffers[i].Get(),
         &d3d11Flags,
         D3D12_RESOURCE_STATE_RENDER_TARGET,
         D3D12_RESOURCE_STATE_PRESENT,
         IID_PPV_ARGS(&x11wrappedBackBuffers[i])
      ));

      swapChainBuffers[i]->SetName(L"swap Chain Buffers");
      ThrowIfFailed(x11Device->CreateRenderTargetView(x11wrappedBackBuffers[i].Get(), nullptr, &x11Target[i]));

      // Not sure why nogo here
      //ThrowIfFailed(swapChain1->GetBuffer(i, __uuidof(ID3D11Resource), &pBackBuffer[i]));
      //ThrowIfFailed(x11Device->CreateRenderTargetView(pBackBuffer[i].Get(), nullptr, &x11Target[i]));
   }
}

void Graphics::LoadBase2D()
{
   float dpiX;
   float dpiY;

   dpiX = (float)GetDpiForWindow(hWnd);
   dpiY = dpiX;
   //m_d2dFactory->GetDesktopDpi(&dpiX, &dpiY);
   D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
      D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
      D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
      dpiX,
      dpiY
   );

   D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
   ThrowIfFailed(m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice));
   ThrowIfFailed(m_d2dDevice->CreateDeviceContext(deviceOptions, &x11d2dDeviceContext));
   ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dWriteFactory));
   for (UINT i{ 0 }; i < bufferCount; i++)
   {
      // Create a render target for D2D to draw directly to this back buffer.
      ComPtr<IDXGISurface> surface;
      ThrowIfFailed(x11wrappedBackBuffers[i].As(&surface));
      ThrowIfFailed(x11d2dDeviceContext->CreateBitmapFromDxgiSurface(
         surface.Get(),
         &bitmapProperties,
         &x11d2dRenderTargets[i]));
   }
}

void Graphics::CreateShaderX11()
{
   // create pixel shader
   ComPtr<ID3DBlob> pBlobPixel;
   ThrowIfFailed(D3DReadFileToBlob(L"PixelShaderX11.cso", &pBlobPixel));
   ThrowIfFailed(x11Device->CreatePixelShader(pBlobPixel->GetBufferPointer(), pBlobPixel->GetBufferSize(), nullptr, &x11PixelShader));

   // create vertex shader
   ComPtr<ID3DBlob> pBlobVertex;
   ThrowIfFailed(D3DReadFileToBlob(L"VertexShaderX11.cso", &pBlobVertex));
   ThrowIfFailed(x11Device->CreateVertexShader(pBlobVertex->GetBufferPointer(), pBlobVertex->GetBufferSize(), nullptr, &x11VertexShader));

   // input (vertex) layout
   const D3D11_INPUT_ELEMENT_DESC ied[] =
   {
      { "POSITIONX", 0, DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA, 0 },
   };

   ThrowIfFailed(x11Device->CreateInputLayout(
      ied, (UINT)std::size(ied),
      pBlobVertex->GetBufferPointer(),
      pBlobVertex->GetBufferSize(),
      &x11InputLayout
   ));
}

void Graphics::LoadDepentX11()
{
   CreateShaderX11();

   // create vertex buffer
   const VertexX11 verticesX11[] =
   {
      { -1.0f, -1.0f, -1.0f },
      {  1.0f, -1.0f, -1.0f },
      { -1.0f,  1.0f, -1.0f },
      {  1.0f,  1.0f, -1.0f },

      { -1.0f, -1.0f,  1.0f },
      {  1.0f, -1.0f,  1.0f },
      { -1.0f,  1.0f,  1.0f },
      {  1.0f,  1.0f,  1.0f },
   };

   D3D11_BUFFER_DESC verticesX11Desc = {};
   verticesX11Desc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
   verticesX11Desc.Usage = D3D11_USAGE_DEFAULT;
   verticesX11Desc.CPUAccessFlags = 0u;
   verticesX11Desc.MiscFlags = 0u;
   verticesX11Desc.ByteWidth = sizeof(verticesX11);
   verticesX11Desc.StructureByteStride = sizeof(VertexX11);
   D3D11_SUBRESOURCE_DATA verticeX11Data = {};
   verticeX11Data.pSysMem = verticesX11;
   ThrowIfFailed(x11Device->CreateBuffer(&verticesX11Desc, &verticeX11Data, &x11VertexBuffer));

   const unsigned short indicesX11Right[] =
   {
      0,2,1, 2,3,1,  // Back Face
      1,3,5, 3,7,5,  // Left Face
      2,6,3, 3,6,7,  // Top Face
      4,5,7, 4,7,6,  // Front Face
      0,4,2, 2,4,6,  // Right Face
      0,1,4, 1,5,4   // Bottom Face
   };

   unsigned short indicesX11[36];
   for (int i = 0; i < 36; i++)
   {
      indicesX11[i] = indicesX11Right[i];
   }

   D3D11_BUFFER_DESC indicesX11Desc = {};
   indicesX11Desc.BindFlags = D3D11_BIND_INDEX_BUFFER;
   indicesX11Desc.Usage = D3D11_USAGE_DEFAULT;
   indicesX11Desc.CPUAccessFlags = 0u;
   indicesX11Desc.MiscFlags = 0u;
   indicesX11Desc.ByteWidth = sizeof(indicesX11);
   indicesX11Desc.StructureByteStride = sizeof(unsigned short);
   D3D11_SUBRESOURCE_DATA indiceX11Data = {};
   indiceX11Data.pSysMem = indicesX11;
   ThrowIfFailed(x11Device->CreateBuffer(&indicesX11Desc, &indiceX11Data, &x11IndexBuffer));
   indiceX11Count = (UINT)std::size(indicesX11);

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
   ThrowIfFailed(x11Device->CreateBuffer(&cbd, &csd, &x11MatrixBuffer));

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
   //colorBuffer = cb;
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
   ThrowIfFailed(x11Device->CreateBuffer(&colorDesc, &colorSub, &x11ColorBuffer));

   x11ViewPort.Width = (float)width;
   x11ViewPort.Height = (float)height;
   x11ViewPort.MinDepth = 0;
   x11ViewPort.MaxDepth = 1;
   x11ViewPort.TopLeftX = 0;
   x11ViewPort.TopLeftY = 0;

   // Depth Stencil
   D3D11_DEPTH_STENCIL_DESC depthDesc = {};
   depthDesc.StencilEnable = false;
   depthDesc.DepthEnable = false;

   ThrowIfFailed(x11Device->CreateDepthStencilState(
      &depthDesc, &x11DepthStencilState));

   if (DWriteFlag)
   {
      // DWrite
      ThrowIfFailed(x11d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &x11d2dtextBrush));
      ThrowIfFailed(m_dWriteFactory->CreateTextFormat(
         L"Arial",
         NULL,
         DWRITE_FONT_WEIGHT_NORMAL,
         DWRITE_FONT_STYLE_NORMAL,
         DWRITE_FONT_STRETCH_NORMAL,
         25,
         L"en-us",
         &x11d2dtextFormat
      ));
   }
}

void Graphics::WaitForPreviousFrame()
{
   // Need to set frameIndex before call this routine

   if (DirectX12Flag || DirectX11on12Flag)
   {
      // Wait until the previous frame is finished.
      if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
      {
         ThrowIfFailed(fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent));
         WaitForSingleObject(fenceEvent, INFINITE);
      }

      fenceValue[frameIndex]++;
   }
}

void Graphics::OnRenderBegin(float angle)
{
   frameIndex = swapChain->GetCurrentBackBufferIndex();
   WaitForPreviousFrame();

   if (DirectX11on12Flag || DirectX12Flag)
   {
      OnRenderX12(angle);
   }

   if (DWriteFlag)
   {
      x11On12Device->AcquireWrappedResources(x11wrappedBackBuffers[frameIndex].GetAddressOf(), 1);
      OnRender2DWrite();
   }
}

void Graphics::OnRender(float angle)
{
   if (DirectX11OnlyFlag || DirectX11on12Flag)
   {
      OnRenderX11(angle);
   }
}

void Graphics::OnRenderEnd(float angle)
{
   if (DWriteFlag)
   {
      x11On12Device->ReleaseWrappedResources(x11wrappedBackBuffers[frameIndex].GetAddressOf(), 1);
      x11DeviceContext->Flush();
   }

   if (DirectX12Flag || DirectX11on12Flag)
   {
      ThrowIfFailed(commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]));
   }

   // Present the frame.
   ThrowIfFailed(swapChain->Present(1, 0));

}

void Graphics::OnRenderX12(float angle)
{
   ThrowIfFailed(commandAllocators[frameIndex]->Reset());
   ThrowIfFailed(commandList->Reset(commandAllocators[frameIndex].Get(), pipelineState.Get()));

   D3D12_RESOURCE_BARRIER resourceBarrier;
   resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
   resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
   resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
   resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
   resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
   resourceBarrier.Transition.pResource = swapChainBuffers[frameIndex].Get();
   commandList->ResourceBarrier(1, &resourceBarrier);

   // Indicate that the back buffer will be used as a render target.
   D3D12_CPU_DESCRIPTOR_HANDLE renderTargetDesc = m_rtvHeap->GetCPUDescriptorHandleForHeapStart();
   renderTargetDesc.ptr += (SIZE_T)frameIndex * (SIZE_T)rtvDescriptorSize;

   D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle = dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
   commandList->OMSetRenderTargets(1, &renderTargetDesc, FALSE, &dsvHandle);

   // Record commands.
   const float clearColor[] = { 0.1f, 0.1f, 0.1f, 1.0f };
   commandList->ClearRenderTargetView(renderTargetDesc, clearColor, 0, nullptr);
   commandList->ClearDepthStencilView(dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart(), D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);
   commandList->SetGraphicsRootSignature(rootSignature.Get());

   // Set necessary state.
   commandList->RSSetViewports(1, &viewport);
   commandList->RSSetScissorRects(1, &scissorRect);

   commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
   commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
   commandList->IASetIndexBuffer(&indexBufferView);

   float offsetx = -3.0f;
   float offsety = 0.0f;

   matrixBuffer.transform = XMMatrixTranspose(
      XMMatrixRotationZ(angle) *
      XMMatrixRotationY(angle) *
      XMMatrixScaling(0.5f, 0.5f, 0.5f) *
      XMMatrixTranslation(offsetx, offsety, 8.0f) *
      XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 10.0f));

   memcpy(matrixBufferGPUAddress + 0 * ConstantBufferPerObjectAlignedSize, &matrixBuffer, sizeof(matrixBuffer));
   commandList->SetGraphicsRootConstantBufferView(0,
      matrixBufferUploadHeaps->GetGPUVirtualAddress() + 0 * ConstantBufferPerObjectAlignedSize);

   memcpy(colorBufferGPUAddress + 0 * ConstantBufferPerObjectAlignedSize, &colorBuffer, sizeof(colorBuffer));

   commandList->SetGraphicsRootConstantBufferView(1,
      colorBufferUploadHeaps->GetGPUVirtualAddress() + 0 * ConstantBufferPerObjectAlignedSize);

   if (DirectX12Flag)
   {
      commandList->DrawIndexedInstanced(indicesCount, 1u, 0u, 0u, 0u);
   }

   // Indicate that the back buffer will now be used to present.
   if (!DWriteFlag)
   {
      resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
      resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
      resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
      resourceBarrier.Transition.pResource = swapChainBuffers[frameIndex].Get();
      commandList->ResourceBarrier(1, &resourceBarrier);
   }

   ThrowIfFailed(commandList->Close());

   // Execute the command list.
   ID3D12CommandList *ppCommandLists[] = { commandList.Get() };
   commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void Graphics::OnRenderX11(float angle)
{
   // X11 only
   if (DirectX11OnlyFlag)
   {
      const float color[] = { 0.1f, 0.1f, 0.1f, 1.0f };
      x11DeviceContext->ClearRenderTargetView(x11Target[frameIndex].Get(), color);
      //x11DeviceContext->ClearDepthStencilView(pDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0u);
   }

   // Bind vertex buffer to pipeline
   const UINT stride = sizeof(VertexX11);
   const UINT offset = 0u;
   x11DeviceContext->IASetVertexBuffers(0u, 1u, x11VertexBuffer.GetAddressOf(), &stride, &offset);
   x11DeviceContext->IASetIndexBuffer(x11IndexBuffer.Get(), DXGI_FORMAT_R16_UINT, 0u);

   float offsetx = 3.0f;
   float offsety = 0.0f;
   matrixBuffer.transform = XMMatrixTranspose(
      XMMatrixRotationZ(angle) *
      XMMatrixRotationY(angle) *
      XMMatrixScaling(0.5f, 0.5f, 0.5f) *
      XMMatrixTranslation(offsetx, offsety, 8.0f) *
      XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 10.0f));

   D3D11_MAPPED_SUBRESOURCE msr;
   ThrowIfFailed(x11DeviceContext->Map(
      x11MatrixBuffer.Get(), 0u,
      D3D11_MAP_WRITE_DISCARD, 0u,
      &msr
   ));
   memcpy(msr.pData, &matrixBuffer, sizeof(matrixBuffer));
   x11DeviceContext->Unmap(x11MatrixBuffer.Get(), 0u);
   x11DeviceContext->VSSetConstantBuffers(0u, 1u, x11MatrixBuffer.GetAddressOf());

   D3D11_MAPPED_SUBRESOURCE msrColor;
   ThrowIfFailed(x11DeviceContext->Map(
      x11ColorBuffer.Get(), 0u,
      D3D11_MAP_WRITE_DISCARD, 0u,
      &msrColor
   ));
   memcpy(msrColor.pData, &colorBuffer, sizeof(colorBuffer));
   x11DeviceContext->Unmap(x11ColorBuffer.Get(), 0u);
   x11DeviceContext->PSSetConstantBuffers(0u, 1u, x11ColorBuffer.GetAddressOf());

   // bind pixel shader
   x11DeviceContext->PSSetShader(x11PixelShader.Get(), nullptr, 0u);

   // bind vertex shader
   x11DeviceContext->VSSetShader(x11VertexShader.Get(), nullptr, 0u);

   // bind vertex layout
   x11DeviceContext->IASetInputLayout(x11InputLayout.Get());

   x11DeviceContext->OMSetDepthStencilState(x11DepthStencilState.Get(), 0u);

   // bind render target
   x11DeviceContext->OMSetRenderTargets(1u, x11Target[frameIndex].GetAddressOf(), nullptr);

   // Set primitive topology to triangle list (groups of 3 vertices)
   x11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

   x11DeviceContext->RSSetViewports(1u, &x11ViewPort);

   if (DirectX11Flag)
   {
      x11DeviceContext->DrawIndexed(indiceX11Count, 0u, 0u);
   }

   ////test
   //offsetx = 0.0f;
   //offsety = 1.0f;
   //matrixBuffer.transform = XMMatrixTranspose(
   //   XMMatrixRotationZ(angle) *
   //   XMMatrixRotationY(angle) *
   //   XMMatrixScaling(0.5f, 0.5f, 0.5f) *
   //   XMMatrixTranslation(offsetx, offsety, 8.0f) *
   //   XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 10.0f));

   //ThrowIfFailed(x11DeviceContext->Map(
   //   x11MatrixBuffer.Get(), 0u,
   //   D3D11_MAP_WRITE_DISCARD, 0u,
   //   &msr
   //));
   //memcpy(msr.pData, &matrixBuffer, sizeof(matrixBuffer));
   //x11DeviceContext->Unmap(x11MatrixBuffer.Get(), 0u);
   //x11DeviceContext->VSSetConstantBuffers(0u, 1u, x11MatrixBuffer.GetAddressOf());


}

void Graphics::OnRender2DWrite()
{
   // DWrite

   D2D1_SIZE_F rtSize = x11d2dRenderTargets[frameIndex]->GetSize();
   D2D1_RECT_F textRect = D2D1::RectF(20, 20, rtSize.width, rtSize.height);
   static const WCHAR text[] = L"Charles was here";

   // Render text directly to the back buffer.
   x11d2dDeviceContext->SetTarget(x11d2dRenderTargets[frameIndex].Get());
   x11d2dDeviceContext->BeginDraw();
   x11d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
   x11d2dDeviceContext->DrawText(
      text,
      _countof(text) - 1,
      x11d2dtextFormat.Get(),
      &textRect,
      x11d2dtextBrush.Get()
   );
   ThrowIfFailed(x11d2dDeviceContext->EndDraw());
}

void Graphics::CleanUp()
{
   if (DirectX11on12Flag || DirectX12Flag)
   {
      // wait for the gpu to finish all frames
      for (int i = 0; i < bufferCount; ++i)
      {
         frameIndex = i;
         WaitForPreviousFrame();
         ThrowIfFailed(commandAllocators[frameIndex]->Reset());
         ThrowIfFailed(commandList->Reset(commandAllocators[frameIndex].Get(), pipelineState.Get()));
         ThrowIfFailed(commandList->Close());
      }

      for (int i = 0; i < bufferCount; ++i)
      {
         frameIndex = i;
         ThrowIfFailed(commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]));
         WaitForPreviousFrame();
      }
   }
}

void Graphics::DrawIndexed(UINT count) noexcept
{
   x11DeviceContext->DrawIndexed(count, 0u, 0u);
}
