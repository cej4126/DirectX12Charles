#include "Camera.h"
#include "imgui/imgui.h"

XMMATRIX Camera::GetMatrix() const noexcept
{
   const auto pos = XMVector3Transform(
      XMVectorSet(0.0f, 0.0f, -range, 0.0f),
      XMMatrixRotationRollPitchYaw(-pitchPosition, yawPosition, 0.0f));

   return XMMatrixLookAtLH(
      pos,
      XMVectorZero(),
      XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)) *
      XMMatrixRotationRollPitchYaw(-pitchRotation, yawRotation, -rollRotation);
}

void Camera::CreateControlWindow() noexcept
{
   if (ImGui::Begin("Camera Control"))
   {
      ImGui::Text("Object");
      ImGui::SliderFloat("Range Object", &range, 0.0f, 80.0f, "%.1f");
      ImGui::SliderAngle("Yaw Object", &yawPosition, -180.0f, 180.0f);
      ImGui::SliderAngle("Pitch Object", &pitchPosition, -89.0f, 89.0f);
      ImGui::Text("Orientation");
      ImGui::SliderAngle("Roll Rotation", &rollRotation, -180.0f, 180.0f);
      ImGui::SliderAngle("Pitch Rotation", &pitchRotation, -180.0f, 180.0f);
      ImGui::SliderAngle("Yaw Rotation", &yawRotation, -180.0f, 180.0f);
      if (ImGui::Button("Reset"))
      {
         Reset();
      }
   }
   ImGui::End();
}

void Camera::Reset() noexcept
{
   range = 20.0f;
   pitchPosition = 0.0f;
   yawPosition = 0.0f;
   rollRotation = 0.0f;
   pitchRotation = 0.0f;
   yawRotation = 0.0f;
}
