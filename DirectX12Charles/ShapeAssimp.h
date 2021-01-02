#pragma once
#include "stdafx.h"
#include "Graphics.h"
#include "Object.h"
#include "DrawFunction.h"
#include "Transform.h"

class ShapeAssimp : public DrawFunction
{
public:
   ShapeAssimp(Graphics &gfx, Shape::shapeType type, float range, ID3D12Resource *mylightView, int MaterialIndex);
   void Update(float dt) noexcept override;
   XMMATRIX GetTransformXM() const noexcept override;
   Graphics::MaterialType getMaterial() { return material; }
   void getMaterialData(Graphics::MaterialType &myMaterial) const noexcept;
   void SpawnControlWindow() noexcept;
   int getMaterialIndex() const noexcept { return MaterialIndex; }

private:
   Graphics &gfx;
   struct Vertex
   {
      XMFLOAT3 pos;
      XMFLOAT3 normal;
   };
   //void Scale(std::vector< Vertex > &vertices, float x, float y, float z);
   void SyncMaterial() noexcept;

   Bindable *object = nullptr;

protected:
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

private:
   int MaterialIndex = -1;
   Graphics::MaterialType material;
};
