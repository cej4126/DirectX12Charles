#include "CameraX11.h"
#include "imgui.h"

XMMATRIX CameraX11::GetMatricX11() const noexcept
{
	const auto pos = XMVector3Transform(
		XMVectorSet(0.0f, 0.0f, -range, 0.0f),
		XMMatrixRotationRollPitchYaw(rollPosition, pitchPosition, yawPosition));

	return XMMatrixLookAtLH(
		pos,
		XMVectorZero(),
		XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)) *
		XMMatrixRotationRollPitchYaw(rollRotation, pitchRotation, yawRotation);
}

void CameraX11::CreateControlWindowX11() noexcept
{
	if (ImGui::Begin("Camera Control"))
	{
		ImGui::Text("Position");
		ImGui::SliderFloat("Range", &range, 0.0f, 80.0f, "%.1f");
		ImGui::SliderFloat("Roll", &rollPosition, 0.0f, 80.0f, "%.1f");
		ImGui::SliderAngle("Pitch", &pitchPosition, -180.0f, 180.0f);
		ImGui::SliderAngle("Yaw", &yawPosition, -89.0f, 89.0f);
		ImGui::Text("Orientation");
		ImGui::SliderAngle("Roll", &rollRotation, -180.0f, 180.0f);
		ImGui::SliderAngle("Pitch", &pitchRotation, -180.0f, 180.0f);
		ImGui::SliderAngle("Yaw", &yawRotation, -180.0f, 180.0f);
		if (ImGui::Button("Reset"))
		{
			ResetX11();
		}
	}
	ImGui::End();
}

void CameraX11::ResetX11() noexcept
{
	range = 20.0f;
	rollPosition = 0.0f;
	pitchPosition = 0.0f;
	yawPosition = 0.0f;
	rollRotation = 0.0f;
	pitchRotation = 0.0f;
	yawRotation = 0.0f;
}
