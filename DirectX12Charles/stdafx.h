#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <Windows.h>
#include <optional>
#include <memory>
#include <vector>
#include <wrl.h>
#include <d3d12.h>
#include <dxgi1_6.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>

inline void ThrowIfFailed(HRESULT hr)
{
   if (hr != S_OK)
   {
      throw;
   }
}