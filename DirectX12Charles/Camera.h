#pragma once
#include "Graphics.h"

class Camera
{
public:
   Camera() noexcept;
   XMMATRIX GetMatrix() const noexcept;
   void CreateControlWindow() noexcept;
   void Reset() noexcept;

   void Rotate(float dx, float dy) noexcept;
   void Translate(DirectX::XMFLOAT3 translation) noexcept;

private:
   float wrap_angle(float theta);
   static constexpr float PI = 3.14159265f;
   DirectX::XMFLOAT3 pos;
   float pitch;
   float yaw;
   static constexpr float travelSpeed = 12.0f;
   static constexpr float rotationSpeed = 0.001f;
};
