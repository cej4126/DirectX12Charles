#pragma once
#include "stdafx.h"
#include "Window.h"
#include "BoxX11.h"
#include "OneBoxX11.h"

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
   std::vector<std::unique_ptr<class BoxX11>> boxes;
   std::unique_ptr<class OneBoxX11 > box;
};

