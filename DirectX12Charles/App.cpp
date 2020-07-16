#include "App.h"
#include "BoxX11.h"

App::App()
   :
   wnd(1000, 800)
{
   std::mt19937 gen(2018);

   std::mt19937 rng(std::random_device{}());
   std::uniform_real_distribution<float> rangedist(6.0f, 20.0f);

   lastTime = std::chrono::steady_clock::now();

   for (auto i = 0; i < 1; i++)
   {
      float range = rangedist(rng);
      boxes.push_back(std::make_unique<BoxX11>(wnd.Gfx(), range));
   }
}

int App::Go()
{
   while (wnd.running)
   {
      if (const auto code = Window::ProcessMessages())
      {
         return *code;
      }
      if (wnd.running)
      {
         DoFrame();
      }
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
