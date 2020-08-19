#pragma once
#include "stdafx.h"
#include "Graphics.h"
#include "BindableX11.h"

class DrawX11
{
   template<class T>
   friend class DrawBaseX11;
public:
   DrawX11() = default;
   DrawX11(const DrawX11 &) = delete;
   virtual ~DrawX11() = default;

   void Draw(Graphics &gfx) const noexcept;
   virtual void Update(float dt) noexcept = 0;
   virtual XMMATRIX GetTransformXM() const noexcept = 0;
   virtual UINT getIndexCount() const noexcept = 0;

protected:
   void AddBind(std::unique_ptr < BindableX11 > bind) noexcept;

private:
   virtual const std::vector<std::unique_ptr<BindableX11>> &GetStaticBinds() const noexcept = 0;
   std::vector<std::unique_ptr< BindableX11 >> binds;
};

template<class T>
class DrawBaseX11 : public DrawX11   // DrawableBase
{
public:
   bool isStaticSet() { return !staticBinds.empty(); }
   static void addStaticBind(std::unique_ptr < BindableX11 > bind, UINT indexBuffer)
   {
      staticBinds.push_back(std::move(bind));
   }

private:
   const std::vector<std::unique_ptr < BindableX11>> &GetStaticBinds() const noexcept override
   {
      return staticBinds;
   }
   static std::vector<std::unique_ptr<BindableX11>> staticBinds;
};

template<class T>
std::vector<std::unique_ptr<BindableX11>> DrawBaseX11 <T>::staticBinds;