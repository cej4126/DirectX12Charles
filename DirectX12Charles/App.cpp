#include "App.h"
#include "Shape.h"
#include "Surface.h"
#include "GDIPlusManager.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

GDIPlusManager gdipm;

//#define NO_LIGHT

App::App()
   :
   wnd(1280, 720)
{
   std::mt19937 gen(2018);

   std::mt19937 rng(std::random_device{}());
   std::uniform_real_distribution<float> rangedist(3.0f, 15.0f);

   lastTime = std::chrono::steady_clock::now();

   dwriteitem = std::make_unique<dwritedraw>(wnd.Gfx());

#ifndef NO_LIGHT
   light = std::make_unique<ShapePointLight>(wnd.Gfx(), 0.5f);
#endif
   int MaxBoxX12Count = 6;
   int MaterialCount = 0;
   int objectCount = 0;
   Shape::shapeType type;
   for (auto i = 0; i < MaxBoxX12Count; i++)
   {
      float range = rangedist(rng);

      type = static_cast<Shape::shapeType>(Shape::TextureCube + (i % 4));
      drawItems.push_back(std::make_unique<ShapeLighted>(wnd.Gfx(), type, range, light->getLightView(), MaterialCount));
      ++objectCount;
      ++MaterialCount;

      //drawItems.push_back(std::make_unique<ShapeAssimp>(wnd.Gfx(), Shape::TextureSuzanne, range, light->getLightView(), MaterialCount));
      ++objectCount;
      //++MaterialCount;


      //drawItems.push_back(std::make_unique<ShapeTextureCube>(wnd.Gfx(), range));
      ++objectCount;
      //drawItems.push_back(std::make_unique<ShapePicture>(wnd.Gfx(), Shape::TextureCube, range, "..\\..\\DirectX12Charles\\Images\\280893.jpg"));
      ++objectCount;
      //drawItems.push_back(std::make_unique<ShapePicture>(wnd.Gfx(), Shape::TextureCube, range, "..\\..\\DirectX12Charles\\Images\\cobalt-city.jpg"));
      ++objectCount;
      //drawItems.push_back(std::make_unique<ShapePicture>(wnd.Gfx(), Shape::TextureCube, range, "..\\..\\DirectX12Charles\\Images\\picture3.jpg"));
      ++objectCount;

      //type = static_cast<Shape::shapeType>(i % static_cast<int>(Shape::Sphere + 1));
      //drawItems.push_back(std::make_unique<ShapeColorIndex>(wnd.Gfx(), type, range));
      ++objectCount;
      //drawItems.push_back(std::make_unique<ShapeColorBlended>(wnd.Gfx(), type, range));
      ++objectCount;
   }

   //nano = std::make_unique<Model>(wnd.Gfx(), "..\\..\\DirectX12Charles\\Models\\nanosuit.obj",
   //   light->getLightView(), MaterialCount, objectCount);
   //nano = std::make_unique<Model>(wnd.Gfx(), "..\\..\\DirectX12Charles\\Models\\boxy.gltf.glb",
   //   light->getLightView(), MaterialCount, objectCount);
   nano = std::make_unique<Model>(wnd.Gfx(), "..\\..\\DirectX12Charles\\Models\\nano.gltf",
      light->getLightView(), MaterialCount, objectCount);
   ++MaterialCount;


   wnd.Gfx().CreateMatrixConstant(objectCount);
   wnd.Gfx().CreateMaterialConstant(MaterialCount);

   for (auto &b : drawItems)
   {
      if (auto po = dynamic_cast<ShapeLighted *>(b.get()))
      {
         lightedObjects.push_back(po);
      }

      int materialIndex = b->getMaterialIndex();
      if (materialIndex != -1)
      {
         Graphics::MaterialType material;
         b->getMaterialData(material);
         wnd.Gfx().CopyMaterialConstant(materialIndex, material);
      }
   }
   nano->FirstCommand();

   wnd.Gfx().RunCommandList();
   wnd.Gfx().SetProjection(XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 40.0f));
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

//void App::ShowRawInputWindow()
//{
//   while (const auto d = wnd.input.ReadRawDelta())
//   {
//      x += d->x;
//      y += d->y;
//   }
//   if (ImGui::Begin("Raw Input"))
//   {
//      ImGui::Text("Tally: (%d,%d)", x, y);
//      ImGui::Text("Cursor: %s", wnd.CursorEnabled() ? "Enable" : "Disable");
//   }
//   ImGui::End();
//}

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
   wnd.Gfx().SetCamera(cam.GetMatrix());

   wnd.Gfx().OnRenderBegin();

   wnd.Gfx().OnRender();

   int index = 0;

#ifndef NO_LIGHT
   auto &lightObject = light;
   lightObject->Draw(wnd.Gfx(), index);
#endif

   nano->Draw(wnd.Gfx(), index);

   for (auto &b : drawItems)
   {
      b->Update(dt);
      b->Draw(wnd.Gfx(), index);
   }

   dwriteitem->Draw();

   while (auto e = wnd.input.ReadKey())
   {
      if (e->isPress())
      {
         switch (e->GetCode())
         {
            //case VK_INSERT:
            case VK_ESCAPE:
               if (wnd.CursorEnabled())
               {
                  wnd.DisableCursor();
                  wnd.input.EnableRaw();
               }
               else
               {
                  wnd.EnableCursor();
                  wnd.input.DisableRaw();
               }
               break;
         }
      }
   }

   if (!wnd.CursorEnabled())
   {
      if (wnd.input.KeyIsPressed('W'))
      {
         cam.Translate({ 0.0f, 0.0f, dt });
      }
      else if (wnd.input.KeyIsPressed('A'))
      {
         cam.Translate({ -dt, 0.0f, 0.0f });
      }
      else if (wnd.input.KeyIsPressed('S'))
      {
         cam.Translate({ 0.0f, 0.0f, -dt });
      }
      else if (wnd.input.KeyIsPressed('D'))
      {
         cam.Translate({ dt, 0.0f, 0.0f });
      }
      else if (wnd.input.KeyIsPressed('R'))
      {
         cam.Translate({ 0.0f, dt, 0.0f });
      }
      else if (wnd.input.KeyIsPressed('F'))
      {
         cam.Translate({ 0.0f, -dt, 0.0f });
      }

      while (const auto delta = wnd.input.ReadRawDelta())
      {
         if (!wnd.CursorEnabled())
         {
            cam.Rotate((float)delta->x, (float)delta->y);
         }
      }


   }

   SpawnSimulation();

   cam.CreateControlWindow();
#ifndef NO_LIGHT
   lightObject->CreateLightControl();
#endif

   SpawnObjectControl();
   nano->ShowWindow();
   //ShowRawInputWindow();

   ImGui::Render();
   ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

   wnd.Gfx().DrawCommandList();

   wnd.Gfx().OnRenderEnd();

}

void App::SpawnSimulation()
{
   // Start the ImGui frame
   ImGui_ImplDX11_NewFrame();
   ImGui_ImplWin32_NewFrame();
   ImGui::NewFrame();
   ImGui::Begin("Simulation Speed");

   ImGui::SliderFloat("Speed Factor", &speedFactor, 0.0f, 4.0f);
   ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
      1000.0f / ImGui::GetIO().Framerate,
      ImGui::GetIO().Framerate);
   ImGui::End();
}

void App::SpawnObjectControl()
{
   static bool firstTime = true;

   if (!lightedObjects.empty())
   {
      if (ImGui::Begin(("Object " + std::to_string(objectIndex)).c_str()))
      {
         bool changed = ImGui::InputInt("Id", &objectIndex) || firstTime;
         if (changed)
         {
            firstTime = false;
            if (objectIndex < 1)
            {
               objectIndex = 1;
            }
            else if (objectIndex > lightedObjects.size())
            {
               objectIndex = (int)lightedObjects.size();
            }

            for (std::vector<class ShapeLighted *>::iterator it = lightedObjects.begin(); it != lightedObjects.end(); ++it)
            {
               ShapeLighted *c = *it;
               int i = c->getMaterialIndex() + 1;
               if (objectIndex == i)
               {
                  currentObject = c;
               }
            }
         }
         if (currentObject != nullptr)
         {
            currentObject->SpawnControlWindow();
         }
      }
      ImGui::End();
   }
}
