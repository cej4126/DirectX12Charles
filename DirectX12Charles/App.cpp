#include "App.h"
#include "Shape.h"
#include "Surface.h"
#include "GDIPlusManager.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

GDIPlusManager gdipm;

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

   int MaxBoxX12Count = 6;
   for (auto i = 0; i < MaxBoxX12Count; i++)
   {
      Shape::shapeType type = static_cast<Shape::shapeType>(i % static_cast<int>(Shape::Sphere + 1));
      float range = rangedist(rng);

      if ((i % 4) == 0)
      {
         drawItems.push_back(std::make_unique<ShapeTextureCube>(wnd.Gfx(), range));
         drawItems.push_back(std::make_unique<ShapePicture>(wnd.Gfx(), range));
      }
      drawItems.push_back(std::make_unique<ShapeColorIndex>(wnd.Gfx(), type, range));
      drawItems.push_back(std::make_unique<ShapeColorBlended>(wnd.Gfx(), type, range));
   }
   wnd.Gfx().CreateMatrixConstantX12(MaxBoxX12Count);

   int MaxBoxX11Count = 6;
   Shape::shapeType type = Shape::Cube;
   for (auto i = 0; i < MaxBoxX11Count; i++)
   {
      Shape::shapeType type = static_cast<Shape::shapeType>(i % static_cast<int>(Shape::Sphere + 1));
      float range = rangedist(rng);

      if ((i % 4) == 0)
      {
         drawItemsX11.push_back(std::make_unique<ShapeTextureCubeX11>(wnd.Gfx(), range));
         drawItemsX11.push_back(std::make_unique<ShapePictureX11>(wnd.Gfx(), range));
      }
      drawItemsX11.push_back(std::make_unique<ShapeColorIndexX11>(wnd.Gfx(), type, range));
      drawItemsX11.push_back(std::make_unique<ShapeColorBlendedX11>(wnd.Gfx(), type, range));
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
   auto dt = TimeMark() * speedFactor;

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

   // Start the Dear ImGui frame
   ImGui_ImplDX11_NewFrame();
   ImGui_ImplWin32_NewFrame();
   ImGui::NewFrame();
   ImGui::Begin("Simulation Speed");

   ImGui::SliderFloat("Speed Factor", &speedFactor, 0.0f, 4.0f);
   ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
      1000.0f / ImGui::GetIO().Framerate,
      ImGui::GetIO().Framerate);

   ImGui::End();

   ImGui::Render();
   ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


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
