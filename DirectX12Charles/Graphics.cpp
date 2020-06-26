#include "Graphics.h"

using namespace Microsoft::WRL;

Graphics::Graphics(HWND hWnd, int width, int height)
   :
   swapChainBuffers(bufferCount),
   width(width),
   height(height),
   hWnd(hWnd)
{

   aspectRatio = static_cast<float>(width) / static_cast<float>(height);

   // debug 3d
//#if defined(_DEBUG) 
//   ComPtr <ID3D12Debug> m_debugInterface;
//   ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugInterface)));
//   m_debugInterface->EnableDebugLayer();
//#endif


   LoadDriveX12();

   LoadBaseX12();
   CreateFence();
   LoadDepentX12();

//   LoadDriveX11test();

   LoadBaseX11();

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

}

void Graphics::LoadDriveX11test()
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

   // for checking results of d3d functions
   //HRESULT hr;

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

   // Create Descriptor Heap Depth Stencil
   ZeroMemory(&heapDesc, sizeof(heapDesc));
   heapDesc.NumDescriptors = 1;
   heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
   heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

   ThrowIfFailed(device->CreateDescriptorHeap(
      &heapDesc,
      IID_PPV_ARGS(descHeapDepthStencil.ReleaseAndGetAddressOf())));

   D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc;
   ZeroMemory(&depthStencilViewDesc, sizeof(depthStencilViewDesc));
   depthStencilViewDesc.Format = DXGI_FORMAT_D32_FLOAT;
   depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
   depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;

   device->CreateDepthStencilView(depthStencilBuffer.Get(),
      &depthStencilViewDesc,
      descHeapDepthStencil->GetCPUDescriptorHandleForHeapStart());

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

   ThrowIfFailed(commandList->Close());

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
   for (UINT i{ 0 }; i < bufferCount; i++)
   {
      UINT64 initialValue{ 0 };
      ComPtr<ID3D12Fence> fence;
      ThrowIfFailed(device->CreateFence(initialValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(fence.ReleaseAndGetAddressOf())));

      fences.push_back(fence);
      fenceValues.push_back(initialValue);
   }

   // Create Fence Event Handle
   fenceEventHandle = CreateEvent(nullptr, FALSE, FALSE, nullptr);
   if (fenceEventHandle == NULL)
   {
      throw;
   }
}

void Graphics::LoadDepentX12()
{
   // Create an basic root signature.
   D3D12_ROOT_SIGNATURE_DESC rsDesc;
   rsDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
   rsDesc.NumParameters = 0;
   rsDesc.pParameters = nullptr;
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

   // create vertex shader
   ThrowIfFailed(D3DReadFileToBlob(L"VertexShader.cso", vertexShaderBlob.ReleaseAndGetAddressOf()));

   // create pixel shader
   ThrowIfFailed(D3DReadFileToBlob(L"PixelShader.cso", pixelShaderBlob.ReleaseAndGetAddressOf()));

   // Define the vertex input layout.
   D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
   {
       { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
       { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
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

   // Describe and create the graphics pipeline state object (PSO).
   D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
   psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
   psoDesc.pRootSignature = rootSignature.Get();
   psoDesc.VS = { vertexShaderBlob->GetBufferPointer(), vertexShaderBlob->GetBufferSize() };
   psoDesc.PS = { pixelShaderBlob->GetBufferPointer(), pixelShaderBlob->GetBufferSize() };
   psoDesc.RasterizerState = rasterizerDesc;
   psoDesc.BlendState = blendDesc;
   psoDesc.DepthStencilState.DepthEnable = FALSE;
   psoDesc.DepthStencilState.StencilEnable = FALSE;
   psoDesc.SampleMask = UINT_MAX;
   psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
   psoDesc.NumRenderTargets = 1;
   psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
   psoDesc.SampleDesc.Count = 1;
   ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));

   // Create the command list.
   ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocators[0].Get(), pipelineState.Get(), IID_PPV_ARGS(&commandList)));

   // Command lists are created in the recording state, but there is nothing
   // to record yet. The main loop expects it to be closed, so close it now.
   ThrowIfFailed(commandList->Close());

   // Define the geometry for a triangle.
   float offsetx = -0.5f;
   float offsety = 0.5f;
   float size = 0.15f;
   Vertex triangleVertices[] =
   {
       { { 0.0f + offsetx, (size + offsety) * aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
       { { size + offsetx, (-size + offsety)* aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
       { { -size + offsetx, (-size + offsety) * aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
   };

   const UINT vertexBufferSize = sizeof(triangleVertices);

   D3D12_HEAP_PROPERTIES heapProps;
   ZeroMemory(&heapProps, sizeof(heapProps));
   heapProps.Type = D3D12_HEAP_TYPE_UPLOAD;
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

   // Note: using upload heaps to transfer static data like vert buffers is not 
   // recommended. Every time the GPU needs it, the upload heap will be marshalled 
   // over. Please read up on Default Heap usage. An upload heap is used here for 
   // code simplicity and because there are very few verts to actually transfer.
   ThrowIfFailed(device->CreateCommittedResource(
      &heapProps,
      D3D12_HEAP_FLAG_NONE,
      &resourceDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&vertexBuffer)));

   // Copy the triangle data to the vertex buffer.
   UINT8 *pVertexDataBegin;
   D3D12_RANGE readRange;         // We do not intend to read from this resource on the CPU.
   readRange.Begin = 0;
   readRange.End = 0;
   ThrowIfFailed(vertexBuffer->Map(0, &readRange, reinterpret_cast<void **>(&pVertexDataBegin)));
   memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
   vertexBuffer->Unmap(0, nullptr);

   // Initialize the vertex buffer view.
   vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
   vertexBufferView.StrideInBytes = sizeof(Vertex);
   vertexBufferView.SizeInBytes = vertexBufferSize;

   // Create synchronization objects and wait until assets have been uploaded to the GPU.
   ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
   fenceValue = 1;

   // Create an event handle to use for frame synchronization.
   fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
   if (fenceEvent == nullptr)
   {
      ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
   }


   // Wait for the command list to execute; we are reusing the same command 
   // list in our main loop but for now, we just want to wait for setup to 
   // complete before continuing.
   WaitForPreviousFrame();
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

   D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
   ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &m_d2dFactory));
   ComPtr<IDXGIDevice> dxgiDevice;
   ThrowIfFailed(x11On12Device.As(&dxgiDevice));
   ThrowIfFailed(m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice));
   ThrowIfFailed(m_d2dDevice->CreateDeviceContext(deviceOptions, &x11d2dDeviceContext));
   ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dWriteFactory));


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

 

   //// Create a RTV
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

      // Create a render target for D2D to draw directly to this back buffer.
      ComPtr<IDXGISurface> surface;
      ThrowIfFailed(x11wrappedBackBuffers[i].As(&surface));
      ThrowIfFailed(x11d2dDeviceContext->CreateBitmapFromDxgiSurface(
         surface.Get(),
         &bitmapProperties,
         &x11d2dRenderTargets[i]));

      ThrowIfFailed(x11Device->CreateRenderTargetView(x11wrappedBackBuffers[i].Get(), nullptr, &x11Target[i]));

      //ThrowIfFailed(swapChain1->GetBuffer(i, __uuidof(ID3D11Resource), &pBackBuffer[i]));
      //ThrowIfFailed(x11Device->CreateRenderTargetView(pBackBuffer[i].Get(), nullptr, &x11Target[i]));
   }

   // create vertex buffer (1 2d triangle at center of screen)
   const VertexX11 vertices[] =
   {
      { 0.0f,0.5f,1.0f,0.0f,0.0f },
      { 0.5f,-0.5f,0.0f,1.0f,0.0f },
      { -0.5f,-0.5f,0.0f,0.0f,1.0f },
   };

   verticesCount = (UINT)std::size(vertices);

   D3D11_BUFFER_DESC bd = {};
   bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
   bd.Usage = D3D11_USAGE_DEFAULT;
   bd.CPUAccessFlags = 0u;
   bd.MiscFlags = 0u;
   bd.ByteWidth = sizeof(vertices);
   bd.StructureByteStride = sizeof(VertexX11);
   D3D11_SUBRESOURCE_DATA sd = {};
   sd.pSysMem = vertices;
   ThrowIfFailed(x11Device->CreateBuffer(&bd, &sd, &x11VertexBuffer));

   // create pixel shader
   ComPtr<ID3DBlob> pBlob;
   ThrowIfFailed(D3DReadFileToBlob(L"PixelShaderX11.cso", &pBlob));
   ThrowIfFailed(x11Device->CreatePixelShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &x11PixelShader));

   // create vertex shader
   ThrowIfFailed(D3DReadFileToBlob(L"VertexShaderX11.cso", &pBlob));
   ThrowIfFailed(x11Device->CreateVertexShader(pBlob->GetBufferPointer(), pBlob->GetBufferSize(), nullptr, &x11VertexShader));

   // input (vertex) layout (2d position only)
   const D3D11_INPUT_ELEMENT_DESC ied[] =
   {
      { "POSITION",0,DXGI_FORMAT_R32G32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
      { "COLOR",0,DXGI_FORMAT_R32G32B32_FLOAT,0,8u,D3D11_INPUT_PER_VERTEX_DATA,0 },
   };

   ThrowIfFailed(x11Device->CreateInputLayout(
      ied, (UINT)std::size(ied),
      pBlob->GetBufferPointer(),
      pBlob->GetBufferSize(),
      &x11InputLayout
   ));

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

   // DWrite
   ThrowIfFailed(x11d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::DarkRed), &x11d2dtextBrush));
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

void Graphics::WaitForPreviousFrame()
{
   // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
// This is code implemented as such for simplicity. The D3D12HelloFrameBuffering
// sample illustrates how to use fences for efficient resource usage and to
// maximize GPU utilization.

// Signal and increment the fence value.
   const UINT64 fence = fenceValue;
   ThrowIfFailed(commandQueue->Signal(m_fence.Get(), fence));
   fenceValue++;

   // Wait until the previous frame is finished.
   if (m_fence->GetCompletedValue() < fence)
   {
      ThrowIfFailed(m_fence->SetEventOnCompletion(fence, fenceEvent));
      WaitForSingleObject(fenceEvent, INFINITE);
   }

   frameIndex = swapChain->GetCurrentBackBufferIndex();
}

void Graphics::OnRender()
{
   frameIndex = swapChain->GetCurrentBackBufferIndex();
   //frameIndex = pSwap->GetCurrentBackBufferIndex();
   // DirectX 12
   OnRenderX12();

   // DirectX 11
   OnRenderX11();


   // Present the frame.
   ThrowIfFailed(swapChain->Present(1, 0));

   WaitForPreviousFrame();
}

void Graphics::OnRenderX12()
{
   // Command list allocators can only be reset when the associated 
   // command lists have finished execution on the GPU; apps should use 
   // fences to determine GPU execution progress.
   ThrowIfFailed(commandAllocators[frameIndex]->Reset());

   // However, when ExecuteCommandList() is called on a particular command 
   // list, that command list can then be reset at any time and must be before 
   // re-recording.
   ThrowIfFailed(commandList->Reset(commandAllocators[frameIndex].Get(), pipelineState.Get()));

   // Set necessary state.
   commandList->SetGraphicsRootSignature(rootSignature.Get());
   commandList->RSSetViewports(1, &viewport);
   commandList->RSSetScissorRects(1, &scissorRect);

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
   commandList->OMSetRenderTargets(1, &renderTargetDesc, FALSE, nullptr);

   // Record commands.
   const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
   commandList->ClearRenderTargetView(renderTargetDesc, clearColor, 0, nullptr);
   commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
   commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
   commandList->DrawInstanced(3, 1, 0, 0);


   // Indicate that the back buffer will now be used to present.

#define X11_ICLUDED
#ifdef X11_ICLUDED
   resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
   resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
   resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
   resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
   resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
   resourceBarrier.Transition.pResource = swapChainBuffers[frameIndex].Get();
   commandList->ResourceBarrier(1, &resourceBarrier);
#endif

   ThrowIfFailed(commandList->Close());

   // Execute the command list.
   ID3D12CommandList *ppCommandLists[] = { commandList.Get() };
   commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void Graphics::OnRenderX11()
{
   const float color[] = { 0.2f, 0.2f, 1.0f, 0.7f };
   x11DeviceContext->ClearRenderTargetView(x11Target[frameIndex].Get(), color);

   x11On12Device->AcquireWrappedResources(x11wrappedBackBuffers[frameIndex].GetAddressOf(), 1);

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

   // X11


   // Bind vertex buffer to pipeline
   const UINT stride = sizeof(VertexX11);
   const UINT offset = 0u;
   x11DeviceContext->IASetVertexBuffers(0u, 1u, x11VertexBuffer.GetAddressOf(), &stride, &offset);

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

   x11DeviceContext->Draw(verticesCount, 0u);

   x11On12Device->ReleaseWrappedResources(x11wrappedBackBuffers[frameIndex].GetAddressOf(), 1);

   x11DeviceContext->Flush();

}
