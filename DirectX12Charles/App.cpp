#include "App.h"

App::App()
   :
   wnd(1000, 800)
{
   std::mt19937 gen(2018);

   std::mt19937 rng(std::random_device{}());
   std::uniform_real_distribution<float> rangedist(3.0f, 10.0f);

   lastTime = std::chrono::steady_clock::now();

   dwriteitem = std::make_unique<dwritedraw>(wnd.Gfx());

   oneCubeColorIndexX11 = std::make_unique<OneBoxX11>(wnd.Gfx());
   oneCubeColorIndex = std::make_unique<OneBoxX12>(wnd.Gfx());

   int MaxBoxX12Count = 4;
   for (auto i = 0; i < MaxBoxX12Count; i++)
   {
      float range = rangedist(rng);

      if ((i % 2) == 0)
      {
         drawItems.push_back(std::make_unique<ShapeColorIndex>(wnd.Gfx(), range));
      }
      else
      {
         drawItems.push_back(std::make_unique<ShapeColorBlended>(wnd.Gfx(), range));
      }
   }
   wnd.Gfx().CreateMatrixConstantX12(MaxBoxX12Count);

   int MaxBoxX11Count = 4;
   for (auto i = 0; i < MaxBoxX11Count; i++)
   {
      float range = rangedist(rng);
      if ((i % 2) == 0)
      {
         drawItemsX11.push_back(std::make_unique<ShapeColorIndexX11>(wnd.Gfx(), range));
      }
      else
      {
         drawItemsX11.push_back(std::make_unique<ShapeColorBlendedX11>(wnd.Gfx(), range));
      }
   }

   wnd.Gfx().RunCommandList();

   wnd.Gfx().SetProjectionX11(DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
   wnd.Gfx().SetProjectionX12(DirectX::XMMatrixPerspectiveLH(1.0f, 3.0f / 4.0f, 0.5f, 40.0f));
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

float App::TimeMark()
{
   const auto oldTime = lastTime;
   lastTime = std::chrono::steady_clock::now();
   const std::chrono::duration<float> frameTime = lastTime - oldTime;
   return frameTime.count();
}

float App::TimePeek()
{
   return std::chrono::duration<float>(std::chrono::steady_clock::now() - lastTime).count();
}

void App::DoFrame()
{
   auto dt = TimeMark();

   wnd.Gfx().OnRenderBegin();

   wnd.Gfx().OnRender();

   oneCubeColorIndex->Update(dt);
   oneCubeColorIndex->Draw();
   oneCubeColorIndex->LoadConstant();

   int index = 0;
   for (auto &b : drawItems)
   {
      b->Update(dt);
      b->Draw(wnd.Gfx(), index);
      ++index;
   }

   dwriteitem->Draw();
   oneCubeColorIndexX11->Update(dt);
   oneCubeColorIndexX11->Draw();

   wnd.Gfx().DrawCommandList();

   for (auto &b : drawItemsX11)
   {
      b->Update(dt);
      b->Draw(wnd.Gfx());
   }

   wnd.Gfx().OnRenderEnd();
}
