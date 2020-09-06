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
#include "Camera.h"

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
   std::chrono::steady_clock::time_point lastTime;
   std::unique_ptr< class dwritedraw> dwriteitem;

   std::vector<std::unique_ptr<class DrawFunction>> drawItems;
   std::unique_ptr<class ShapePointLight > light;

   Camera cam;
   float speedFactor = 1.0f;
};

