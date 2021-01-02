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

   void Draw(Graphics &gfx, int &index) const noexcept;
   virtual void Update(float dt) noexcept {};
   virtual XMMATRIX GetTransformXM() const noexcept = 0;
   virtual int getMaterialIndex() const noexcept
   {
      return 0;
   };
   virtual void getMaterialData(Graphics::MaterialType &myMaterial) const noexcept {};


protected:
   void AddBind(std::shared_ptr <Bindable> bind) noexcept;

private:
   std::vector<std::shared_ptr<Bindable>> binds;
};

//template<class Y>
//class DrawBase : public DrawFunction
//{
//public:
//   bool isStaticSet() { return !staticBindsX12.empty(); }
//   static void addStaticBind(std::unique_ptr < Bindable > bind, UINT indexBuffer)
//   {
//      staticBindsX12.push_back(std::move(bind));
//   }
//
//private:
//   const std::vector<std::unique_ptr < Bindable>> &GetStaticBinds() const noexcept override
//   {
//      return staticBindsX12;
//   }
//   static std::vector<std::unique_ptr<Bindable>> staticBindsX12;
//};

template<class R>
std::vector<std::shared_ptr<Bindable>> DrawBase;