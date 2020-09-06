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
   //void SetPosition(XMFLOAT3 pos) noexcept;
   void CreateLightControl() noexcept;
   void ResetLightData() noexcept;
   void Update(float dt) noexcept override;
   XMMATRIX GetTransformXM() const noexcept override;

private:
   Graphics &gfx;
   Bindable *object = nullptr;
   float size = 1.0f;
   struct Graphics::lightDataType lightData =
   {
      XMFLOAT3(0.0f, 0.0f, 0.0f), 0.0f,
      XMFLOAT3(0.7f, 0.7f, 0.7f), 0.0f,
      XMFLOAT3(0.05f, 0.05f, 0.05f), 0.0f,
      XMFLOAT3(1.0f, 0.0f, 0.0f), 0.0f,
      1.0f,
      1.0f,
      0.045f,
      0.0075f
   };
   ID3D12Resource *lightView = nullptr;
};
