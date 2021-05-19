#pragma once
#include "stdafx.h"
#include "Graphics.h"

class dwritedraw
{
public:
   dwritedraw(Graphics &gfx);
   void draw();

private:
   Graphics &m_gfx;
   float m_width;
   float m_height;

   Microsoft::WRL::ComPtr<IDWriteTextFormat> m_x11d2dtextFormat;
   Microsoft::WRL::ComPtr<ID2D1SolidColorBrush> m_x11d2dtextBrush;

};

