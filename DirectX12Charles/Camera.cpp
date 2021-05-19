#include <algorithm>
#include "Camera.h"
#include "imgui/imgui.h"

Camera::Camera() noexcept
{
   reset();
}

XMMATRIX Camera::getMatrix() const noexcept
{
   const XMVECTOR forwardBaseVector = XMVectorSet(0.0f, 0.0f, 1.0f, 0.0f);
   // apply the camera rotations to a base vector
   const auto lookVector = XMVector3Transform(forwardBaseVector,
      XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f)
   );
   // generate camera transform (applied to all objects to arrange them relative
   // to camera position/orientation in world) from cam position and direction
   // camera "top" always faces towards +Y (cannot do a barrel roll)
   const auto camPosition = XMLoadFloat3(&m_position);
   const auto camTarget = camPosition + lookVector;
   return XMMatrixLookAtLH(camPosition, camTarget, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
}

void Camera::createControlWindow() noexcept
{
   if (ImGui::Begin("Camera Control"))
   {
      ImGui::Text("Position");
      ImGui::SliderFloat("X", &m_position.x, -80.0f, 80.0f, "%.1f");
      ImGui::SliderFloat("Y", &m_position.y, -80.0f, 80.0f, "%.1f");
      ImGui::SliderFloat("Z", &m_position.z, -80.0f, 80.0f, "%.1f");
      ImGui::Text("Orientation");
      ImGui::SliderAngle("Pitch", &m_pitch, -89.950f, 89.95f);
      ImGui::SliderAngle("Yaw", &m_yaw, -180.0f, 180.0f);
      if (ImGui::Button("Reset"))
      {
         reset();
      }
   }
   ImGui::End();
}

void Camera::reset() noexcept
{
   //pos = { -13.5f, 6.0f, 3.5f };
   m_position = { -7.5f, 0.0f, 0.0f };
   m_pitch = 0.0f;
   m_yaw = PI/2.0f;
   //pos = { -10.0f, 0.0f, -20.0f };
   //pitch = 0.0f;
   //yaw = 0.45f;
}


float Camera::wrapAngle(float theta)
{
   constexpr float twoPi = 2.0f * PI;
   const float mod = fmod(theta, twoPi);
   if (mod > PI)
   {
      return mod - twoPi;
   }
   else if (mod < PI)
   {
      return mod + twoPi;
   }
   return mod;
}

void Camera::rotate(float dx, float dy) noexcept
{
   m_yaw = wrapAngle(m_yaw + dx * Rotation_Speed);
   m_pitch = std::clamp(m_pitch + dy * Rotation_Speed, -PI / 2.0f, PI / 2.0f);
}

void Camera::translate(DirectX::XMFLOAT3 translation) noexcept
{
   XMStoreFloat3(&translation, XMVector3Transform(
      XMLoadFloat3(&translation),
      XMMatrixRotationRollPitchYaw(m_pitch, m_yaw, 0.0f) *
      XMMatrixScaling(Travel_Speed, Travel_Speed, Travel_Speed)
   ));
   m_position = {
      m_position.x + translation.x,
      m_position.y + translation.y,
      m_position.z + translation.z
   };
}
