#pragma once
#include "Graphics.h"

class Camera
{
public:
   XMMATRIX GetMatrix() const noexcept;
   void CreateControlWindow() noexcept;
   void Reset() noexcept;

private:
   float range = 20.0f;

   float pitchPosition = 0.0f;
   float yawPosition = 0.0f;
   float rollRotation = 0.0f;
   float pitchRotation = 0.0f;
   float yawRotation = 0.0f;
};
