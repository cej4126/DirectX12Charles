#include "dwritedraw.h"

dwritedraw::dwritedraw(Graphics &gfx)
   :
   gfx(gfx),
   width(0),
   height(0)
{
   width = static_cast<float>(gfx.GetWidth());
   height = static_cast<float>(gfx.GetHeight());

   // DWrite
   ThrowIfFailed(gfx.Get2dContext()->CreateSolidColorBrush(
      D2D1::ColorF(D2D1::ColorF::White),
      &x11d2dtextBrush));
   ThrowIfFailed(gfx.Get2dWriteFactory()->CreateTextFormat(
      L"Arial",
      NULL,
      DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL,
      25,
      L"en-us",
      &x11d2dtextFormat
   ));

}

void dwritedraw::Draw()
{
   //D2D1_SIZE_F rtSize = x11d2dRenderTargets[frameIndex]->GetSize();
   D2D1_RECT_F textRect = D2D1::RectF(20, 20, width, height);
   static const WCHAR textx12[] = L"DirectX12";
   static const WCHAR textx11[] = L"DirectX11";

   //// Render text directly to the back buffer.
   //gfx.Get2dContext()->SetTarget(x11d2dRenderTargets[frameIndex].Get());
   gfx.Get2dContext()->BeginDraw();

   gfx.Get2dContext()->DrawRectangle(D2D1::RectF(5.0f, 5.0f, width - 5.0f, height - 5.0f), x11d2dtextBrush.Get());
   gfx.Get2dContext()->DrawLine(D2D1::Point2F(width / 2.0f, 5.0f), D2D1::Point2F(width / 2.0f, height - 5.0f), x11d2dtextBrush.Get());

   gfx.Get2dContext()->SetTransform(D2D1::Matrix3x2F::Identity());
   gfx.Get2dContext()->DrawText(
      textx12,
      _countof(textx12) - 1,
      x11d2dtextFormat.Get(),
      &textRect,
      x11d2dtextBrush.Get()
   );

   textRect.left = 520;
   gfx.Get2dContext()->DrawText(
      textx11,
      _countof(textx11) - 1,
      x11d2dtextFormat.Get(),
      &textRect,
      x11d2dtextBrush.Get()
   );

   ThrowIfFailed(gfx.Get2dContext()->EndDraw());
}
