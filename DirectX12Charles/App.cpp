#include "App.h"

App::App()
   :
   wnd(1000, 800)
{

}

int App::Go()
{
   while (true)
   {
      if (const auto code = Window::ProcessMessages())
      {
         return *code;
      }
   }
   DoFrame();
   return 0;
}

App::~App()
{
}

void App::DoFrame()
{
}
