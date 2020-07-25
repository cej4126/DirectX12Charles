#include "BindableX11.h"

ID3D11DeviceContext *BindableX11::GetContext(Graphics &gfx) noexcept
{
   return gfx.x11DeviceContext.Get();
}

ID3D11Device *BindableX11::GetDevice(Graphics &gfx) noexcept
{
   return gfx.x11Device.Get();
}
