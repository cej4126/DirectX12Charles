#pragma once
#include "stdafx.h"
#include "Graphics.h"
#include "Object.h"
#include "DrawFunction.h"
#include "Transform.h"

class ShapePointLight : public DrawBase <ShapePointLight>
{
public:
   ShapePointLight(Graphics &gfx, float size);
   void SetPosition(XMFLOAT3 pos) noexcept;
   void CreateLightControl() noexcept;
   void Reset() noexcept;
   void Update(float dt) noexcept override;
   XMMATRIX GetTransformXM() const noexcept override;
   ID3D12Resource *getLightView() { return lightView; }

private:
   Bindable *object = nullptr;
   float size = 1.0f;
   XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
   ID3D12Resource *lightView = nullptr;
};
