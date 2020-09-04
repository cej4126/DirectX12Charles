#include "DrawFunction.h"

void DrawFunction::Draw(Graphics &gfx, int index) const noexcept
{
   for (auto &b : GetStaticBinds())
   {
      b->Bind(gfx, 0);
   }

   for (auto &b : GetStaticBinds())
   {
      b->Bind(gfx, 2);
   }

   for (auto &b : binds)
   {
      b->Bind(gfx, index);
   }

}

void DrawFunction::AddBind(std::unique_ptr<Bindable> bind) noexcept
{
   binds.push_back(std::move(bind));
}
