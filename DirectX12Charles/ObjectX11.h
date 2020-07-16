#pragma once
#include "stdafx.h"
#include "BindableX11.h"
#include "Graphics.h"

class ObjectX11 : public BindableX11
{
public:
   ObjectX11(Graphics &gfx);

	template<class V>
	void AddVertexBuffer(const std::vector<V> &vertices)
	{
		D3D11_BUFFER_DESC bd = {};
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.Usage = D3D11_USAGE_DEFAULT;
		bd.CPUAccessFlags = 0u;
		bd.MiscFlags = 0u;
		bd.ByteWidth = UINT(sizeof(V) * vertices.size());
		bd.StructureByteStride = sizeof(V);
		D3D11_SUBRESOURCE_DATA sd = {};
		sd.pSysMem = vertices.data();
		ThrowIfFailed(GetDevice(gfx)->CreateBuffer(&bd, &sd, &pVertexBuffer));
	}

	void AddIndexBuffer(const std::vector<unsigned short> &indices);

	void AddShaders(const std::wstring &vertexPath, const std::wstring &pixelPath);

	// Pixel Constant Buffer
	template<typename C>
	void AddPixelConstantBuffer(const C &consts)
	{
		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof(consts);
		cbd.StructureByteStride = 0u;

		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &consts;
		ThrowIfFailed(GetDevice(gfx)->CreateBuffer(&cbd, &csd, &pConstantBuffer));
	}

	// Layout
	void AddInputLayout(const std::vector<D3D11_INPUT_ELEMENT_DESC> &layout);

	// Topology
	void AddTopology(D3D11_PRIMITIVE_TOPOLOGY type);
	void Bind(Graphics &gfx) noexcept override;

public:
	Graphics &gfx;

private:

   // VertexBuffer 
   Microsoft::WRL::ComPtr<ID3D11Buffer> pVertexBuffer;
   UINT vertexStride;
   // IndexBuffer 
   UINT indexCount;
   Microsoft::WRL::ComPtr<ID3D11Buffer> pIndexBuffer;

	Microsoft::WRL::ComPtr<ID3DBlob> pVertexBytecodeBlob;
	Microsoft::WRL::ComPtr<ID3D11VertexShader> pVertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> pPixelBytecodeBlob;
	Microsoft::WRL::ComPtr<ID3D11PixelShader> pPixelShader;

	// Constant Buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> pConstantBuffer;

	// Layout
	Microsoft::WRL::ComPtr<ID3D11InputLayout> pInputLayout;
	// Topology
	D3D11_PRIMITIVE_TOPOLOGY topologyType;
};

