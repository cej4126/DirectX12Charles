#pragma once
#include "stdafx.h"
#include "Graphics.h"
#include "Object.h"
#include "DrawFunction.h"
#include "Transform.h"

class ShapeLighted : public DrawBase <ShapeLighted>
{
public:
   ShapeLighted(Graphics &gfx, Shape::shapeType type, float range, ID3D12Resource *mylightView, int MaterialIndex);
   void Update(float dt) noexcept override;
   XMMATRIX GetTransformXM() const noexcept override;
   Graphics::MaterialType getMaterial() { return material; }
   int getMaterialIndex() const noexcept { return MaterialIndex; }
   void getMaterialData(Graphics::MaterialType &myMaterial) const noexcept;

private:
   struct Vertex
   {
      XMFLOAT3 pos;
      XMFLOAT3 normal;
   };
   void Scale(std::vector< Vertex > &vertices, float x, float y, float z);

   Bindable *object = nullptr;
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
   int MaterialIndex = -1;
   Graphics::MaterialType material;
};
