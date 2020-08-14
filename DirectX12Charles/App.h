#pragma once
#include "stdafx.h"
#include "Window.h"
#include "dwritedraw.h"
#include "BoxX11.h"
#include "BoxX12.h"
#include "OneBoxX11.h"
#include "OneBoxX12.h"

class App
{
public:
   App();
   int Go();
   ~App();
private:
   float TimeMark();
   float TimePeek();

   void DoFrame();
   Window wnd;

private:
   std::chrono::steady_clock::time_point lastTime;
   std::unique_ptr< class dwritedraw> dwriteitem;
   std::vector<std::unique_ptr<class BoxX11>> boxesX11;
   std::vector<std::unique_ptr<class BoxX12>> boxesX12;

   std::unique_ptr<class OneBoxX11 > boxX11;
   std::unique_ptr<class OneBoxX12 > boxX12;
};

