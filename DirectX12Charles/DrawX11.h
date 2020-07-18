#include "stdafx.h"
#include "Graphics.h"
#include "BindableX11.h"

class BindableX11;

class DrawX11
{
   template<class T>
   friend class DrawBaseX11;
public:
   DrawX11() = default;
   DrawX11(const DrawX11 &) = delete;
   void Draw(Graphics &gfx) const noexcept;
   virtual ~DrawX11() = default;

   virtual void Update(float dt) noexcept = 0;
   UINT count = 0;

protected:
   void AddBind(std::unique_ptr < BindableX11 > bind) noexcept;

private:
   std::vector<std::unique_ptr< BindableX11 >> binds;
};
