#pragma once
#include "stdafx.h"
#include "Graphics.h"
#include "Object.h"
#include "DrawFunction.h"
#include "Transform.h"

class ShapeLighted : public DrawBase <ShapeLighted>
{
public:
   ShapeLighted(Graphics &gfx, float range, ID3D12Resource *mylightView);
   void Update(float dt) noexcept override;
   XMMATRIX GetTransformXM() const noexcept override;

private:
   struct Vertex
   {
      XMFLOAT3 pos;
      //struct
      //{
      //   float u;
      //   float v;
      //} tex;
      XMFLOAT3 n;
   };
   void Scale(std::vector< Vertex > &vertices, float x, float y, float z);
   void SetNormals(std::vector <unsigned short>& indices, std::vector< Vertex >& vertices) noexcept;

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
};
