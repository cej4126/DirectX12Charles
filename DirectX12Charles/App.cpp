#include "App.h"

App::App()
   :
   wnd(1000, 800)
{
   lastTime = std::chrono::steady_clock::now();
}

int App::Go()
{
   while (true)
   {
      if (const auto code = Window::ProcessMessages())
      {
         return *code;
      }
      DoFrame();
   }
   return 0;
}

App::~App()
{
}

float App::TimePeek()
{
   return std::chrono::duration<float>(std::chrono::steady_clock::now() - lastTime).count();
}

float App::TimeMark()
{
   const auto oldTime = lastTime;
   lastTime = std::chrono::steady_clock::now();
   const std::chrono::duration<float> frameTime = lastTime - oldTime;
   return frameTime.count();
}

void App::DoFrame()
{
   wnd.Gfx().OnRender(TimePeek());
}
