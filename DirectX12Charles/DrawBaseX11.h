#pragma once
#include "stdafx.h"
#include "DrawX11.h"
#include "BindableX11.h"

template<class T>
class DrawBaseX11 : public DrawX11   // DrawableBase
{
public:
   bool isStaticSet() { return !Binds.empty(); }
   static void addStaticBind(std::unique_ptr < BindableX11 > bind)
   {
      Binds.push_back(std::move(bind));
   }

private:
   static std::vector<std::unique_ptr<BindableX11>> Binds;
};

template<class T>
std::vector<std::unique_ptr<BindableX11>> DrawBaseX11 <T>::Binds;