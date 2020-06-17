#pragma once
#include "stdafx.h"

class Graphics
{
public:
	Graphics(HWND hWnd, int width, int height);
	Graphics(const Graphics &) = delete;
	Graphics &operator=(const Graphics &) = delete;
	~Graphics() = default;

	void OnRender();

private:
	void ItemDepent();
	void WaitForPreviousFrame();

protected:
	static const UINT bufferCount = 3;
	struct Vertex
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT4 color;
	};
	int width;
	int height;
	float aspectRatio;


	Microsoft::WRL::ComPtr<IDXGIFactory4> m_DxgiFactory4;
	Microsoft::WRL::ComPtr<IDXGIAdapter3> adapter;
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> swapChainBuffers;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilBuffer;
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> descHeapDepthStencil;
	std::vector<Microsoft::WRL::ComPtr<ID3D12CommandAllocator>> commandAllocators;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Fence>> fences;
	std::vector<UINT64> fenceValues;
	HANDLE fenceEventHandle;

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderBlob;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderBlob;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexBuffer;

	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;

	UINT frameIndex;
	HANDLE fenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> m_fence;
	UINT64 fenceValue;

	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRect;
	int rtvDescriptorSize;
};

