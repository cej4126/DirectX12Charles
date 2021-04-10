#include "App.h"
#include "Shape.h"
#include "Surface.h"
#include "GDIPlusManager.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"

#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

#include "test.h"
testFunction test;

GDIPlusManager gdipm;

App::App()
   :
   wnd(1280, 720)
{
   std::mt19937 gen(2018);

   std::mt19937 rng(std::random_device{}());
   std::uniform_real_distribution<float> rangedist(3.0f, 15.0f);

   lastTime = std::chrono::steady_clock::now();

   dwriteitem = std::make_unique<dwritedraw>(wnd.Gfx());

   int MaxBoxX12Count = 1;
   int MaterialCount = 0;
   int objectCount = 0;

   light = std::make_unique<DrawPointLight>(wnd.Gfx(), objectCount, 0.5f);

   //plane = std::make_unique<DrawNormal>(wnd.Gfx(),
   //   objectCount,
   //   Shape::Plane, 6.0f,
   //   "..\\..\\DirectX12Charles\\Images\\brickwall.jpg",
   //   "..\\..\\DirectX12Charles\\Images\\brickwall_normal.jpg",
   //   light->getLightView(), MaterialCount);
   //plane->SetPos(XMFLOAT3(15.0f, 5.0f, 20.0f));

   //cube = std::make_unique<DrawNormal>(wnd.Gfx(),
   //   objectCount,
   //   Shape::TextureCube, 12.0f,
   //   "..\\..\\DirectX12Charles\\Images\\brickwall.jpg",
   //   "..\\..\\DirectX12Charles\\Images\\brickwall_normal.jpg",
   //   light->getLightView(), MaterialCount);
   //cube->SetPos(XMFLOAT3(-15.0f, 5.0f, 30.0f));

   //drawItems.push_back(std::make_unique<DrawNormal>(wnd.Gfx(), objectCount, Shape::Plane, 3.0f,
   //   "..\\..\\DirectX12Charles\\Images\\brickwall.jpg", "..\\..\\DirectX12Charles\\Images\\brickwall_normal.jpg", light->getLightView(), MaterialCount));

   //nano = std::make_unique<DrawModel>(wnd.Gfx(), objectCount, 1.0f,
   //   "..\\..\\DirectX12Charles\\Models\\nano_textured\\nanosuit.obj",
   //   light->getLightView(), MaterialCount);

   //wall = std::make_unique<DrawModel>(wnd.Gfx(), objectCount, 5.0f,
   //   "..\\..\\DirectX12Charles\\models\\brick_wall\\brick_wall.obj",
   //   light->getLightView(), MaterialCount);
   //wall->SetPosition(XMMatrixTranslation(-4.0f, 7.0f, 0.0f ));

   //gobber = std::make_unique<DrawModel>(wnd.Gfx(), objectCount, 5.0f,
   //   "..\\..\\DirectX12Charles\\models\\gobber\\GoblinX.obj",
   //   light->getLightView(), MaterialCount);
   //gobber->SetPosition(XMMatrixTranslation(-4.0f, 7.0f, 0.0f));

   //sponza = std::make_unique<DrawModel>(wnd.Gfx(), objectCount, 1.0f / 20.0f,
   //   "..\\..\\DirectX12Charles\\models\\sponza\\sponza.obj",
   //   light->getLightView(), MaterialCount);
   //sponza->SetPosition(XMMatrixTranslation(0.0f, 0.0f, 0.0f));

   for (auto i = 0; i < MaxBoxX12Count; i++)
   {
      float range = rangedist(rng);

      Shape::shapeType type = static_cast<Shape::shapeType>(Shape::TextureCube + (i % 4));
      drawItems.push_back(std::make_unique<DrawLighted>(wnd.Gfx(), objectCount, type, range, light->getLightView(), MaterialCount));
      drawItems.push_back(std::make_unique<DrawAssimp>(wnd.Gfx(), objectCount, Shape::TextureSuzanne, range, light->getLightView(), MaterialCount));
      drawItems.push_back(std::make_unique<DrawColorBlended>(wnd.Gfx(), objectCount, type, range));
      drawItems.push_back(std::make_unique<DrawColorIndex>(wnd.Gfx(), objectCount, type, range));
      drawItems.push_back(std::make_unique<DrawTextureCube>(wnd.Gfx(), objectCount, range));
      drawItems.push_back(std::make_unique<DrawPictureCube>(wnd.Gfx(), objectCount, Shape::TextureCube, range, "..\\..\\DirectX12Charles\\Images\\280893.jpg"));
      drawItems.push_back(std::make_unique<DrawPictureCube>(wnd.Gfx(), objectCount, Shape::TextureCube, range, "..\\..\\DirectX12Charles\\Images\\cobalt-city.jpg"));
      drawItems.push_back(std::make_unique<DrawPictureCube>(wnd.Gfx(), objectCount, Shape::TextureCube, range, "..\\..\\DirectX12Charles\\Images\\picture3.jpg"));
   }

   wnd.Gfx().CreateMatrixConstant(objectCount);
   wnd.Gfx().CreateMaterialConstant(MaterialCount);

   for (auto &b : drawItems)
   {
      if (auto po = dynamic_cast<DrawLighted *>(b.get()))
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

   if (nano != nullptr)
   {
      nano->FirstCommand();
   }
   if (wall != nullptr)
   {
      wall->FirstCommand();
   }
   if (gobber != nullptr)
   {
      gobber->FirstCommand();
   }
   if (sponza != nullptr)
   {
      sponza->FirstCommand();
   }

   wnd.Gfx().RunCommandList();
   wnd.Gfx().SetProjection(XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 80.0f));
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
   wnd.Gfx().SetCamera(cam.GetMatrix());

   wnd.Gfx().OnRenderBegin();

   wnd.Gfx().OnRender();

   int index = 0;

   auto &lightObject = light;
   lightObject->Draw(wnd.Gfx());

   if (plane != nullptr)
   {
      plane->Draw(wnd.Gfx());
   }
   if (cube != nullptr)
   {
      cube->Draw(wnd.Gfx());
   }
   if (nano != nullptr)
   {
      nano->Draw(wnd.Gfx());
   }
   if (wall != nullptr)
   {
      wall->Draw(wnd.Gfx());
   }
   if (gobber != nullptr)
   {
      gobber->Draw(wnd.Gfx());
   }
   if (sponza != nullptr)
   {
      sponza->Draw(wnd.Gfx());
   }

   for (auto &b : drawItems)
   {
      b->Update(dt);
      b->Draw(wnd.Gfx());
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
   lightObject->CreateLightControl();

   SpawnObjectControl();

   if (plane != nullptr)
   {
      plane->SpawnControlWindow("plane");
   }
   if (cube != nullptr)
   {
      cube->SpawnControlWindow("cube");
   }
   if (nano != nullptr)
   {
      nano->ShowWindow("Model Nano");
   }
   if (wall != nullptr)
   {
      wall->ShowWindow("Model Wall");
   }
   if (gobber != nullptr)
   {
      gobber->ShowWindow("Model Gobber");
   }
   if (sponza != nullptr)
   {
      sponza->ShowWindow("Model Sponza");
   }

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

            for (std::vector<class DrawLighted *>::iterator it = lightedObjects.begin(); it != lightedObjects.end(); ++it)
            {
               DrawLighted *c = *it;
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
