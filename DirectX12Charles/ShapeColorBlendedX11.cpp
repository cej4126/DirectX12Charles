#include "ShapeColorBlendedX11.h"
using namespace std;

ShapeColorBlendedX11::ShapeColorBlendedX11(Graphics &gfx, Shape::shapeType type, float range)
   :
   range(range)
{
   random_device rd;
   mt19937 gen(rd());
   uniform_real_distribution<float> rand2pi(0.0f, 3.1415f * 2.0f);
   uniform_real_distribution<float> rand1_3pi(0.0f, 3.1415f * 0.7f);
   uniform_real_distribution<float> randcolor(0.0f, 1.0f);

   boxRoll = rand2pi(gen);
   boxPitch = rand2pi(gen);
   boxYaw = rand2pi(gen);
   spaceRoll = rand2pi(gen);
   spacePitch = rand2pi(gen);
   spaceYaw = rand2pi(gen);

   boxRollRate = rand1_3pi(gen);
   boxPitchRate = rand1_3pi(gen);
   boxYawRate = rand1_3pi(gen);
   spaceRollRate = rand1_3pi(gen);
   spacePitchRate = rand1_3pi(gen);
   spaceYawRate = rand1_3pi(gen);

#ifdef FIX_ROTATION
   boxRoll = 0.0f * 3.1415f;
   boxPitch = 0.0 * 3.1415f;
   boxYaw = 0.0f * 3.1415f;
   boxRollRate = 0.0f;
   boxPitchRate = 0.5f;
   boxYawRate = 0.1f;

   spaceRoll = 0.0f;
   spacePitch = 0.0f;
   spaceYaw = 0.0f;
   spaceRollRate = 0.0f;
   spacePitchRate = 0.0f;
   spaceYawRate = 0.0f;
#endif

   if (!isStaticSet())
   {
      struct Vertex
      {
         XMFLOAT3 pos;
         struct
         {
            unsigned char r;
            unsigned char g;
            unsigned char b;
            unsigned char a;
         } color;
      };

      auto model = gfx.shape.GetShapeData<Vertex>();

      std::unique_ptr < ObjectX11 > object = std::make_unique<ObjectX11>(gfx);

      for (int i = 0; i < model.vertices.size(); i++)
      {
         unsigned char r = (UINT)(255 * randcolor(gen));
         unsigned char b = (UINT)(255 * randcolor(gen));
         unsigned char g = (UINT)(255 * randcolor(gen));
         model.vertices[i].color = { r, b, g, 255 };

      }

      object->AddVertexBuffer(model.vertices);
      object->AddIndexBuffer(model.indices);

      object->AddShaders(L"ColorBlendedVSX11.cso", L"ColorBlendedPSX11.cso");

      //// Pixel Constant Buffer
      object->AddPixelConstantBuffer(nullptr, false);

      // Layout
      const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
      {
         { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
         { "Color", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
      };
      object->AddInputLayout(ied);

      addStaticBind(std::move(object), (UINT)model.indices.size());
   }

   std::unique_ptr < TransformX11 > trans = std::make_unique<TransformX11>(gfx, *this);
   trans->AddTransformConstantBuffer();
   trans->setIndices(gfx.shape.getIndiceStart(type), gfx.shape.getIndiceCount(type));

   AddBind(std::move(trans));
}

void ShapeColorBlendedX11::Update(float dt) noexcept
{
   boxRoll += boxRollRate * dt;
   boxPitch += boxPitchRate * dt;
   boxYaw += boxYawRate * dt;
   spaceRoll += spaceRollRate * dt;
   spacePitch += spacePitchRate * dt;
   spaceYaw += spaceYawRate * dt;
}

XMMATRIX ShapeColorBlendedX11::GetTransformXM() const noexcept
{
#ifndef FIX_ROTATION
   return DirectX::XMMatrixRotationRollPitchYaw(boxPitch, boxYaw, boxRoll) *
      DirectX::XMMatrixTranslation(range, 0.0f, 0.0f) *
      DirectX::XMMatrixRotationRollPitchYaw(spacePitch, spaceYaw, spaceRoll);
#else
   return DirectX::XMMatrixRotationRollPitchYaw(boxPitch, boxYaw, boxRoll) *
      DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
#endif
}
