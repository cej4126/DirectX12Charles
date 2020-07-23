#pragma once
#include "stdafx.h"
#include "DrawX11.h"
#include "BindableX11.h"

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