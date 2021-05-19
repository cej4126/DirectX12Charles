#pragma once
#include "stdafx.h"
#include "Input.h"
#include "Graphics.h"

class Window
{
public:
   Window(int width, int height);
   Window(const Window &) = delete;
   Window &operator=(const Window &) = delete;
   static std::optional<int> processMessages() noexcept;
   ~Window();

   void enableCursor() noexcept;
   void disableCursor() noexcept;
   bool cursorEnabled() const noexcept;

   Graphics &gfx();
   Input m_input;
   bool m_running = true;

private:
   void confineCursor() noexcept;
   void freeCursor() noexcept;
   void showCursor() noexcept;
   void hideCursor() noexcept;
   void enableImGuiMouse() noexcept;
   void disableImGuiMouse() noexcept;

   static LRESULT CALLBACK handleMsgInit(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
   static LRESULT CALLBACK handleMsgMain(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
   LRESULT handleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
   //LPCTSTR WindowName = L"Charles";
   //LPCTSTR WindowName = L"Charles";
   static constexpr const char *Window_Name = "Charles";
   int m_width;
   int m_height;
   HWND m_hWnd;
   HINSTANCE m_hInstance;
   std::unique_ptr<Graphics> m_pGfx;
   bool m_graphicActive = false;
   bool m_cursorEnabled = true;
   std::vector<BYTE> m_rawBuffer;
};
