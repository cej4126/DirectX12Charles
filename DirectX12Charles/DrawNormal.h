#pragma once
#include "stdafx.h"
#include "Graphics.h"
#include "NormalObject.h"
#include "DrawFunction.h"
#include "Transform.h"


class DrawNormal : public DrawFunction
{
public:
	DrawNormal(Graphics &gfx, int &index, Shape::shapeType type, float size,
		const std::string &texturefilename,
		const std::string &normalfilename,
		ID3D12Resource *lightView, int &MaterialIndex);
	void SetPos(DirectX::XMFLOAT3 pos) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	int getMaterialIndex() const noexcept { return m_materialIndex; }
	void getMaterialData(Graphics::MaterialType &myMaterial) const noexcept
	{
		myMaterial.materialColor = { 0.0f, 0.0f, 0.0f, 1.0f };	
		myMaterial.specularInensity = m_material.specularInensity;
		myMaterial.specularPower = m_material.specularPower;
		myMaterial.hasNormal = 0;
		myMaterial.hasGloss = 0;
	}
	void SyncMaterial() noexcept;
	void SpawnControlWindow(const std::string &name) noexcept;

private:
	Graphics &m_gfx;
   
protected:
	// x     y      z
	DirectX::XMFLOAT3 m_pos = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT3 m_rot = { 0.0f, 0.0f, 0.0f };
	float m_size = 1.0f;
	int m_materialIndex = -1;
	Graphics::MaterialType m_material;
};

