#pragma once
#include "stdafx.h"
#include "Shape.h"
#include "ShapeAssimp.h"

using namespace DirectX;

class Graphics
{
   friend class BindableX11;

public:
   Graphics(HWND hWnd, int width, int height);
   Graphics(const Graphics &) = delete;
   Graphics &operator=(const Graphics &) = delete;
   ~Graphics();

   struct TransformMatrix
   {
      XMMATRIX modelViewProg;
      XMMATRIX model;
   };

   struct lightDataType
   {
      XMFLOAT3 position;
      float pad1;
      XMFLOAT3 ambient;
      float pad3;
      XMFLOAT3 diffuseColor;
      float pad4;
      float diffuseIntensity;
      float attConst;
      float attLin;
      float attQuad;
   } lightData;

   struct MaterialType
   {
      XMFLOAT3 materialColor;
      float specularInensity = 0.6f;
      float specularPower = 30.0f;
      float pad[2];
   };

   TransformMatrix matrixBuffer;
   int ConstantBufferPerObjectAlignedSize = (sizeof(matrixBuffer) + 255) & ~255;


   void RunCommandList();
   void OnRenderBegin();
   void OnRender();
   void DrawCommandList();
   void OnRenderEnd();
   void CleanUp();


   void CreateMatrixConstant(UINT count);
   void CreateMaterialConstant(UINT count);
   void SetMatrixConstant(UINT index, TransformMatrix matrix, int rootVS, int rootPS) noexcept;

   void CopyMaterialConstant(UINT index, MaterialType &matrix) noexcept;
   void SetMaterialConstant(UINT index) noexcept;


   UINT64 UpdateSubresource(
      _In_ ID3D12Resource *pDestinationResource,
      _In_ ID3D12Resource *pIntermediate,
      _In_reads_(NumSubresources) D3D12_SUBRESOURCE_DATA *pSrcData);

   ID3D12Device *GetDevice() noexcept { return device.Get(); }
   ID3D12GraphicsCommandList *GetCommandList() noexcept { return commandList.Get(); }

   void SetProjection(FXMMATRIX proj) noexcept { projection = proj; }
   XMMATRIX GetProjection() const noexcept { return projection; }

   void SetCamera(FXMMATRIX cam) noexcept { camera = cam; };
   XMMATRIX GetCamera() const noexcept { return camera; };

   ID3D11DeviceContext *GetContextX11() noexcept { return x11DeviceContext.Get(); }
   ID3D11Device *GetDeviceX11() noexcept { return x11Device.Get(); }

   ID2D1DeviceContext2 *Get2dContext() noexcept { return x11d2dDeviceContext.Get(); }
   IDWriteFactory *Get2dWriteFactory() noexcept { return m_dWriteFactory.Get(); }
   int GetWidth() { return width; }
   int GetHeight() { return height; }

   Shape shape;
   ShapeAssimp shapeAssimp;

private:
   void WaitForPreviousFrame();
   void LoadDrive();
   void LoadBase();
   void CreateFence();

   void LoadDepent();
   void OnRender(float dt);

   // DirectX 11
   void LoadBaseX11();
   void OnRenderX11();

   // DWrite
   void LoadBase2D();
   void OnRender2DWrite();

private:
   Microsoft::WRL::ComPtr <ID3D12Resource> MatrixBufferUploadHeaps;
   UINT8 *matrixBufferGPUAddress;

   Microsoft::WRL::ComPtr <ID3D12Resource> MaterialBufferUploadHeaps;
   UINT8 *MaterialBufferGPUAddress;

protected:
   static const UINT bufferCount = 3;

   int width;
   int height;
   float aspectRatio;
   HWND hWnd;

   XMMATRIX projection;
   XMMATRIX camera;

   Microsoft::WRL::ComPtr <IDXGIFactory4> m_DxgiFactory4;
   Microsoft::WRL::ComPtr <IDXGIAdapter3> adapter;
   Microsoft::WRL::ComPtr <ID3D12Device> device;
   Microsoft::WRL::ComPtr <IDXGISwapChain1> swapChain1;
   Microsoft::WRL::ComPtr <ID3D12CommandQueue> commandQueue;
   Microsoft::WRL::ComPtr <IDXGISwapChain3> swapChain;
   Microsoft::WRL::ComPtr <ID3D12Resource> swapChainBuffers[bufferCount];
   Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> m_rtvHeap;
   Microsoft::WRL::ComPtr < ID3D12DescriptorHeap> dsDescriptorHeap;
   Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilBuffer;
   std::vector<Microsoft::WRL::ComPtr <ID3D12CommandAllocator>> commandAllocators;
   Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList;
   HANDLE fenceEvent;
   Microsoft::WRL::ComPtr<ID3D12Fence> fence[bufferCount];
   UINT64 fenceValue[bufferCount];
   HANDLE fenceEventHandle;

   UINT frameIndex;

   D3D12_VIEWPORT viewport;
   D3D12_RECT scissorRect;
   int rtvDescriptorSize;
   float angle = 0.0f;

   // DirectX 11

   Microsoft::WRL::ComPtr<ID3D11Device> x11Device;
   Microsoft::WRL::ComPtr<ID3D11DeviceContext> x11DeviceContext;
   Microsoft::WRL::ComPtr<ID3D11On12Device> x11On12Device;
   Microsoft::WRL::ComPtr<ID3D11Resource> x11BackBuffer[bufferCount];
   Microsoft::WRL::ComPtr<ID3D11RenderTargetView> x11Target[bufferCount];
   Microsoft::WRL::ComPtr<ID3D12Resource> x11renderTargets[bufferCount];
   Microsoft::WRL::ComPtr<ID3D11Resource> x11wrappedBackBuffers[bufferCount];
   Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer[bufferCount];
   Microsoft::WRL::ComPtr<ID3D11DepthStencilState> x11DepthStencilState;
   D3D11_VIEWPORT x11ViewPort;

   Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;

   // DWrite
   Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
   Microsoft::WRL::ComPtr<ID2D1Bitmap1> x11d2dRenderTargets[bufferCount];
   Microsoft::WRL::ComPtr<ID2D1DeviceContext2> x11d2dDeviceContext;

   Microsoft::WRL::ComPtr<ID2D1Factory3> m_d2dFactory;
   Microsoft::WRL::ComPtr<ID2D1Device2> m_d2dDevice;
   Microsoft::WRL::ComPtr<IDWriteFactory> m_dWriteFactory;

};

