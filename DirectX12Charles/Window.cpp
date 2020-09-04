#include "stdafx.h"
#include "Window.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"


// Window Stuff
Window::Window(int width, int height)
   :
   width(width),
   height(height)
{
   hInstance = GetModuleHandle(nullptr);

   WNDCLASSEX wc = { 0 };
   wc.cbSize = sizeof(wc);
   wc.style = CS_OWNDC;
   wc.lpfnWndProc = HandleMsgInit;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   wc.hInstance = hInstance;
   wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
   wc.hCursor = nullptr;
   wc.hbrBackground = nullptr;
   wc.lpszMenuName = nullptr;
   wc.lpszClassName = WindowName;
   wc.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
   RegisterClassEx(&wc);

   // calculate window size based on desired client region size
   RECT wr;
   wr.left = 100;
   wr.right = width + wr.left;
   wr.top = 100;
   wr.bottom = height + wr.top;
   if (AdjustWindowRect(&wr, WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU, FALSE) == 0)
   {
      throw;
   }
   // create window & get hWnd
   hWnd = CreateWindow(
      WindowName, WindowName,
      WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
      CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
      nullptr, nullptr, hInstance, this);
   // check for error
   if (hWnd == nullptr)
   {
      throw;
   }
   // newly created windows start off as hidden
   ShowWindow(hWnd, SW_SHOWDEFAULT);

   // create graphics object
   pGfx = std::make_unique<Graphics>(hWnd, width, height);
   graphicActive = true;
}

std::optional<int> Window::ProcessMessages() noexcept
{
   MSG msg;
   // while queue has messages, remove and dispatch them (but do not block on empty queue)
   while (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE))
   {
      // check for quit because peekmessage does not signal this via return val
      if (msg.message == WM_QUIT)
      {
         // return optional wrapping int (arg to PostQuitMessage is in wparam) signals quit
         return (int)msg.wParam;
      }

      // TranslateMessage will post auxilliary WM_CHAR messages from key msgs
      TranslateMessage(&msg);
      DispatchMessage(&msg);
   }

   // return empty optional when not quitting app
   return {};
}

Window::~Window()
{
   DestroyWindow(hWnd);
   UnregisterClass(WindowName, hInstance);
}

Graphics &Window::Gfx()
{
   if (!pGfx)
   {
      throw;
   }
   return *pGfx;
}

LRESULT CALLBACK Window::HandleMsgInit(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   // Wait for window construction to get the this pointer
   if (msg == WM_NCCREATE)
   {
      // Put the main handler with the this pointer
      const CREATESTRUCT *const pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
      Window *const pWin = static_cast<Window *>(pCreate->lpCreateParams);
      SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWin));
      SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::HandleMsgMain));
   }
   return DefWindowProc(hwnd, msg, wParam, lParam);
}


LRESULT CALLBACK Window::HandleMsgMain(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   Window *const pWin = reinterpret_cast<Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
   return pWin->HandleMsg(hwnd, msg, wParam, lParam);
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Window::HandleMsg(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
   {
      return true;
   }
   bool imguiActive = false;
   if (graphicActive)
   {
      const auto &imio = ImGui::GetIO();
      switch (msg)
      {
         case WM_KEYDOWN:
         case WM_SYSKEYDOWN:
         case WM_KEYUP:
         case WM_SYSKEYUP:
         case WM_CHAR:
         case WM_MOUSEMOVE:
         case WM_LBUTTONDOWN:
         case WM_RBUTTONDOWN:
         case WM_LBUTTONUP:
         case WM_RBUTTONUP:
         case WM_MOUSEWHEEL:
            // stifle this keyboard message if imgui wants to capture
            if (imio.WantCaptureKeyboard)
            {
               imguiActive = true;
            }
            break;

            //case WM_LBUTTONDOWN:
            //{
            //   // Not sure about the SetForegroundWindow
            //   SetForegroundWindow(hWnd);
            //   if (imio.WantCaptureKeyboard)
            //   {
            //      imguiActive = true;
            //   }
            //   break;
      }
   }

   // If imGui does not want the input
   if (!imguiActive)
   {
      switch (msg)
      {
         case WM_CLOSE:
            running = false;
            pGfx->CleanUp();

            PostQuitMessage(0);
            return 0;
         case WM_KILLFOCUS:
            input.ClearState();
            break;

         case WM_KEYDOWN:
         case WM_SYSKEYDOWN:

            if (!(lParam & 0x40000000) || (input.AutorepeatIsEnabled()))
            {
               input.OnKeyPressed(static_cast<unsigned char>(wParam));
            }
            break;
            return 0;
            break;
         case WM_KEYUP:
         case WM_SYSKEYUP:
            input.OnKeyReleased(static_cast<unsigned char>(wParam));
            break;

         case WM_CHAR:
            input.OnChar(static_cast<unsigned char>(wParam));
            break;

         case WM_MOUSEMOVE:
         {
            const POINTS pt = MAKEPOINTS(lParam);
            if ((pt.x >= 0) && (pt.x < width) && (pt.y >= 0) && (pt.y < height))
            {
               input.OnMouseMove(pt.x, pt.y);
               if (!input.IsInWindow())
               {
                  SetCapture(hWnd);
                  input.OnMouseEnter();
               }
            }
            else
            {
               if (wParam & (MK_LBUTTON | MK_RBUTTON))
               {
                  input.OnMouseMove(pt.x, pt.y);
               }
               // button up -> release capture / log event for leaving
               else
               {
                  ReleaseCapture();
                  input.OnMouseLeave();
               }
            }
            break;
         }

         case WM_LBUTTONDOWN:
         {
            SetForegroundWindow(hWnd);
            const POINTS pt = MAKEPOINTS(lParam);
            input.OnLeftPressed(pt.x, pt.y);
            break;
         }
         case WM_RBUTTONDOWN:
         {
            const POINTS pt = MAKEPOINTS(lParam);
            input.OnRightPressed(pt.x, pt.y);
            break;
         }
         case WM_LBUTTONUP:
         {
            const POINTS pt = MAKEPOINTS(lParam);
            input.OnLeftReleased(pt.x, pt.y);
            // release mouse if outside of window
            if (pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height)
            {
               ReleaseCapture();
               input.OnMouseLeave();
            }
            break;
         }
         case WM_RBUTTONUP:
         {
            const POINTS pt = MAKEPOINTS(lParam);
            input.OnRightReleased(pt.x, pt.y);
            // release mouse if outside of window
            if (pt.x < 0 || pt.x >= width || pt.y < 0 || pt.y >= height)
            {
               ReleaseCapture();
               input.OnMouseLeave();
            }
            break;
         }
         case WM_MOUSEWHEEL:
         {
            const POINTS pt = MAKEPOINTS(lParam);
            const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            input.OnWheelDelta(pt.x, pt.y, delta);
            break;
         }

         default:
            break;
      }
   }

   return DefWindowProc(hwnd, msg, wParam, lParam);
}
