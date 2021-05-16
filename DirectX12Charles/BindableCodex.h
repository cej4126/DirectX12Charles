
#pragma once
#include "stdafx.h"
#include "Bindable.h"
#include "Graphics.h"

namespace Bind
{
   class BindableCodex
   {
   public:
      template<class T, typename ...Params>
      static std::shared_ptr<T> Resolve(Graphics &gfx, Params&& ... p) noexcept
      {
         return Get().Resolve_<T>(gfx, std::forward<Params>(p) ...);
      }
   private:

      template<class T, typename...Params>
      std::shared_ptr<T> Resolve_(Graphics &gfx, Params&& ... p) noexcept
      {
         const auto key = T::GenerateUID(std::forward<Params>(p)...);
         const auto i = binds.find(key);
         if (i == binds.end())
         {
            auto bind = std::make_shared<T>(gfx, std::forward<Params>(p)...);
            binds[key] = bind;
            return bind;
         }
         else
         {
            return std::static_pointer_cast<T>(i->second);
         }
      }

      static BindableCodex &Get()
      {
         static BindableCodex codex;
         return codex;
      }
   private:
      std::unordered_map<std::string, std::shared_ptr<Bindable>> binds;
   };
}

