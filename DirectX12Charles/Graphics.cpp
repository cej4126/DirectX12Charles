#include "Graphics.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

using namespace Microsoft::WRL;
using namespace DirectX;

Graphics::Graphics(HWND hWnd, int width, int height)
   :
   width(width),
   height(height),
   hWnd(hWnd)
{
   // debug 3d
#if defined(_DEBUG) 
   ComPtr <ID3D12Debug> m_debugInterface;
   ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&m_debugInterface)));
   m_debugInterface->EnableDebugLayer();
#endif


   LoadDevice();
   LoadBase();
   LoadBaseX11();
   LoadBase2D();

   // Setup Dear ImGui context
   IMGUI_CHECKVERSION();
   ImGui::CreateContext();
   ImGuiIO &io = ImGui::GetIO(); //(void)io;
   io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
   // Setup Dear ImGui style
   ImGui::StyleColorsDark();

   // Setup Platform/Renderer bindings
   ImGui_ImplWin32_Init(hWnd);
   ImGui_ImplDX11_Init(x11Device.Get(), x11DeviceContext.Get());
}

Graphics::~Graphics()
{
   ImGui_ImplDX11_Shutdown();
}

void Graphics::LoadDevice()
{
   UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG) 
   dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
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
   //hr = D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
   hr = D3D12CreateDevice(adapterTemp.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device));
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

void Graphics::LoadBase()
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

   LoadDepent();

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

void Graphics::LoadDepent()
{
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

   D3D12_HEAP_PROPERTIES depthheapProps;
   ZeroMemory(&depthheapProps, sizeof(depthheapProps));
   depthheapProps.Type = D3D12_HEAP_TYPE_DEFAULT;
   depthheapProps.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
   depthheapProps.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
   depthheapProps.CreationNodeMask = 1;
   depthheapProps.VisibleNodeMask = 1;

   D3D12_RESOURCE_DESC depthresourceDesc;
   ZeroMemory(&depthresourceDesc, sizeof(depthresourceDesc));
   depthresourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
   depthresourceDesc.Alignment = 0;
   depthresourceDesc.Width = width;
   depthresourceDesc.Height = height;
   depthresourceDesc.DepthOrArraySize = 1;
   depthresourceDesc.MipLevels = 0;
   depthresourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
   depthresourceDesc.SampleDesc.Count = 1;
   depthresourceDesc.SampleDesc.Quality = 0;
   depthresourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
   depthresourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

   ThrowIfFailed(device->CreateCommittedResource(
      &depthheapProps,
      D3D12_HEAP_FLAG_NONE,
      &depthresourceDesc,
      D3D12_RESOURCE_STATE_DEPTH_WRITE,
      &depthOptimizedClearValue,
      IID_PPV_ARGS(depthStencilBuffer.ReleaseAndGetAddressOf())));
   depthStencilBuffer->SetName(L"depth stencil buffer");

   device->CreateDepthStencilView(depthStencilBuffer.Get(), &depthStencilDesc, dsDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
}

void Graphics::CreateMatrixConstant(UINT count)
{
   // I think the default buffer is 4K
   int ConstantBufferPerObjectAlignedSize = (sizeof(XMMATRIX) + 255) & ~255;
   //   if (count * ConstantBufferPerObjectAlignedSize > 4096)
   //   {
   //      throw;
   //   }
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
      IID_PPV_ARGS(&MatrixBufferUploadHeaps)));

   D3D12_RANGE readRange;
   readRange.Begin = 0;
   readRange.End = 0;
   ThrowIfFailed(MatrixBufferUploadHeaps->Map(0, &readRange, reinterpret_cast<void **>(&matrixBufferGPUAddress)));
}

void Graphics::SetMatrixConstant(UINT index, TransformMatrix matrix, int rootVS, int rootPS) noexcept
{

   int ConstantBufferPerObjectAlignedSize = (sizeof(matrix) + 255) & ~255;


   if (rootVS >= 0)
   {
      commandList->SetGraphicsRootConstantBufferView(rootVS,
         MatrixBufferUploadHeaps->GetGPUVirtualAddress() + index * ConstantBufferPerObjectAlignedSize);
   }

   if (rootPS >= 0)
   {
      commandList->SetGraphicsRootConstantBufferView(rootPS,
         MatrixBufferUploadHeaps->GetGPUVirtualAddress() + index * ConstantBufferPerObjectAlignedSize);
   }

   memcpy(matrixBufferGPUAddress + index * ConstantBufferPerObjectAlignedSize, &matrix, sizeof(matrix));
}

void Graphics::CreateMaterialConstant(UINT count)
{
   // I think the default buffer is 4K
   int ConstantBufferPerObjectAlignedSize = (sizeof(MaterialType) + 255) & ~255;

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
   //constantHeapDesc.Width = D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
   constantHeapDesc.Width = 3 * D3D12_DEFAULT_RESOURCE_PLACEMENT_ALIGNMENT;
   constantHeapDesc.MipLevels = 1;

   ThrowIfFailed(device->CreateCommittedResource(
      &constantHeapUpload,
      D3D12_HEAP_FLAG_NONE,
      &constantHeapDesc,
      D3D12_RESOURCE_STATE_GENERIC_READ,
      nullptr,
      IID_PPV_ARGS(&MaterialBufferUploadHeaps)));

   D3D12_RANGE readRange;
   readRange.Begin = 0;
   readRange.End = 0;
   ThrowIfFailed(MaterialBufferUploadHeaps->Map(0, &readRange, reinterpret_cast<void **>(&MaterialBufferGPUAddress)));
}

void Graphics::CopyMaterialConstant(UINT index, MaterialType &material) noexcept
{
   int ConstantBufferPerObjectAlignedSize = (sizeof(MaterialType) + 255) & ~255;
   memcpy(MaterialBufferGPUAddress + index * ConstantBufferPerObjectAlignedSize, &material, sizeof(material));
}

void Graphics::SetMaterialConstant(UINT index) noexcept
{
   int ConstantBufferPerObjectAlignedSize = (sizeof(MaterialType) + 255) & ~255;

   commandList->SetGraphicsRootConstantBufferView(2,
      MaterialBufferUploadHeaps->GetGPUVirtualAddress() + index * ConstantBufferPerObjectAlignedSize);
}

void Graphics::RunCommandList()
{
   // Now we execute the command list to upload the initial assets (triangle data)
   commandList->Close();
   ID3D12CommandList *ppCommandLists[] = { commandList.Get() };
   commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

   fenceValue[frameIndex]++;
   ThrowIfFailed(commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]));
}


UINT64 Graphics::UpdateSubresource(
   _In_ ID3D12Resource *pDefaultBuffer,
   _In_ ID3D12Resource *pUploadBuffer,
   _In_reads_(1) D3D12_SUBRESOURCE_DATA *pSrcData)
{
   UINT64 RequiredSize = 0;
   UINT64 MemToAlloc = static_cast<UINT64>(sizeof(D3D12_PLACED_SUBRESOURCE_FOOTPRINT) + sizeof(UINT) + sizeof(UINT64));
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
   UINT64 *pRowSizesInBytes = reinterpret_cast<UINT64 *>(pLayouts + 1);
   UINT *pNumRows = reinterpret_cast<UINT *>(pRowSizesInBytes + 1);

   D3D12_RESOURCE_DESC Desc = pDefaultBuffer->GetDesc();
   ID3D12Device *pDevice;
   pDefaultBuffer->GetDevice(__uuidof(*pDevice), reinterpret_cast<void **>(&pDevice));
   pDevice->GetCopyableFootprints(&Desc,
      0,  // FirstSubresource,
      1,  // NumSubresources,
      0,  // IntermediateOffset,
      pLayouts, pNumRows,
      pRowSizesInBytes,
      &RequiredSize);
   pDevice->Release();

   // Minor validation
   D3D12_RESOURCE_DESC IntermediateDesc = pUploadBuffer->GetDesc();
   D3D12_RESOURCE_DESC DestinationDesc = pDefaultBuffer->GetDesc();
   if (IntermediateDesc.Dimension != D3D12_RESOURCE_DIMENSION_BUFFER ||
      IntermediateDesc.Width < RequiredSize + pLayouts[0].Offset ||
      RequiredSize >(SIZE_T) - 1)
   {
      return 0;
   }

   BYTE *pData;
   HRESULT hr = pUploadBuffer->Map(0, NULL, reinterpret_cast<void **>(&pData));
   if (FAILED(hr))
   {
      return 0;
   }

   if (pRowSizesInBytes[0] > (SIZE_T)-1)
   {
      return 0;
   }
   D3D12_MEMCPY_DEST DestData = { pData + pLayouts[0].Offset, pLayouts[0].Footprint.RowPitch, pLayouts[0].Footprint.RowPitch * pNumRows[0] };
   for (UINT z = 0; z < pLayouts[0].Footprint.Depth; ++z)
   {
      BYTE *pDestSlice = reinterpret_cast<BYTE *>(DestData.pData) + DestData.SlicePitch * z;
      const BYTE *pSrcSlice = reinterpret_cast<const BYTE *>(pSrcData[0].pData) + pSrcData[0].SlicePitch * z;
      for (UINT y = 0; y < pNumRows[0]; ++y)
      {
         memcpy(pDestSlice + DestData.RowPitch * y,
            pSrcSlice + pSrcData[0].RowPitch * y,
            (SIZE_T)pRowSizesInBytes[0]);
      }
   }
   pUploadBuffer->Unmap(0, NULL);

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

      commandList->CopyBufferRegion(
         pDefaultBuffer, 0, pUploadBuffer, pLayouts[0].Offset, pLayouts[0].Footprint.Width);
   }
   else
   {
      // Not used
      D3D12_TEXTURE_COPY_LOCATION Dst = {};
      Dst.pResource = pDefaultBuffer;
      Dst.SubresourceIndex = 0;
      D3D12_TEXTURE_COPY_LOCATION Src = {};
      Src.pResource = pUploadBuffer;
      Src.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
      Src.PlacedFootprint = pLayouts[0];
      commandList->CopyTextureRegion(&Dst, 0, 0, 0, &Src, nullptr);
   }
   HeapFree(GetProcessHeap(), 0, pMem);
   return RequiredSize;
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
   }

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
}

void Graphics::LoadBase2D()
{
   float dpiX;
   float dpiY;

   dpiX = (float)GetDpiForWindow(hWnd);
   dpiY = dpiX;
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

void Graphics::WaitForPreviousFrame()
{
   // Need to set frameIndex before call this routine
   // Wait until the previous frame is finished.
   if (fence[frameIndex]->GetCompletedValue() < fenceValue[frameIndex])
   {
      ThrowIfFailed(fence[frameIndex]->SetEventOnCompletion(fenceValue[frameIndex], fenceEvent));
      WaitForSingleObject(fenceEvent, INFINITE);
   }

   fenceValue[frameIndex]++;
}

void Graphics::OnRenderBegin()
{
   frameIndex = swapChain->GetCurrentBackBufferIndex();
   WaitForPreviousFrame();

   OnRender(angle);
}

void Graphics::OnRender()
{
   x11On12Device->AcquireWrappedResources(x11wrappedBackBuffers[frameIndex].GetAddressOf(), 1);
   OnRender2DWrite();
   OnRenderX11();
}

void Graphics::DrawCommandList()
{
#ifdef NOT_USE_DWRITE
   // Indicate that the back buffer will now be used to present.
   if (!DWriteFlag)
   {
      D3D12_RESOURCE_BARRIER resourceBarrier;

      resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
      resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
      resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
      resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
      resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
      resourceBarrier.Transition.pResource = swapChainBuffers[frameIndex].Get();
      commandList->ResourceBarrier(1, &resourceBarrier);
   }
#endif

   ThrowIfFailed(commandList->Close());

   // Execute the command list.
   ID3D12CommandList *ppCommandLists[] = { commandList.Get() };
   commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void Graphics::OnRenderEnd()
{
   x11On12Device->ReleaseWrappedResources(x11wrappedBackBuffers[frameIndex].GetAddressOf(), 1);

   x11DeviceContext->Flush();
   ThrowIfFailed(commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]));

   // Present the frame.
   ThrowIfFailed(swapChain->Present(1, 0));

}

void Graphics::OnRender(float dt)
{
   ThrowIfFailed(commandAllocators[frameIndex]->Reset());

   ThrowIfFailed(commandList->Reset(commandAllocators[frameIndex].Get(), nullptr));

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

   // Set necessary state.
   commandList->RSSetViewports(1, &viewport);
   commandList->RSSetScissorRects(1, &scissorRect);
}

void Graphics::OnRenderX11()
{
   x11DeviceContext->OMSetDepthStencilState(x11DepthStencilState.Get(), 0u);

   // bind render target
   x11DeviceContext->OMSetRenderTargets(1u, x11Target[frameIndex].GetAddressOf(), nullptr);

   x11DeviceContext->RSSetViewports(1u, &x11ViewPort);
}

void Graphics::OnRender2DWrite()
{
   // Render text directly to the back buffer.
   x11d2dDeviceContext->SetTarget(x11d2dRenderTargets[frameIndex].Get());
}

void Graphics::CleanUp()
{
   // wait for the gpu to finish all frames
   for (int i = 0; i < bufferCount; ++i)
   {
      frameIndex = i;
      WaitForPreviousFrame();
   }

   // Wait for windows to finish
   for (int i = 0; i < bufferCount; ++i)
   {
      frameIndex = i;
      ThrowIfFailed(commandQueue->Signal(fence[frameIndex].Get(), fenceValue[frameIndex]));
      WaitForPreviousFrame();
   }
}
