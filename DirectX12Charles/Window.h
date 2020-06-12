#pragma once
#include "stdafx.h"
//#include "Keyboard.h"
//#include "Mouse.h"
//#include "Graphics.h"
//#include <optional>
//#include <memory>


class Window
{
public:
	Window(HINSTANCE hIinst, int width, int height, LPSTR name);
	Window(const Window &) = delete;
	Window &operator=(const Window &) = delete;
	~Window();
private:
	LPSTR name;
	int width;
	int height;
	HWND hWnd;
	HINSTANCE hIinst;
};
