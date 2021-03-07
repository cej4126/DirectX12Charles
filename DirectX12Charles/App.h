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
#include "DrawGobber.h"

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
   void SpawnSimulation();
   void SpawnObjectControl();

private:
   std::chrono::steady_clock::time_point lastTime;
   std::unique_ptr< class dwritedraw> dwriteitem;

   std::vector<std::unique_ptr<class DrawFunction>> drawItems;
   std::vector<class DrawLighted * > lightedObjects;
   std::unique_ptr<class DrawPointLight > light;
   std::unique_ptr<class DrawNormal > plane = nullptr;
   std::unique_ptr<class DrawNormal > cube = nullptr;

   Camera cam;
   float speedFactor = 1.0f;
   int objectIndex = 1;
   DrawLighted *currentObject = nullptr;

   std::unique_ptr<class DrawModel > nano = nullptr;
   std::unique_ptr<class DrawModel > wall = nullptr;
   std::unique_ptr<class DrawGobber > gobber = nullptr;
};

