#pragma once
#include "stdafx.h"
#include "DrawBaseX11.h"
#include "ObjectX11.h"

class BoxX11 : public DrawBaseX11 <BoxX11>
{
public:
   BoxX11(Graphics &gfx, float range);
   void Update(float dt) noexcept override;

private:
   struct Vertex
   {
      struct
      {
         float x;
         float y;
         float z;
      } pos;
   };

   BindableX11 *object;
   float range;

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

