#include "BoxX11.h"
#include "Geometry.h"
using namespace std;

BoxX11::BoxX11(Graphics &gfx, float range)
   :
   range(range)
{
   random_device rd;
   mt19937 gen(rd());
   uniform_real_distribution<float> rand2pi(0.0f, 3.1415f * 2.0f);
   uniform_real_distribution<float> rand1_3pi(0.0f, 3.1415f * 0.7f);

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
      std::unique_ptr < ObjectX11 > object = std::make_unique<ObjectX11>(gfx);

      struct Vertex
      {
         XMFLOAT3 pos;
      };
      auto model = Cube::Make<Vertex>();
      //auto model = Plane::Make<Vertex>();
      //auto model = Cylinder::Make<Vertex>();
      //auto model = Cone::Make<Vertex>();
      //auto model = Prism::Make<Vertex>();
      //auto model = Sphere::Make<Vertex>();

      object->AddVertexBuffer(model.vertices);

      object->AddIndexBuffer(model.indices);

      object->AddShaders(L"VertexShaderX11.cso", L"PixelShaderX11.cso");

      struct ConstantBuffer2
      {
         struct
         {
            float r;
            float g;
            float b;
            float a;
         } face_colors[6];
      };
      const ConstantBuffer2 cb2 =
      {
         {
            {1.0f, 0.0f, 1.0f, 1.0f},
            {1.0f, 0.0f, 0.0f, 1.0f},
            {0.0f, 1.0f, 0.0f, 1.0f},
            {0.0f, 0.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 0.0f, 1.0f},
            {0.0f, 1.0f, 1.0f, 1.0f},
         }
      };

      // Pixel Constant Buffer
      object->AddPixelConstantBuffer(cb2);

      // Layout
      const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
      {
         { "PositionX",0,DXGI_FORMAT_R32G32B32_FLOAT,0,0,D3D11_INPUT_PER_VERTEX_DATA,0 },
      };
      object->AddInputLayout(ied);

      addStaticBind(std::move(object), (UINT)model.indices.size());
   }

   std::unique_ptr < TransformX11 > trans = std::make_unique<TransformX11>(gfx, *this);
   trans->AddTransformConstantBuffer();

   AddBind(std::move(trans));
}

void BoxX11::Update(float dt) noexcept
{
   boxRoll += boxRollRate * dt;
   boxPitch += boxPitchRate * dt;
   boxYaw += boxYawRate * dt;
   spaceRoll += spaceRollRate * dt;
   spacePitch += spacePitchRate * dt;
   spaceYaw += spaceYawRate * dt;
}

XMMATRIX BoxX11::GetTransformXM() const noexcept
{
#ifndef FIX_ROTATION
   return DirectX::XMMatrixRotationRollPitchYaw(boxPitch, boxYaw, boxRoll) *
      DirectX::XMMatrixTranslation(range, 0.0f, 0.0f) *
      DirectX::XMMatrixRotationRollPitchYaw(spacePitch, spaceYaw, spaceRoll) *
      DirectX::XMMatrixTranslation(8.0f, -4.0f, 20.0f);
#else
   return DirectX::XMMatrixRotationRollPitchYaw(boxPitch, boxYaw, boxRoll) *
      DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f) *
      DirectX::XMMatrixTranslation(0.0f, 0.0f, 20.0f);
#endif
}
