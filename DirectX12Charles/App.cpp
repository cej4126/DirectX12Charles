#include "App.h"
#include "BoxX11.h"

App::App()
   :
   wnd(1000, 800)
{
   std::mt19937 gen(2018);

   std::mt19937 rng(std::random_device{}());
   std::uniform_real_distribution<float> rangedist(1.0f, 5.0f);
//   std::uniform_real_distribution<float> rangedist(6.0f, 20.0f);

   lastTime = std::chrono::steady_clock::now();

   for (auto i = 0; i < 1; i++)
   {
      float range = rangedist(rng);
      boxes.push_back(std::make_unique<BoxX11>(wnd.Gfx(), range));
   }
   wnd.Gfx().SetProjectionX11(DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
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
   auto dt = TimePeek();


   wnd.Gfx().OnRenderBegin(dt);

   wnd.Gfx().OnRender(dt);

   for (auto &b : boxes)
   {
      b->Update(dt);
      b->Draw(wnd.Gfx());
   }

   wnd.Gfx().OnRenderEnd(dt);

}
