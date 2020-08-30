#pragma once
#include "stdafx.h"
#include "Input.h"
#include "Graphics.h"

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

class Window
{
public:
   Window(int width, int height);
   Window(const Window &) = delete;
   Window &operator=(const Window &) = delete;
   static std::optional<int> ProcessMessages() noexcept;
   ~Window();
   Graphics &Gfx();
   Input input;
   bool running = true;

private:
   static LRESULT CALLBACK HandleMsgInit(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
   static LRESULT CALLBACK HandleMsgMain(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
   LRESULT HandleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
   LPCTSTR WindowName = L"Charles";
   int width;
   int height;
   HWND hWnd;
   HINSTANCE hInstance;
   std::unique_ptr<Graphics> pGfx;
   bool graphicActive = false;
};
