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
      return App{}.go();
   }
   catch (...)
   {
      //MessageBox(nullptr, L"Any Enception", L"Exception", MB_OK | MB_ICONEXCLAMATION);
      MessageBox(nullptr, "Any Enception", "Exception", MB_OK | MB_ICONEXCLAMATION);
   }
   return -1;
}