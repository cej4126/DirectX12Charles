#include "stdafx.h"
#include "App.h"

int CALLBACK WinMain(
   HINSTANCE hInstance,
   HINSTANCE hPrevInstance,
   LPSTR     lpCmdLine,
   int       nCmdShow)
{
   try
   {
      return App{}.Go();
   }
   catch (...)
   {
      MessageBox(nullptr, L"Exception any", L"Exception", MB_OK | MB_ICONEXCLAMATION);
   }
   return -1;
}