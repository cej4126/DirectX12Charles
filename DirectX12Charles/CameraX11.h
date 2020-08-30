#pragma once
#include "Graphics.h"

class CameraX11
{
public:
   XMMATRIX GetMatricX11() const noexcept;
   void CreateControlWindowX11() noexcept;
   void ResetX11() noexcept;

private:
   float range;

   float rollPosition = 0.0f;
   float pitchPosition = 0.0f;
   float yawPosition = 0.0f;
   float rollRotation = 0.0f;
   float pitchRotation = 0.0f;
   float yawRotation = 0.0f;
};
