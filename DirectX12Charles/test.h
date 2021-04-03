#pragma once
#include "stdafx.h"

class testFunction
{
public:
   typedef struct
   {
      int heightCount;
      int widthCount;
      std::string filename;
   } TestData;
   std::vector <TestData> testData;
   int totalHeightCount = 0;
   int totalWidthCount = 0;

   void addTestData(int height, int width, std::string filename)
   {
      if ((height == 0) || (width == 0))
      {
         int i = 0;
      }
      else
      {
         TestData data = { height, width, filename };
         testData.push_back(data);
      }
      totalHeightCount += height;
      totalWidthCount += width;
   }
};

extern testFunction test;