#pragma once
#include "stdafx.h"
#include "Graphics.h"
#include "Bindable.h"

class DrawFunction
{
   //template<class Z>
   //friend class DrawBase;
public:
   DrawFunction() = default;
   DrawFunction(const DrawFunction &) = delete;
   virtual ~DrawFunction() = default;

   void Draw(Graphics &gfx) const noexcept;
   virtual void Update(float dt) noexcept {};
   virtual XMMATRIX GetTransformXM() const noexcept = 0;
   virtual int getMaterialIndex() const noexcept
   {
      return 0;
   };
   virtual void getMaterialData(Graphics::MaterialType &myMaterial) const noexcept {};


protected:
   void AddBind(std::shared_ptr <Bind::Bindable> bind) noexcept;

private:
   std::vector<std::shared_ptr<Bind::Bindable>> binds;
};

template<class R>
std::vector<std::shared_ptr<Bind::Bindable>> DrawBase;