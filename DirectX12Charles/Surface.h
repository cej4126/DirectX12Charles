#pragma once
#include "stdafx.h"

class Surface
{
public:
   class Color
   {
   public:
      unsigned int dword;
      constexpr Color() noexcept : dword()
      {}
      // copy
      constexpr Color(const Color &color) noexcept
         :
         dword(color.dword)
      {}
      constexpr Color(unsigned int dw) noexcept
         :
         dword(dw)
      {}
      constexpr Color(unsigned char a, unsigned char r, unsigned char g, unsigned char b) noexcept
         :
         dword(b | (g << 8u) | (r << 16u) | (a << 24u) )
      {}
      constexpr Color(unsigned char r, unsigned char g, unsigned char b) noexcept
         :
         dword(b | (g << 8u) | (r << 16u))
      {}
      constexpr Color(Color color, unsigned char a) noexcept
         :
         Color(color.dword | (a << 24u))
      {}
      Color &operator = (Color color) noexcept
      {
         dword = color.dword;
         return *this;
      }
      constexpr unsigned char GetA() const noexcept
      {
         return dword >> 24u;
      }
      constexpr unsigned char GetR() const noexcept
      {
         return (dword >> 16u) & 0xffu;
      }
      constexpr unsigned char GetG() const noexcept
      {
         return (dword >> 8u) & 0xffu;
      }
      constexpr unsigned char GetB() const noexcept
      {
         return dword & 0xffu;
      }
      void SetA(unsigned char a) noexcept
      {
         dword = (dword & 0x00ffffff) | (a << 24u);
      }
      void SetR(unsigned char r) noexcept
      {
         dword = (dword & 0xff00ffff) | (r << 16u);
      }
      void SetG(unsigned char g) noexcept
      {
         dword = (dword & 0xffff00ff) | (g << 8u);
      }
      void SetB(unsigned char b) noexcept
      {
         dword = (dword & 0xffffff00) | b;
      }
   };

public:
   Surface(unsigned int width, unsigned int height, unsigned int pitch) noexcept;
   Surface(unsigned int width, unsigned int height) noexcept;
   Surface(Surface &&source) noexcept;
   Surface(Surface &) = delete;
   Surface &operator=(Surface && source) noexcept;
   Surface &operator=(const Surface &) = delete;
   ~Surface();

   void Clear(Color fillValue) noexcept;
   void PutPixel(unsigned int x, unsigned int y, Color c) noexcept;
   Color GetPixel(unsigned int x, unsigned int y) const noexcept;
   unsigned int GetWidth() const noexcept;
   unsigned int GetHeight() const noexcept;
   Color *GetBufferPtr() noexcept;
   const Color *GetBufferPtr() const noexcept;
   static Surface FromFile(const std::string &filename);
   void Save(const std::string &filename) const;
   void Copy(const Surface &src) noexcept;
private:
   Surface(unsigned int width, unsigned int height, std::unique_ptr<Color[]> pBufferParam) noexcept;
   std::unique_ptr<Color[]> pBuffer;
   unsigned int width;
   unsigned int height;

};

