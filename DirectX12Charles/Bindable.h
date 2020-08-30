#pragma once
#include "Graphics.h"

class Bindable
{
public:
   virtual void Bind(Graphics &gfx, int drawStep) noexcept = 0;
   virtual ~Bindable() = default;
};

