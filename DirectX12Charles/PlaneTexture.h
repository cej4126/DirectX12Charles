#pragma once
#include "stdafx.h"
#include "Graphics.h"
#include "Object.h"
#include "DrawFunction.h"
#include "Transform.h"


class PlaneTexture : public DrawFunction
{
public:
	PlaneTexture(Graphics &gfx, int &index, float size);
	void SetPos(DirectX::XMFLOAT3 pos) noexcept;
	void SetRotation(float roll, float pitch, float yaw) noexcept;
	DirectX::XMMATRIX GetTransformXM() const noexcept override;
	int getMaterialIndex() const noexcept { return -1; }
	//void getMaterialData(Graphics::MaterialType &myMaterial) const noexcept {};
private:
	//Bind::Bindable *object = nullptr;

	//DirectX::XMFLOAT3 pos = { 1.0f,1.0f,1.0f };
	//float roll = 0.0f;
	//float pitch = 0.0f;
	//float yaw = 0.0f;

};

