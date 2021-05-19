#include "Surface.h"
#include <wchar.h>
#include <filesystem>

Surface::Surface(unsigned int width, unsigned int height)
{
   HRESULT hr = m_scratch.Initialize2D(m_format, width, height, 1u, 1u);
   if (FAILED(hr))
   {
      throw ("Failed to initialize ScratchImage");
   }
}

void Surface::clear(Color fillValue) noexcept
{
   const auto width = getWidth();
   const auto height = getHeight();
   auto &imageData = *m_scratch.GetImage(0, 0, 0);
   for (size_t y = 0u; y < height; y++)
   {
      auto rowStart = reinterpret_cast<Color *>(imageData.pixels + imageData.rowPitch * y);
      std::fill(rowStart, rowStart + imageData.width, fillValue);
   }
}

void Surface::putPixel(unsigned int x, unsigned int y, Color c) noexcept
{
   assert(x >= 0);
   assert(y >= 0);
   assert(x <getWidth());
   assert(y < getHeight());
   auto &imageData = *m_scratch.GetImage(0, 0, 0);
   reinterpret_cast<Color *>(&imageData.pixels[y * imageData.rowPitch])[x] = c;
}

Surface::Color Surface::getPixel(unsigned int x, unsigned int y) const noexcept
{
   assert(x >= 0);
   assert(y >= 0);
   assert(x < getWidth());
   assert(y < getHeight());
   auto &imageData = *m_scratch.GetImage(0, 0, 0);
   return reinterpret_cast<Color *>(&imageData.pixels[y * imageData.rowPitch])[x];
}

unsigned int Surface::getWidth() const noexcept
{
   return (unsigned int)m_scratch.GetMetadata().width;
}

unsigned int Surface::getHeight() const noexcept
{
   return (unsigned int)m_scratch.GetMetadata().height;
}

Surface::Color *Surface::getBufferPtr() noexcept
{
   return reinterpret_cast<Color *>(m_scratch.GetPixels());
}

const const Surface::Color *Surface::getBufferPtr() const noexcept
{
   return const_cast<Surface *>(this)->getBufferPtr();
}

const Surface::Color *Surface::getBufferPtrConst() const noexcept
{
   return const_cast<Surface *>(this)->getBufferPtr();
}

Surface Surface::fromFile(const std::string &filename)
{
   wchar_t wideName[512];
   mbstowcs_s(nullptr, wideName, filename.c_str(), _TRUNCATE);

   DirectX::ScratchImage scratch;
   HRESULT hr = DirectX::LoadFromWICFile(wideName, DirectX::WIC_FLAGS_NONE, nullptr, scratch);
   if (FAILED(hr))
   {
      throw ("Failed to load image");
   }

   if (scratch.GetImage(0, 0, 0)->format != m_format)
   {
      DirectX::ScratchImage converted;
      hr = DirectX::Convert(
         *scratch.GetImage(0, 0, 0),
         m_format,
         DirectX::TEX_FILTER_DEFAULT,
         DirectX::TEX_THRESHOLD_DEFAULT,
         converted);
      if (FAILED(hr))
      {
         throw ("Failed to convert image");
      }
      return Surface(std::move(converted));
   }
   return Surface(std::move(scratch));
}

void Surface::save(const std::string &filename) const
{
   const auto GetCodecID = [](const std::string &filename)
   {
      const std::filesystem::path path = filename;
      const auto ext = path.extension().string();

      if (ext == ".png")
      {
         return DirectX::WIC_CODEC_PNG;
      }
      else if (ext == ".jpg")
      {
         return DirectX::WIC_CODEC_JPEG;
      }
      else if (ext == ".bmp")
      {
         return DirectX::WIC_CODEC_BMP;
      }
      throw ("Image format not supported");
   };

   wchar_t wideName[512];
   mbstowcs_s(nullptr, wideName, filename.c_str(), _TRUNCATE);

   HRESULT hr = DirectX::SaveToWICFile(
      *m_scratch.GetImage(0, 0, 0),
      DirectX::WIC_FLAGS_NONE,
      GetWICCodec(GetCodecID(filename)),
      wideName);

   if (FAILED(hr))
   {
      throw ("Failed to save image");
   }
}

bool Surface::alphaLoaded() const noexcept
{
   return !m_scratch.IsAlphaAllOpaque();
}

Surface::Surface(DirectX::ScratchImage scratch) noexcept
   :
   m_scratch(std::move(scratch))
{
}