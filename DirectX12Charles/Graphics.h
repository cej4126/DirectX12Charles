#pragma once
#include "stdafx.h"

class Graphics
{
public:
	Graphics(HWND hWnd, int width, int height);
	Graphics(const Graphics &) = delete;
	Graphics &operator=(const Graphics &) = delete;
	~Graphics() = default;

protected:
	static const UINT bufferCount = 3;

	Microsoft::WRL::ComPtr<IDXGIFactory4> m_DxgiFactory4;
	Microsoft::WRL::ComPtr<IDXGIAdapter3> adapter;
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;
};

