#pragma once
#include "stdafx.h"

using namespace DirectX;

class Graphics
{
public:
	Graphics(HWND hWnd, int width, int height);
	Graphics(const Graphics &) = delete;
	Graphics &operator=(const Graphics &) = delete;
	~Graphics() = default;

	void OnRender(float angle);

private:
	void LoadDriveX12();
	void LoadBaseX12();
	void CreateFence();
	void LoadDepentX12();
	void LoadVertexBuffer();
	void LoadIndexBuffer();
	void OnRenderX12(float angle);
	void WaitForPreviousFrame();

	// DirectX 11
	void LoadDriveX11Only();
	void LoadBaseX11();
	void LoadDepentX11();
	void OnRender2DWrite();
	void OnRenderX11(float angle);

	// DWrite
	void LoadBase2D();

protected:
	static const UINT bufferCount = 3;
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		struct
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		} color;
	};
	int width;
	int height;
	float aspectRatio;
	HWND hWnd;

	struct ConstantBuffer
	{
		XMMATRIX transform;
	};
	ConstantBuffer constantBuffer;


	Microsoft::WRL::ComPtr <IDXGIFactory4> m_DxgiFactory4;
	Microsoft::WRL::ComPtr <IDXGIAdapter3> adapter;
	Microsoft::WRL::ComPtr <ID3D12Device> device;
	Microsoft::WRL::ComPtr <IDXGISwapChain1> swapChain1;
	Microsoft::WRL::ComPtr <ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr <IDXGISwapChain3> swapChain;
	std::vector<Microsoft::WRL::ComPtr <ID3D12Resource>> swapChainBuffers;
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> m_rtvHeap;
	Microsoft::WRL::ComPtr <ID3D12Resource> depthStencilBuffer;
	Microsoft::WRL::ComPtr <ID3D12DescriptorHeap> descHeapDepthStencil;
	std::vector<Microsoft::WRL::ComPtr <ID3D12CommandAllocator>> commandAllocators;
	Microsoft::WRL::ComPtr <ID3D12GraphicsCommandList> commandList;
	std::vector<Microsoft::WRL::ComPtr <ID3D12Fence>> fences;
	std::vector<UINT64> fenceValues;
	HANDLE fenceEventHandle;

	Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr <ID3DBlob> vertexShaderBlob;
	Microsoft::WRL::ComPtr <ID3DBlob> pixelShaderBlob;
	Microsoft::WRL::ComPtr <ID3D12PipelineState> pipelineState;

	Microsoft::WRL::ComPtr <ID3D12Resource> vertexBuffer;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	Microsoft::WRL::ComPtr <ID3D12Resource> indexBuffer;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	UINT indicesCount;

	struct ConstantBufferPerObject
	{
		XMMATRIX wvpMat;
	};
	int ConstantBufferPerObjectAlignedSize = (sizeof(ConstantBufferPerObject) + 255) & ~255;
	ConstantBufferPerObject cbPerObject;
	Microsoft::WRL::ComPtr <ID3D12Resource> constantBufferUploadHeaps;
	UINT8 *cbvGPUAddress;

	UINT frameIndex;
	HANDLE fenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	UINT64 fenceValue;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;
	int rtvDescriptorSize;

   // DirectX 11
	struct VertexX11
	{
		struct
		{
			float x;
			float y;
		} pos;
		struct
		{
			unsigned char r;
			unsigned char g;
			unsigned char b;
			unsigned char a;
		} color;
	};

	struct ConstantBufferX11
	{
		XMMATRIX transform;
	};
	ConstantBufferX11 cbX11;

	Microsoft::WRL::ComPtr<ID3D11Device> x11Device;
	Microsoft::WRL::ComPtr<ID3D11DeviceContext> x11DeviceContext;
	Microsoft::WRL::ComPtr<ID3D11On12Device> x11On12Device;
	Microsoft::WRL::ComPtr<ID3D11Buffer> x11VertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> x11IndexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> x11ConstantBuffer;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> x11PixelShader;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> x11VertexShader;
	Microsoft::WRL::ComPtr<ID3D11InputLayout> x11InputLayout;
	Microsoft::WRL::ComPtr<ID3D11Resource> x11BackBuffer[bufferCount];
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> x11Target[bufferCount];
	Microsoft::WRL::ComPtr<ID3D12Resource> x11renderTargets[bufferCount];
	Microsoft::WRL::ComPtr<ID3D11Resource> x11wrappedBackBuffers[bufferCount];
	Microsoft::WRL::ComPtr<ID3D11Resource> pBackBuffer[bufferCount];
	Microsoft::WRL::ComPtr<ID3D11DepthStencilState> x11DepthStencilState;
	D3D11_VIEWPORT x11ViewPort;
	UINT verticeX11Count;
	UINT indiceX11Count;

	Microsoft::WRL::ComPtr<IDXGISwapChain> pSwap;

	// DWrite
	Microsoft::WRL::ComPtr<IDXGIDevice> dxgiDevice;
	Microsoft::WRL::ComPtr<ID2D1Bitmap1> x11d2dRenderTargets[bufferCount];
	Microsoft::WRL::ComPtr<ID2D1DeviceContext2> x11d2dDeviceContext;
	Microsoft::WRL::ComPtr<IDWriteTextFormat> x11d2dtextFormat;
	Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> x11d2dtextBrush;
	Microsoft::WRL::ComPtr<ID2D1Factory3> m_d2dFactory;
	Microsoft::WRL::ComPtr<ID2D1Device2> m_d2dDevice;
	Microsoft::WRL::ComPtr<IDWriteFactory> m_dWriteFactory;
};

