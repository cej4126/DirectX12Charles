#include "DrawFunction.h"

void DrawFunction::Draw(Graphics &gfx, int &index) const noexcept
{
   for (auto &b : binds)
   {
      b->Bind(gfx, index);
   }
   ++index;
}

void DrawFunction::AddBind(std::shared_ptr<Bind::Bindable> bind) noexcept
{
   binds.push_back(std::move(bind));
}
