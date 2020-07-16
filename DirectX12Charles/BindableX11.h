#pragma once
#include "Graphics.h"

class BindableX11
{
public:
   virtual void Bind(Graphics &gfx) noexcept = 0;
   virtual ~BindableX11() = default;

protected:
   static ID3D11DeviceContext *GetContext(Graphics &gfx) noexcept;
   static ID3D11Device *GetDevice(Graphics &gfx) noexcept;
};
