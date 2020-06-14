#pragma once
#include "stdafx.h"
//#include "Keyboard.h"
//#include "Mouse.h"
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
private:
	LPCTSTR WindowName = L"Charles";
	int width;
	int height;
	HWND hWnd;
	HINSTANCE hInstance;
	std::unique_ptr<Graphics> pGfx;
};
