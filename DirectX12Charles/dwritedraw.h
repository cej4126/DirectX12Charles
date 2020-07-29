#pragma once
#include "stdafx.h"
#include "Graphics.h"

class dwritedraw
{
public:
   dwritedraw(Graphics &gfx);
   void Draw();

private:
   Graphics &gfx;
   float width;
   float height;

   Microsoft::WRL::ComPtr<IDWriteTextFormat> x11d2dtextFormat;
   Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> x11d2dtextBrush;

};

