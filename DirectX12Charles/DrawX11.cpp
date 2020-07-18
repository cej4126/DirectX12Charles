#include "DrawX11.h"

void DrawX11::Draw(Graphics &gfx) const noexcept
{
   //for (auto &b : binds)
   //{
   //   b->Bind(gfx);
   //}
   //gfx.DrawIndexed(b->)
}

void DrawX11::AddBind(std::unique_ptr<BindableX11> bind) noexcept
{
}
