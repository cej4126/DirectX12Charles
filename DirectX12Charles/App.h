#pragma once
#include "stdafx.h"
#include "Window.h"
#include "dwritedraw.h"
#include "DrawColorIndex.h"
#include "DrawColorBlended.h"
#include "DrawPictureCube.h"
#include "DrawNormal.h"
#include "DrawTextureCube.h"
#include "DrawLighted.h"
#include "DrawPointLight.h"
#include "DrawAssimp.h"
#include "Camera.h"
#include "DrawModel.h"

class App
{
public:
   App();
   int go();
   ~App();
private:
   float timeMark();
   float timePeek();

   void doFrame();
   Window m_window;

private:
   void spawnSimulation();
   void spawnObjectControl();

private:
   std::chrono::steady_clock::time_point m_lastTime;
   std::unique_ptr< class dwritedraw> m_dwriteitem;

   std::vector<std::unique_ptr<class DrawFunction>> m_drawItems;
   std::vector<class DrawLighted * > m_lightedObjects;
   std::unique_ptr<class DrawPointLight > m_light;
   std::unique_ptr<class DrawNormal > m_plane = nullptr;
   std::unique_ptr<class DrawNormal > m_cube = nullptr;

   Camera m_camera;
   float m_speedFactor = 1.0f;
   int m_objectIndex = 1;
   DrawLighted *m_currentObject = nullptr;

   std::unique_ptr<class DrawModel > m_nano = nullptr;
   std::unique_ptr<class DrawModel > m_wall = nullptr;
   std::unique_ptr<class DrawModel > m_gobber = nullptr;
   std::unique_ptr<class DrawModel > m_sponza = nullptr;
};

