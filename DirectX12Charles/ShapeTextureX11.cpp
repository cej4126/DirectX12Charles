#include "ShapeTextureX11.h"
using namespace std;

//#define FIX_ROTATION


ShapeTextureX11::ShapeTextureX11(Graphics &gfx, Shape::shapeType type, float range)
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
   boxPitchRate = 0.0f;
   boxYawRate = 0.0f;

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
            float u;
            float v;
         } tex;
      };

      auto model = gfx.shape.GetShapeData<Vertex>();
      for (int i = 0; i < static_cast<int>(Shape::ShapeCount); i++)
      {
         Shape::shapeType type = static_cast<Shape::shapeType>(i);
//         Shape::shapeType type = Shape::Plane;
         UINT start = gfx.shape.getVerticesStart(type);
         UINT count = gfx.shape.getVerticesCount(type);
         if ((type != Shape::Cone) && (type != Shape::Prism) && (type != Shape::Cylinder) && (type != Shape::Sphere))
         {
            assert((count % 4) == 0);
            for (unsigned int j = start; j < start + count; j += 4)
            {
               model.vertices[(size_t)(j + 0)].tex = { 0.0f,0.0f };
               model.vertices[(size_t)(j + 1)].tex = { 1.0f,0.0f };
               model.vertices[(size_t)(j + 2)].tex = { 0.0f,1.0f };
               model.vertices[(size_t)(j + 3)].tex = { 1.0f,1.0f };
            }
         }
      }

      std::unique_ptr < ObjectX11 > object = std::make_unique<ObjectX11>(gfx);
      object->AddTexture(Surface::FromFile("..\\..\\DirectX12Charles\\Images\\kappa50.png"));
      object->AddVertexBuffer(model.vertices);
      object->AddSampler();
      object->AddIndexBuffer(model.indices);

      object->AddShaders(L"TextureVSX11.cso", L"TexturePSX11.cso");

      // Layout
      const std::vector<D3D11_INPUT_ELEMENT_DESC> ied =
      {
          { "Position", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
          { "TexCoord", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
      };
      object->AddInputLayout(ied);

      addStaticBind(std::move(object), (UINT)model.indices.size());
   }

   std::unique_ptr < TransformX11 > trans = std::make_unique<TransformX11>(gfx, *this);
   trans->AddTransformConstantBuffer();
   UINT start = gfx.shape.getIndiceStart(type);
   UINT count = gfx.shape.getIndiceCount(type);
   trans->setIndices(start, count);

   AddBind(std::move(trans));
}

void ShapeTextureX11::Update(float dt) noexcept
{
   boxRoll += boxRollRate * dt;
   boxPitch += boxPitchRate * dt;
   boxYaw += boxYawRate * dt;
   spaceRoll += spaceRollRate * dt;
   spacePitch += spacePitchRate * dt;
   spaceYaw += spaceYawRate * dt;
}

XMMATRIX ShapeTextureX11::GetTransformXM() const noexcept
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
