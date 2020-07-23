#pragma once
#include "stdafx.h"
#include "BindableX11.h"
#include "Graphics.h"
#include "DrawX11.h"

class TransformX11 : public BindableX11
{
public:
   TransformX11(Graphics &gfx, const DrawX11 &parent);
//	~TransformX11() = default;

	// Vertex Constant Buffer
	void AddTransformConstantBuffer()
	{

		D3D11_BUFFER_DESC cbd;
		cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		cbd.Usage = D3D11_USAGE_DYNAMIC;
		cbd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		cbd.MiscFlags = 0u;
		cbd.ByteWidth = sizeof(Vertexcbuf);
		cbd.StructureByteStride = 0u;

		D3D11_SUBRESOURCE_DATA csd = {};
		csd.pSysMem = &Vertexcbuf;
		ThrowIfFailed(GetDevice(gfx)->CreateBuffer(&cbd, &csd, &pTransformConstantBuffer));
	}
	void Bind(Graphics &gfx) noexcept override;

public:
	Graphics &gfx;
	XMMATRIX Vertexcbuf;

private:
	// Vetrix Constant Buffer
	Microsoft::WRL::ComPtr<ID3D11Buffer> pTransformConstantBuffer;
   //static std::unique_ptr<XMMATRIX> pVertexcbuf;
	const DrawX11 &parentTransform;
};


//std::unique_ptr<XMMATRIX> pVertexcbuf;