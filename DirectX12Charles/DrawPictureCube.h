#pragma once
#include "stdafx.h"
#include "Graphics.h"
#include "Object.h"
#include "Texture.h"
#include "Transform.h"
#include "DrawFunction.h"

class DrawPictureCube : public DrawFunction
{
public:
   DrawPictureCube(Graphics &gfx, int &index, Shape::shapeType type, float range, const std::string &filename);
   void Update(float dt) noexcept override;
   XMMATRIX GetTransformXM() const noexcept override;
   int getMaterialIndex() const noexcept { return -1; }
   void getMaterialData(Graphics::MaterialType &myMaterial) const noexcept {};

private:
   Bind::Bindable *object = nullptr;
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
