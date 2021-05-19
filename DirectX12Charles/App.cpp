#include "App.h"
#include "Shape.h"
#include "Surface.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"
#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>

App::App()
   :
   m_window(1280, 720)
{
   std::mt19937 gen(2018);

   std::mt19937 rng(std::random_device{}());
   std::uniform_real_distribution<float> rangedist(3.0f, 15.0f);

   m_lastTime = std::chrono::steady_clock::now();

   m_dwriteitem = std::make_unique<dwritedraw>(m_window.gfx());

   int MaxBoxX12Count = 1;
   //int MaxBoxX12Count = 20;
   int MaterialCount = 0;
   int objectCount = 0;

   m_light = std::make_unique<DrawPointLight>(m_window.gfx(), objectCount, 0.5f);

   //plane = std::make_unique<DrawNormal>(m_window.gfx(),
   //   objectCount,
   //   Shape::Plane, 6.0f,
   //   "..\\..\\DirectX12Charles\\Images\\brickwall.jpg",
   //   "..\\..\\DirectX12Charles\\Images\\brickwall_normal.jpg",
   //   m_light->getLightView(), MaterialCount);
   //plane->SetPos(XMFLOAT3(15.0f, 5.0f, 20.0f));

   m_cube = std::make_unique<DrawNormal>(m_window.gfx(),
      objectCount,
      Shape::TextureCube, 1.0f,
      "..\\..\\DirectX12Charles\\Images\\brickwall.jpg",
      "..\\..\\DirectX12Charles\\Images\\brickwall_normal.jpg",
      m_light->getLightView(), MaterialCount);
   //m_cube->SetPos(XMFLOAT3(-5.0f, 7.0f, 0.0f));

   //m_drawItems.push_back(std::make_unique<DrawNormal>(m_window.gfx(), objectCount, Shape::Plane, 3.0f,
   //   "..\\..\\DirectX12Charles\\Images\\brickwall.jpg", "..\\..\\DirectX12Charles\\Images\\brickwall_normal.jpg", m_light->getLightView(), MaterialCount));

   //m_nano = std::make_unique<DrawModel>(m_window.gfx(), objectCount, 1.0f,
   //   "..\\..\\DirectX12Charles\\Models\\nano_textured\\nanosuit.obj",
   //   m_light->getLightView(), MaterialCount);

   //m_wall = std::make_unique<DrawModel>(m_window.gfx(), objectCount, 5.0f,
   //   "..\\..\\DirectX12Charles\\models\\brick_wall\\brick_wall.obj",
   //   m_light->getLightView(), MaterialCount);
   //m_wall->SetPosition(XMMatrixTranslation(-4.0f, 7.0f, 0.0f ));

   //m_gobber = std::make_unique<DrawModel>(m_window.gfx(), objectCount, 5.0f,
   //   "..\\..\\DirectX12Charles\\models\\gobber\\GoblinX.obj",
   //   m_light->getLightView(), MaterialCount);
   //m_gobber->SetPosition(XMMatrixTranslation(-4.0f, 7.0f, 0.0f));

   //m_sponza = std::make_unique<DrawModel>(m_window.gfx(), objectCount, 1.0f / 20.0f,
   //   "..\\..\\DirectX12Charles\\models\\sponza\\sponza.obj",
   //   m_light->getLightView(), MaterialCount);
   //m_sponza->SetPosition(XMMatrixTranslation(0.0f, 0.0f, 0.0f));

   for (auto i = 0; i < MaxBoxX12Count; i++)
   {
      float range = rangedist(rng);

      Shape::shapeType type = static_cast<Shape::shapeType>(Shape::TextureCube + (i % 4));
      //m_drawItems.push_back(std::make_unique<DrawLighted>(m_window.gfx(), objectCount, type, range, m_light->getLightView(), MaterialCount));
      //m_drawItems.push_back(std::make_unique<DrawAssimp>(m_window.gfx(), objectCount, Shape::TextureSuzanne, range, m_light->getLightView(), MaterialCount));
      //m_drawItems.push_back(std::make_unique<DrawColorBlended>(m_window.gfx(), objectCount, type, range));
      //m_drawItems.push_back(std::make_unique<DrawColorIndex>(m_window.gfx(), objectCount, type, range));
      //m_drawItems.push_back(std::make_unique<DrawTextureCube>(m_window.gfx(), objectCount, range));
      m_drawItems.push_back(std::make_unique<DrawPictureCube>(m_window.gfx(), objectCount, Shape::TextureCube, range, "..\\..\\DirectX12Charles\\Images\\280893.jpg"));
      //m_drawItems.push_back(std::make_unique<DrawPictureCube>(m_window.gfx(), objectCount, Shape::TextureCube, range, "..\\..\\DirectX12Charles\\Images\\cobalt-city.jpg"));
      //m_drawItems.push_back(std::make_unique<DrawPictureCube>(m_window.gfx(), objectCount, Shape::TextureCube, range, "..\\..\\DirectX12Charles\\Images\\picture3.jpg"));
   }

   m_window.gfx().createMatrixConstant(objectCount);
   m_window.gfx().createMaterialConstant(MaterialCount);

   for (auto &b : m_drawItems)
   {
      if (auto po = dynamic_cast<DrawLighted *>(b.get()))
      {
         m_lightedObjects.push_back(po);
      }

      int materialIndex = b->getMaterialIndex();
      if (materialIndex != -1)
      {
         Graphics::MaterialType material;
         b->getMaterialData(material);
         m_window.gfx().copyMaterialConstant(materialIndex, material);
      }
   }

   if (m_nano != nullptr)
   {
      m_nano->FirstCommand();
   }
   if (m_wall != nullptr)
   {
      m_wall->FirstCommand();
   }
   if (m_gobber != nullptr)
   {
      m_gobber->FirstCommand();
   }
   if (m_sponza != nullptr)
   {
      m_sponza->FirstCommand();
   }

   m_window.gfx().runCommandList();
   m_window.gfx().setProjection(XMMatrixPerspectiveLH(1.0f, 9.0f / 16.0f, 0.5f, 400.0f));
}

int App::go()
{
   while (m_window.m_running)
   {
      if (const auto code = Window::processMessages())
      {
         return *code;
      }
      if (m_window.m_running)
      {
         doFrame();
      }
   }

   return 0;
}

App::~App()
{
}

float App::timeMark()
{
   const auto oldTime = m_lastTime;
   m_lastTime = std::chrono::steady_clock::now();
   const std::chrono::duration<float> frameTime = m_lastTime - oldTime;
   return frameTime.count();
}

float App::timePeek()
{
   return std::chrono::duration<float>(std::chrono::steady_clock::now() - m_lastTime).count();
}

void App::doFrame()
{
   auto dt = timeMark() * m_speedFactor;
   m_window.gfx().setCamera(m_camera.getMatrix());

   m_window.gfx().onRenderBegin();

   m_window.gfx().onRender();

   int index = 0;

   auto &lightObject = m_light;
   if (m_light != nullptr)
   {
      lightObject->Draw(m_window.gfx());
   }

   if (m_plane != nullptr)
   {
      m_plane->Draw(m_window.gfx());
   }
   if (m_cube != nullptr)
   {
      m_cube->Draw(m_window.gfx());
   }
   if (m_nano != nullptr)
   {
      m_nano->Draw(m_window.gfx());
   }
   if (m_wall != nullptr)
   {
      m_wall->Draw(m_window.gfx());
   }
   if (m_gobber != nullptr)
   {
      m_gobber->Draw(m_window.gfx());
   }
   if (m_sponza != nullptr)
   {
      m_sponza->Draw(m_window.gfx());
   }

   for (auto &b : m_drawItems)
   {
      b->Update(dt);
      b->Draw(m_window.gfx());
   }

   m_dwriteitem->draw();

   while (auto e = m_window.m_input.ReadKey())
   {
      if (e->isPress())
      {
         switch (e->GetCode())
         {
            //case VK_INSERT:
            case VK_ESCAPE:
               if (m_window.cursorEnabled())
               {
                  m_window.disableCursor();
                  m_window.m_input.enableRaw();
               }
               else
               {
                  m_window.enableCursor();
                  m_window.m_input.disableRaw();
               }
               break;
         }
      }
   }

   if (!m_window.cursorEnabled())
   {
      if (m_window.m_input.keyIsPressed('W'))
      {
         m_camera.translate({ 0.0f, 0.0f, dt });
      }
      else if (m_window.m_input.keyIsPressed('S'))
      {
         m_camera.translate({ 0.0f, 0.0f, -dt });
      }
      else if (m_window.m_input.keyIsPressed('A'))
      {
         m_camera.translate({ -dt, 0.0f, 0.0f });
      }
      else if (m_window.m_input.keyIsPressed('D'))
      {
         m_camera.translate({ dt, 0.0f, 0.0f });
      }
      else if (m_window.m_input.keyIsPressed('Q'))
      {
         m_camera.translate({ 0.0f, dt, 0.0f });
      }
      else if (m_window.m_input.keyIsPressed('E'))
      {
         m_camera.translate({ 0.0f, -dt, 0.0f });
      }

      while (const auto delta = m_window.m_input.readRawDelta())
      {
         if (!m_window.cursorEnabled())
         {
            m_camera.rotate((float)delta->x, (float)delta->y);
         }
      }
   }

   spawnSimulation();

   m_camera.createControlWindow();
   if (m_light != nullptr)
   {
      lightObject->CreateLightControl();
   }

   spawnObjectControl();

   if (m_plane != nullptr)
   {
      m_plane->SpawnControlWindow("plane");
   }
   if (m_cube != nullptr)
   {
      m_cube->SpawnControlWindow("cube");
   }
   if (m_nano != nullptr)
   {
      m_nano->ShowWindow("Model Nano");
   }
   if (m_wall != nullptr)
   {
      m_wall->ShowWindow("Model Wall");
   }
   if (m_gobber != nullptr)
   {
      m_gobber->ShowWindow("Model Gobber");
   }
   if (m_sponza != nullptr)
   {
      m_sponza->ShowWindow("Model Sponza");
   }

   ImGui::Render();
   ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());

   m_window.gfx().drawCommandList();

   m_window.gfx().onRenderEnd();

}

void App::spawnSimulation()
{
   // Start the ImGui frame
   ImGui_ImplDX11_NewFrame();
   ImGui_ImplWin32_NewFrame();
   ImGui::NewFrame();
   ImGui::Begin("Simulation Speed");

   ImGui::SliderFloat("Speed Factor", &m_speedFactor, 0.0f, 4.0f);
   ImGui::Text("Application average %.3f ms/frame (%.1f FPS)",
      1000.0f / ImGui::GetIO().Framerate,
      ImGui::GetIO().Framerate);
   ImGui::End();
}

void App::spawnObjectControl()
{
   static bool firstTime = true;

   if (!m_lightedObjects.empty())
   {
      if (ImGui::Begin(("Object " + std::to_string(m_objectIndex)).c_str()))
      {
         bool changed = ImGui::InputInt("Id", &m_objectIndex) || firstTime;
         if (changed)
         {
            firstTime = false;
            if (m_objectIndex < 1)
            {
               m_objectIndex = 1;
            }
            else if (m_objectIndex > m_lightedObjects.size())
            {
               m_objectIndex = (int)m_lightedObjects.size();
            }

            for (std::vector<class DrawLighted *>::iterator it = m_lightedObjects.begin(); it != m_lightedObjects.end(); ++it)
            {
               DrawLighted *c = *it;
               int i = c->getMaterialIndex() + 1;
               if (m_objectIndex == i)
               {
                  m_currentObject = c;
               }
            }
         }
         if (m_currentObject != nullptr)
         {
            m_currentObject->SpawnControlWindow();
         }
      }
      ImGui::End();
   }
}
