#pragma once
#include "Graphics.h"

class BindableX12
{
public:
   virtual void Bind(Graphics &gfx, int drawStep) noexcept = 0;
   virtual ~BindableX12() = default;
};

