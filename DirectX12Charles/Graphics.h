#pragma once
#include "stdafx.h"
#include "Shape.h"
#include "ShapeAssimp.h"

using namespace Microsoft::WRL;
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
      XMFLOAT3 viewLightPos;
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
      XMFLOAT4 materialColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
      float specularInensity = 0.6f;
      float specularPower = 30.0f;
      int hasNormal = false;
      int hasGloss = false;
      int hasSpecular = false;
      XMFLOAT4 specularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
      float specularWeight = 1.0f;
   };

   TransformMatrix matrixBuffer;
   int ConstantBufferPerObjectAlignedSize = (sizeof(matrixBuffer) + 255) & ~255;


   void runCommandList();
   void onRenderBegin();
   void onRender();
   void drawCommandList();
   void onRenderEnd();
   void cleanUp();


   void createMatrixConstant(UINT count);
   void createMaterialConstant(UINT count);
   void setMatrixConstant(UINT index, TransformMatrix matrix, int rootVS, int rootPS) noexcept;

   void copyMaterialConstant(UINT index, MaterialType &matrix) noexcept;
   void setMaterialConstant(UINT index) noexcept;


   UINT64 updateSubresource(
      _In_ ID3D12Resource *pDestinationResource,
      _In_ ID3D12Resource *pIntermediate,
      _In_reads_(NumSubresources) D3D12_SUBRESOURCE_DATA *pSrcData);

   ID3D12Device *getDevice() noexcept { return m_device.Get(); }
   ID3D12GraphicsCommandList *getCommandList() noexcept { return m_commandList.Get(); }

   void setProjection(FXMMATRIX proj) noexcept { m_projection = proj; }
   XMMATRIX getProjection() const noexcept { return m_projection; }

   void setCamera(FXMMATRIX camera) noexcept { m_camera = camera; };
   XMMATRIX getCamera() const noexcept { return m_camera; };

   ID3D11DeviceContext *setContextX11() noexcept { return m_x11DeviceContext.Get(); }
   ID3D11Device *getDeviceX11() noexcept { return m_x11Device.Get(); }

   ID2D1DeviceContext2 *get2dContext() noexcept { return m_x11d2dDeviceContext.Get(); }
   IDWriteFactory *get2dWriteFactory() noexcept { return m_dWriteFactory.Get(); }
   int getWidth() { return m_width; }
   int getHeight() { return m_height; }

   Shape m_shape;
   ShapeAssimp m_shapeAssimp;

private:
   void waitForPreviousFrame();
   void loadDevice();
   void loadBase();
   void createFence();

   void loadDepent();
   void onRender(float dt);

   // DirectX 11
   void loadBaseX11();
   void onRenderX11();

   // DWrite
   void loadBase2D();
   void onRender2DWrite();

private:
   ComPtr <ID3D12Resource> m_matrixBufferUploadHeaps;
   UINT8 *m_matrixBufferGPUAddress;

   ComPtr <ID3D12Resource> m_materialBufferUploadHeaps;
   UINT8 *m_materialBufferGPUAddress;

protected:
   static const UINT Buffer_Count = 3;

   int m_width;
   int m_height;
   HWND m_hWnd;

   XMMATRIX m_projection;
   XMMATRIX m_camera;

   ComPtr <IDXGIFactory4> m_dxgiFactory4;
   ComPtr <IDXGIAdapter3> m_adapter;
   ComPtr <ID3D12Device> m_device;
   ComPtr <IDXGISwapChain1> m_swapChain1;
   ComPtr <ID3D12CommandQueue> m_commandQueue;
   ComPtr <IDXGISwapChain3> m_swapChain;
   ComPtr <ID3D12Resource> m_swapChainBuffers[Buffer_Count];
   ComPtr <ID3D12DescriptorHeap> m_rtvHeap;
   ComPtr < ID3D12DescriptorHeap> m_dsDescriptorHeap;
   ComPtr <ID3D12Resource> m_depthStencilBuffer;
   std::vector<ComPtr <ID3D12CommandAllocator>> m_commandAllocators;
   ComPtr <ID3D12GraphicsCommandList> m_commandList;
   HANDLE m_fenceEvent;
   ComPtr<ID3D12Fence> m_fences[Buffer_Count];
   UINT64 m_fenceValues[Buffer_Count];
   HANDLE m_fenceEventHandle;

   UINT m_frameIndex;

   D3D12_VIEWPORT m_viewport;
   D3D12_RECT m_scissorRect;
   int m_rtvDescriptorSize;
   float m_angle = 0.0f;

   // DirectX 11

   ComPtr<ID3D11Device> m_x11Device;
   ComPtr<ID3D11DeviceContext> m_x11DeviceContext;
   ComPtr<ID3D11On12Device> m_x11On12Device;
   ComPtr<ID3D11Resource> m_x11BackBuffers[Buffer_Count];
   ComPtr<ID3D11RenderTargetView> m_x11Targets[Buffer_Count];
   ComPtr<ID3D12Resource> m_x11renderTargets[Buffer_Count];
   ComPtr<ID3D11Resource> m_x11wrappedBackBuffers[Buffer_Count];
   ComPtr<ID3D11Resource> m_pBackBuffers[Buffer_Count];
   ComPtr<ID3D11DepthStencilState> m_x11DepthStencilState;
   D3D11_VIEWPORT m_x11ViewPort;

   ComPtr<IDXGISwapChain> m_pSwap;

   // DWrite
   ComPtr<IDXGIDevice> m_dxgiDevice;
   ComPtr<ID2D1Bitmap1> m_x11d2dRenderTargets[Buffer_Count];
   ComPtr<ID2D1DeviceContext2> m_x11d2dDeviceContext;

   ComPtr<ID2D1Factory3> m_d2dFactory;
   ComPtr<ID2D1Device2> m_d2dDevice;
   ComPtr<IDWriteFactory> m_dWriteFactory;
};

