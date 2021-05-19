#include "dwritedraw.h"

dwritedraw::dwritedraw(Graphics &gfx)
   :
   m_gfx(gfx),
   m_width(0),
   m_height(0)
{
   m_width = static_cast<float>(gfx.getWidth());
   m_height = static_cast<float>(gfx.getHeight());

   // DWrite
   ThrowIfFailed(gfx.get2dContext()->CreateSolidColorBrush(
      D2D1::ColorF(D2D1::ColorF::White),
      &m_x11d2dtextBrush));
   ThrowIfFailed(gfx.get2dWriteFactory()->CreateTextFormat(
      L"Arial",
      NULL,
      DWRITE_FONT_WEIGHT_NORMAL,
      DWRITE_FONT_STYLE_NORMAL,
      DWRITE_FONT_STRETCH_NORMAL,
      25,
      L"en-us",
      &m_x11d2dtextFormat
   ));

}

void dwritedraw::draw()
{
   //D2D1_SIZE_F rtSize = x11d2dRenderTargets[frameIndex]->GetSize();
   D2D1_RECT_F textRect = D2D1::RectF(20, 20, m_width, m_height);
   static const WCHAR textx12[] = L"DirectX12 ESC Mouse Move";

   //// Render text directly to the back buffer.
   //gfx.Get2dContext()->SetTarget(x11d2dRenderTargets[frameIndex].Get());
   m_gfx.get2dContext()->BeginDraw();

   m_gfx.get2dContext()->DrawRectangle(D2D1::RectF(5.0f, 5.0f, m_width - 5.0f, m_height - 5.0f), m_x11d2dtextBrush.Get());
   //gfx.Get2dContext()->DrawLine(D2D1::Point2F(width / 2.0f, 5.0f), D2D1::Point2F(width / 2.0f, height - 5.0f), x11d2dtextBrush.Get());

   m_gfx.get2dContext()->SetTransform(D2D1::Matrix3x2F::Identity());
   m_gfx.get2dContext()->DrawText(
      textx12,
      _countof(textx12) - 1,
      m_x11d2dtextFormat.Get(),
      &textRect,
      m_x11d2dtextBrush.Get()
   );


   ThrowIfFailed(m_gfx.get2dContext()->EndDraw());
}
