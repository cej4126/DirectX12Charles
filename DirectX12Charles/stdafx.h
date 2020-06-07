#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <d3d12.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>


inline void ThrowIfFailed(HRESULT hr)
{
   if (hr != S_OK)
   {
      throw;
   }
}