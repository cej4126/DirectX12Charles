#include "BoxX11.h"

BoxX11::BoxX11(Graphics &gfx, float range)
   :
   range(range)
{
   std::mt19937 rng;
   std::uniform_real_distribution<float> rand2pi(0.0f, 3.1415f * 2.0f);
   std::uniform_real_distribution<float> rand1_3pi(0.0f, 3.1415f * 0.3f);

   boxRoll = rand2pi(rng);
   boxPitch = rand2pi(rng);
   boxYaw = rand2pi(rng);
   spaceRoll = rand2pi(rng);
   spacePitch = rand2pi(rng);
   spaceYaw = rand2pi(rng);

   boxRollRate = rand1_3pi(rng);
   boxPitchRate = rand1_3pi(rng);
   boxYawRate = rand1_3pi(rng);
   spaceRollRate = rand1_3pi(rng);
   spacePitchRate = rand1_3pi(rng);
   spaceYawRate = rand1_3pi(rng);

   if (!isStaticSet())
   {
      std::unique_ptr < ObjectX11 > object = std::make_unique<ObjectX11>(gfx);

      // Vertices
      const std::vector<Vertex> vertices =
      {
         { -1.0f,-1.0f,-1.0f },
         { 1.0f,-1.0f,-1.0f },
         { -1.0f,1.0f,-1.0f },
         { 1.0f,1.0f,-1.0f },
         { -1.0f,-1.0f,1.0f },
         { 1.0f,-1.0f,1.0f },
         { -1.0f,1.0f,1.0f },
         { 1.0f,1.0f,1.0f },
      };

      object->AddVertexBuffer(vertices);

      // indies
      const std::vector<unsigned short> indices =
      {
         0,2,1, 2,3,1,
         1,3,5, 3,7,5,
         2,6,3, 3,6,7,
         4,5,7, 4,7,6,
         0,4,2, 2,4,6,
         0,1,4, 1,5,4
      };
      object->AddIndexBuffer(indices);
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
            { 1.0f,0.0f,0.2f },
            { 1.0f,0.0f,0.3f },
            { 1.0f,0.0f,0.4f },
            { 1.0f,0.0f,0.5f },
            { 1.0f,0.0f,0.6f },
            { 1.0f,0.0f,0.7f },
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

      addStaticBind(std::move(object), (UINT)indices.size());
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
   return DirectX::XMMatrixRotationRollPitchYaw(boxPitch, boxYaw, boxRoll) *
      DirectX::XMMatrixTranslation(range, 0.0f, 0.0f) *
      DirectX::XMMatrixRotationRollPitchYaw(spacePitch, spaceYaw, spaceRoll) *
      DirectX::XMMatrixTranslation(0.0f, 0.0f, 20.0f);
}
