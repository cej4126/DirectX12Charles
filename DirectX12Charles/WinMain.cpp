#include "stdafx.h"
#include "App.h"

#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "D3DCompiler.lib")

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
		MessageBox(nullptr, L"Any Enception", L"Exception", MB_OK | MB_ICONEXCLAMATION);
	}
	return -1;
}