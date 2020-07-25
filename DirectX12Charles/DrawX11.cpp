#include "DrawX11.h"

void DrawX11::Draw(Graphics &gfx) const noexcept
{
   for (auto &b : binds)
   {
      b->Bind(gfx);
   }

   for (auto &b : GetStaticBinds())
   {
      b->Bind(gfx);
   }
   gfx.DrawIndexed(indexCountDrawX11);
}

void DrawX11::AddBind(std::unique_ptr<BindableX11> bind) noexcept
{
   binds.push_back(std::move(bind));
}
