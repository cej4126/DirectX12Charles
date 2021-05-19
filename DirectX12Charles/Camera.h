#pragma once
#include "Graphics.h"

class Camera
{
public:
   Camera() noexcept;
   XMMATRIX getMatrix() const noexcept;
   void createControlWindow() noexcept;
   void reset() noexcept;

   void rotate(float dx, float dy) noexcept;
   void translate(DirectX::XMFLOAT3 translation) noexcept;

private:
   float wrapAngle(float theta);
   static constexpr float PI = 3.14159265f;
   DirectX::XMFLOAT3 m_position;
   float m_pitch;
   float m_yaw;
   static constexpr float Travel_Speed = 12.0f;
   static constexpr float Rotation_Speed = 0.001f;
};
