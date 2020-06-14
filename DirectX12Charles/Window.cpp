#include "stdafx.h"
#include "Window.h"

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
	wc.lpfnWndProc = WndProc;
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

LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	switch (msg)
	{
		case WM_KEYDOWN:
			if (wParam == VK_ESCAPE)
			{
	//			//if (MessageBox(0, L"Quit?", L"Quit", MB_YESNO | MB_ICONQUESTION) == IDYES)
				if (MessageBox(0, L"Quit?", L"Quit", MB_YESNO | MB_ICONQUESTION) == IDYES)
				{
					//running = false;
					DestroyWindow(hwnd);
				}
			}
			return 0;
			break;
	//	case WM_MOUSEMOVE:
	//	{
	//		if (wParam == MK_LBUTTON)
	//		{
	//			XMFLOAT2 location;
	//			location.x = (float)GET_X_LPARAM(lParam);
	//			location.y = (float)GET_Y_LPARAM(lParam);
	//			bool mouseDownLast = !mouseDown;
	//			// xapp OnMouseMove
	//			mouseDown = true;
	//		}
	//		else
	//		{
	//			mouseDown = false;
	//		}
	//		return 0;
	//	}
		case WM_DESTROY:
			//running = false;
			// xapp setrunning
			PostQuitMessage(0);
			return 0;
		default:
			break;
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}
