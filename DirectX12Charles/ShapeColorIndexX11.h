#pragma once
#include "stdafx.h"
#include "DrawX11.h"
#include "ObjectX11.h"
#include "TransformX11.h"

class ShapeColorIndexX11 : public DrawBaseX11 <ShapeColorIndexX11>
{
public:
   ShapeColorIndexX11(Graphics &gfx, float range);
   void Update(float dt) noexcept override;
   XMMATRIX GetTransformXM() const noexcept override;

private:
   BindableX11 *object = nullptr;
   float range = 0.0f;

   float boxRoll;
   float boxPitch;
   float boxYaw;
   float spaceRoll;
   float spacePitch;
   float spaceYaw;

   float boxRollRate;
   float boxPitchRate;
   float boxYawRate;
   float spaceRollRate;
   float spacePitchRate;
   float spaceYawRate;
};

