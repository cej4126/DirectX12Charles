#pragma once
#include "stdafx.h"
#include "DrawX11.h"
#include "ObjectX11.h"
#include "TransformX11.h"
#include "Geometry.h"

class ShapeColorBlendedX11 : public DrawBaseX11 <ShapeColorBlendedX11>
{
public:
   ShapeColorBlendedX11(Graphics &gfx, float range);
   void Update(float dt) noexcept override;
   XMMATRIX GetTransformXM() const noexcept override;
   UINT getIndexCount() const noexcept override;

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

   UINT indexCount;
};
