#pragma once
#include "stdafx.h"
#include "Window.h"
#include "dwritedraw.h"
#include "ShapeColorIndex.h"
#include "ShapeColorBlended.h"
#include "ShapePicture.h"
#include "ShapeTextureCube.h"
#include "ShapeLighted.h"
#include "ShapePointLight.h"
#include "ShapeAssimp.h"
#include "Camera.h"
#include "Mesh.h"

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
   //void ShowRawInputWindow();

private:
   std::chrono::steady_clock::time_point lastTime;
   std::unique_ptr< class dwritedraw> dwriteitem;

   std::vector<std::unique_ptr<class DrawFunction>> drawItems;
   std::vector<class ShapeLighted * > lightedObjects;
   std::unique_ptr<class ShapePointLight > light;

   Camera cam;
   float speedFactor = 1.0f;
   int objectIndex = 1;
   ShapeLighted *currentObject = nullptr;
   std::unique_ptr<class Model > nano;
   //int x = 0, y = 0;
};

