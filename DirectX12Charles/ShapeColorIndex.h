#pragma once
#include "stdafx.h"
#include "Graphics.h"
#include "ObjectX12.h"
#include "DrawX12.h"
#include "TransformX12.h"
#include "Shape.h"

class ShapeColorIndex : public DrawBaseX12 <ShapeColorIndex>
{
public:
   ShapeColorIndex(Graphics &gfx, Shape::shapeType type, float range);
   void Update(float dt) noexcept override;
   XMMATRIX GetTransformXM() const noexcept override;

private:
   BindableX12 *object = nullptr;
   float range = 0.0f;

   float boxRoll = 0.0f;
   float boxPitch = 0.0f;
   float boxYaw = 0.0f;
   float spaceRoll = 0.0f;
   float spacePitch = 0.0f;
   float spaceYaw = 0.0f;

   float boxRollRate = 0.0f;
   float boxPitchRate = 0.0f;
   float boxYawRate = 0.0f;
   float spaceRollRate = 0.0f;
   float spacePitchRate = 0.0f;
   float spaceYawRate = 0.0f;
};

