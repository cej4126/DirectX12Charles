#include "stdafx.h"
#include "Window.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_dx11.h"
#include "imgui/imgui_impl_win32.h"


// Window Stuff
Window::Window(int width, int height)
   :
   m_width(width),
   m_height(height)
{
   m_hInstance = GetModuleHandle(nullptr);

   WNDCLASSEX wc = { 0 };
   wc.cbSize = sizeof(wc);
   wc.style = CS_OWNDC;
   wc.lpfnWndProc = handleMsgInit;
   wc.cbClsExtra = 0;
   wc.cbWndExtra = 0;
   wc.hInstance = m_hInstance;
   wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
   wc.hCursor = nullptr;
   wc.hbrBackground = nullptr;
   wc.lpszMenuName = nullptr;
   wc.lpszClassName = Window_Name;
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
   m_hWnd = CreateWindow(
      Window_Name, Window_Name,
      WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU,
      CW_USEDEFAULT, CW_USEDEFAULT, wr.right - wr.left, wr.bottom - wr.top,
      nullptr, nullptr, m_hInstance, this);
   // check for error
   if (m_hWnd == nullptr)
   {
      throw;
   }
   // newly created windows start off as hidden
   ShowWindow(m_hWnd, SW_SHOWDEFAULT);

   // create graphics object
   m_pGfx = std::make_unique<Graphics>(m_hWnd, width, height);
   m_graphicActive = true;

   // register mouse raw input device
   RAWINPUTDEVICE rid;
   rid.usUsagePage = 0x01; // mouse page
   rid.usUsage = 0x02; // mouse usage
   rid.dwFlags = 0;
   rid.hwndTarget = nullptr;
   if (RegisterRawInputDevices(&rid, 1, sizeof(rid)) == FALSE)
   {
      throw;
   }
}

std::optional<int> Window::processMessages() noexcept
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
   DestroyWindow(m_hWnd);
   UnregisterClass(Window_Name, m_hInstance);
}

void Window::enableCursor() noexcept
{
   m_cursorEnabled = true;
   showCursor();
   enableImGuiMouse();
   freeCursor();
}

void Window::disableCursor() noexcept
{
   m_cursorEnabled = false;
   hideCursor();
   disableImGuiMouse();
   confineCursor();
}

Graphics &Window::gfx()
{
   if (!m_pGfx)
   {
      throw;
   }
   return *m_pGfx;
}

void Window::confineCursor() noexcept
{
   RECT rect;
   GetClientRect(m_hWnd, &rect);
   MapWindowPoints(m_hWnd, nullptr, reinterpret_cast<POINT *>(&rect), 2);
   ClipCursor(&rect);
}

void Window::freeCursor() noexcept
{
   ClipCursor(nullptr);
}

void Window::hideCursor() noexcept
{
   while (::ShowCursor(FALSE) >= 0);
}

void Window::showCursor() noexcept
{
   while (::ShowCursor(TRUE) < 0);
}

void Window::enableImGuiMouse() noexcept
{
   ImGui::GetIO().ConfigFlags &= ~ImGuiConfigFlags_NoMouse;
}

void Window::disableImGuiMouse() noexcept
{
   ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_NoMouse;
}

bool Window::cursorEnabled() const noexcept
{
   return m_cursorEnabled;
}

LRESULT CALLBACK Window::handleMsgInit(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   // Wait for window construction to get the this pointer
   if (msg == WM_NCCREATE)
   {
      // Put the main handler with the this pointer
      const CREATESTRUCT *const pCreate = reinterpret_cast<CREATESTRUCT *>(lParam);
      Window *const pWin = static_cast<Window *>(pCreate->lpCreateParams);
      SetWindowLongPtr(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pWin));
      SetWindowLongPtr(hwnd, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&Window::handleMsgMain));
   }
   return DefWindowProc(hwnd, msg, wParam, lParam);
}


LRESULT CALLBACK Window::handleMsgMain(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   Window *const pWin = reinterpret_cast<Window *>(GetWindowLongPtr(hwnd, GWLP_USERDATA));
   return pWin->handleMsg(hwnd, msg, wParam, lParam);
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT Window::handleMsg(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
   if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
   {
      return true;
   }
   bool imguiActive = false;
   if (m_graphicActive)
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
            m_running = false;
            m_pGfx->cleanUp();

            PostQuitMessage(0);
            return 0;
         case WM_KILLFOCUS:
            m_input.ClearState();
            break;

         case WM_ACTIVATE:
            // confine/free cursor on window to foreground/background if cursor disabled
            if (!m_cursorEnabled)
            {
               if (wParam & WA_ACTIVE)
               {
                  confineCursor();
                  hideCursor();
               }
               else
               {
                  freeCursor();
                  showCursor();
               }
            }
            break;

         case WM_KEYDOWN:
         case WM_SYSKEYDOWN:

            if (!(lParam & 0x40000000) || (m_input.AutorepeatIsEnabled()))
            {
               m_input.OnKeyPressed(static_cast<unsigned char>(wParam));
            }
            break;
            return 0;
            break;
         case WM_KEYUP:
         case WM_SYSKEYUP:
            m_input.OnKeyReleased(static_cast<unsigned char>(wParam));
            break;

         case WM_CHAR:
            m_input.OnChar(static_cast<unsigned char>(wParam));
            break;

         case WM_MOUSEMOVE:
         {
            if (!m_cursorEnabled)
            {
               if (!m_input.IsInWindow())
               {
                  SetCapture(hWnd);
                  m_input.OnMouseEnter();
                  hideCursor();
               }
               break;
            }

            const POINTS pt = MAKEPOINTS(lParam);
            if ((pt.x >= 0) && (pt.x < m_width) && (pt.y >= 0) && (pt.y < m_height))
            {
               m_input.OnMouseMove(pt.x, pt.y);
               if (!m_input.IsInWindow())
               {
                  SetCapture(hWnd);
                  m_input.OnMouseEnter();
               }
            }
            else
            {
               if (wParam & (MK_LBUTTON | MK_RBUTTON))
               {
                  m_input.OnMouseMove(pt.x, pt.y);
               }
               // button up -> release capture / log event for leaving
               else
               {
                  ReleaseCapture();
                  m_input.OnMouseLeave();
               }
            }
            break;
         }

         case WM_LBUTTONDOWN:
         {
            SetForegroundWindow(hWnd);
            if (!m_cursorEnabled)
            {
               confineCursor();
               hideCursor();
            }

            const POINTS pt = MAKEPOINTS(lParam);
            m_input.OnLeftPressed(pt.x, pt.y);
            break;
         }
         case WM_RBUTTONDOWN:
         {
            const POINTS pt = MAKEPOINTS(lParam);
            m_input.OnRightPressed(pt.x, pt.y);
            break;
         }
         case WM_LBUTTONUP:
         {
            const POINTS pt = MAKEPOINTS(lParam);
            m_input.OnLeftReleased(pt.x, pt.y);
            // release mouse if outside of window
            if (pt.x < 0 || pt.x >= m_width || pt.y < 0 || pt.y >= m_height)
            {
               ReleaseCapture();
               m_input.OnMouseLeave();
            }
            break;
         }
         case WM_RBUTTONUP:
         {
            const POINTS pt = MAKEPOINTS(lParam);
            m_input.OnRightReleased(pt.x, pt.y);
            // release mouse if outside of window
            if (pt.x < 0 || pt.x >= m_width || pt.y < 0 || pt.y >= m_height)
            {
               ReleaseCapture();
               m_input.OnMouseLeave();
            }
            break;
         }
         case WM_MOUSEWHEEL:
         {
            const POINTS pt = MAKEPOINTS(lParam);
            const int delta = GET_WHEEL_DELTA_WPARAM(wParam);
            m_input.OnWheelDelta(pt.x, pt.y, delta);
            break;
         }

         /************** RAW MOUSE MESSAGES **************/
         case WM_INPUT:
         {
            if (!m_input.RawEnabled())
            {
               break;
            }

            UINT size;
            // first get the size of the input data
            if (GetRawInputData(
               reinterpret_cast<HRAWINPUT>(lParam),
               RID_INPUT,
               nullptr,
               &size,
               sizeof(RAWINPUTHEADER)) == -1)
            {
               // bail msg processing if error
               break;
            }
            m_rawBuffer.resize(size);
            // read in the input data
            if (GetRawInputData(
               reinterpret_cast<HRAWINPUT>(lParam),
               RID_INPUT,
               m_rawBuffer.data(),
               &size,
               sizeof(RAWINPUTHEADER)) != size)
            {
               // bail msg processing if error
               break;
            }
            // process the raw input data
            auto &ri = reinterpret_cast<const RAWINPUT &>(*m_rawBuffer.data());
            if (ri.header.dwType == RIM_TYPEMOUSE &&
               (ri.data.mouse.lLastX != 0 || ri.data.mouse.lLastY != 0))
            {
               m_input.OnRawDelta(ri.data.mouse.lLastX, ri.data.mouse.lLastY);
            }
            break;
         }

         default:
            break;
      }
   }

   return DefWindowProc(hWnd, msg, wParam, lParam);
}
