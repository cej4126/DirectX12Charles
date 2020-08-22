#include "DrawX12.h"

void DrawX12::Draw(Graphics &gfx, int index) const noexcept
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

void DrawX12::AddBind(std::unique_ptr<BindableX12> bind) noexcept
{
   binds.push_back(std::move(bind));
}
