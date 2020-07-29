#pragma once
#include "stdafx.h"
#include "Graphics.h"

class OneBoxX12
{
   OneBoxX12(Graphics &gfx);
   void Update(float dt);
   void Draw();

private:
   Graphics &gfx;
};

