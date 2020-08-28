#pragma once
#include "stdafx.h"
#include "Window.h"
#include "dwritedraw.h"
#include "ShapeColorIndexX11.h"
#include "ShapeColorIndex.h"
#include "ShapeColorBlendedX11.h"
#include "ShapeColorBlended.h"
#include "ShapeTextureX11.h"
#include "ShapeTexture.h"

#include "OneBoxX11.h"
#include "OneBoxX12.h"

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

   std::vector<std::unique_ptr<class DrawX11>> drawItemsX11;
   std::vector<std::unique_ptr<class DrawX12>> drawItems;

   std::unique_ptr<class OneBoxX11 > oneCubeColorIndexX11;
   std::unique_ptr<class OneBoxX12 > oneCubeColorIndex;
};

