#pragma once
#include "Graphics.h"
#include "Surface.h"
#include "Vertex.h"

namespace Bind
{
   class Bindable
   {
   public:
      virtual void Bind(Graphics &gfx) noexcept = 0;
      virtual ~Bindable() = default;
      virtual std::string GetUID() const noexcept
      {
         assert(false);
         return "";
      }

      bool isInitialized() { return initialized; }
      void setInitialized() { initialized = true; }
      int getIndex() { return m_index; }
      void setIndex(int index) { m_index = index; }

   private:
      bool initialized = false;
      int m_index = -1;
   };
}
