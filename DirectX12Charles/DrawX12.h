#pragma once
#include "stdafx.h"
#include "Graphics.h"
#include "BindableX12.h"

class DrawX12
{
   template<class Z>
   friend class DrawBaseX12;
public:
   DrawX12() = default;
   DrawX12(const DrawX12 &) = delete;
   virtual ~DrawX12() = default;

public:
   void Draw(Graphics &gfx, int index) const noexcept;
   virtual void Update(float dt) noexcept = 0;
   virtual XMMATRIX GetTransformXM() const noexcept = 0;

protected:
   void AddBind(std::unique_ptr <BindableX12> bind) noexcept;

private:
   virtual const std::vector<std::unique_ptr<BindableX12>> &GetStaticBinds() const noexcept = 0;
   std::vector<std::unique_ptr<BindableX12>> binds;
};

template<class Y>
class DrawBaseX12 : public DrawX12
{
public:
   bool isStaticSet() { return !staticBindsX12.empty(); }
   static void addStaticBind(std::unique_ptr < BindableX12 > bind, UINT indexBuffer)
   {
      staticBindsX12.push_back(std::move(bind));
   }

private:
   const std::vector<std::unique_ptr < BindableX12>> &GetStaticBinds() const noexcept override
   {
      return staticBindsX12;
   }
   static std::vector<std::unique_ptr<BindableX12>> staticBindsX12;
};

template<class R>
std::vector<std::unique_ptr<BindableX12>> DrawBaseX12 <R>::staticBindsX12;