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
      XMMATRIX modelViewProj;
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

   ID3D12Device *getDevice() noexcept { return m_device.Get(); }
   ID3D12GraphicsCommandList *getCommandList() noexcept { return m_commandList.Get(); }

   void runCommandList();
   void onRenderBegin();
   void onRender();
   void drawCommandList();
   void onRenderEnd();
   void cleanUp();

   void setCamera(FXMMATRIX cameraTransfrom) noexcept { m_cameraTransform = cameraTransfrom; }
   XMMATRIX getCamera() const noexcept { return m_cameraTransform; }
   void setProjection(FXMMATRIX projection) noexcept { m_projection = projection; }
   XMMATRIX getProjection() const noexcept { return m_projection; }
   void createMatrixConstant(UINT index);
   void setMatrixConstant(UINT index, TransformMatrix matrix, int rootVS, int rootPS) noexcept;

   void createMaterialConstant(UINT count);
   void copyMaterialConstant(UINT index, MaterialType &matrix) noexcept;
   void setMaterialConstant(UINT index) noexcept;

   UINT64 updateSubresource(
      _In_ ID3D12Resource *pDestinationResource,
      _In_ ID3D12Resource *pIntermediate,
      _In_reads_(NumSubresources) D3D12_SUBRESOURCE_DATA *pSrcData);


   //ID3D11DeviceContext *setContextX11() noexcept { return m_x11DeviceContext.Get(); }
   //ID3D11Device *getDeviceX11() noexcept { return m_x11Device.Get(); }

   // d2Write
   ID2D1DeviceContext2 *get2dContext() noexcept { return m_x11d2dDeviceContext.Get(); }
   IDWriteFactory *get2dWriteFactory() noexcept { return m_dWriteFactory.Get(); }
   int getWidth() { return m_width; }
   int getHeight() { return m_height; }

   Shape m_shape;
   ShapeAssimp m_shapeAssimp;

protected:
   static const UINT Buffer_Count = 3;
   HWND m_hWnd;
   int m_width;
   int m_height;
   UINT m_frameIndex;
   int m_rtvDescriptorSize;
   UINT64 m_fenceValues[Buffer_Count];
   HANDLE m_fenceHandle;
   XMMATRIX m_projection;
   XMMATRIX m_cameraTransform;

   D3D12_VIEWPORT m_viewPort;
   D3D12_RECT m_scissorRect;

   ComPtr <IDXGIFactory4> m_dxgiFactory4;
   //ComPtr <IDXGIAdapter3> m_adapter;
   ComPtr <ID3D12Device> m_device;
   ComPtr <ID3D12CommandQueue>m_commandQueue;
   ComPtr <IDXGISwapChain3> m_swapChain;
   ComPtr <IDXGISwapChain1> m_swapChain1;
   ComPtr <ID3D12Resource> m_swapChainBuffers[Buffer_Count];
   ComPtr <ID3D12DescriptorHeap> m_rtvHeap;
   std::vector<ComPtr <ID3D12CommandAllocator>> m_commandAllocators;
   ComPtr <ID3D12GraphicsCommandList> m_commandList;
   ComPtr <ID3D12Fence> m_fences[Buffer_Count];
   ComPtr < ID3D12DescriptorHeap> m_dsDescriptorHeap;
   ComPtr <ID3D12Resource> m_depthStencilBuffer;

   // Direct X11
   ComPtr <ID3D11Device> m_x11Device;
   ComPtr <ID3D11DeviceContext> m_x11DeviceContext;
   ComPtr <ID3D11On12Device> m_x11On12Device;
   ComPtr <ID3D11Resource> m_x11WrappedBackBuffers[Buffer_Count];
   ComPtr <ID3D11RenderTargetView> m_x11Targets[Buffer_Count];
   D3D11_VIEWPORT m_x11ViewPort;
   ComPtr <ID3D11DepthStencilState> m_x11DepthStencilState;

private:
   ComPtr <ID3D12Resource> m_matrixBufferUploadHeaps;
   UINT8 *m_matrixBufferGPUAddress;

   void loadDevice();
   void loadBase();
   void loadDepent();
   void createFence();
   void waitForPreviousFrame();

   // DirectX11
   void loadX11OnX12Base();
   void onRenderX11();

   // 2DWrite
   void loadBase2D();
   ComPtr<IDXGIDevice> m_dxgiDevice;
   ComPtr<ID2D1Bitmap1> m_x11d2dRenderTargets[Buffer_Count];
   ComPtr<ID2D1DeviceContext2> m_x11d2dDeviceContext;

   ComPtr<ID2D1Factory3> m_d2dFactory;
   ComPtr<ID2D1Device2> m_d2dDevice;
   ComPtr<IDWriteFactory> m_dWriteFactory;

private:

   ComPtr <ID3D12Resource> m_materialBufferUploadHeaps;
   UINT8 *m_materialBufferGPUAddress;
   float m_angle = 0.0f;
};

