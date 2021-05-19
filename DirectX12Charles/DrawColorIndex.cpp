#include "DrawColorIndex.h"
using namespace std;

//#define FIX_ROTATION

DrawColorIndex::DrawColorIndex(Graphics &gfx, int &index, Shape::shapeType type, float range)
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
   boxPitch = 0.0f * 3.1415f;
   boxYaw = 0.2f * 3.1415f;
   boxRollRate = 0.0f;
   boxPitchRate = 20.0f / 180.0f * 3.1415f;
   boxYawRate = -60.0f / 180.0f * 3.1415f;

   spaceRoll = 0.0f;
   spacePitch = 0.0f;
   spaceYaw = 0.0f;
   spaceRollRate = 0.0f;
   spacePitchRate = 0.0f;
   spaceYawRate = 0.0f;
#endif

   std::shared_ptr<Object> object = Object::Resolve(gfx, "ColorIndex");

   if (!object->isInitialized())
   {
      object->setInitialized();


      using hw3dexp::VertexLayout;
      hw3dexp::VertexBuffer vbuf(std::move(
         VertexLayout{}
         .Append(VertexLayout::Position3D)
      ));

      struct Vertex
      {
         XMFLOAT3 pos;
      };
      auto model = gfx.m_shape.getShapeData<Vertex>();

      for (int i = 0; i < model.m_vertices.size(); i++)
      {
         vbuf.EmplaceBack(
            *reinterpret_cast<XMFLOAT3 *>(&model.m_vertices[i].pos));
      }

      object->LoadVerticesBuffer(vbuf);
      object->LoadIndicesBuffer(model.m_indices);
      object->CreateShader(L"ColorIndexVS.cso", L"ColorIndexPS.cso");

      struct ConstantBufferColor
      {
         struct
         {
            float r;
            float g;
            float b;
            float a;
         } face_colors[6];
      };
      struct ConstantBufferColor colorBuffer;

      ConstantBufferColor cb =
      {
         {
            {0.0f, 0.0f, 1.0f, 1.0f},
            {0.0f, 1.0f, 0.0f, 1.0f},
            {0.0f, 1.0f, 1.0f, 1.0f},
            {1.0f, 0.0f, 0.0f, 1.0f},
            {1.0f, 0.0f, 1.0f, 1.0f},
            {1.0f, 1.0f, 0.0f, 1.0f},
         }
      };
      //colorBuffer = cb;
      for (int i = 0; i < 6; i++)
      {
         colorBuffer.face_colors[i].r = cb.face_colors[i].r;
         colorBuffer.face_colors[i].g = cb.face_colors[i].g;
         colorBuffer.face_colors[i].b = cb.face_colors[i].b;
         colorBuffer.face_colors[i].a = cb.face_colors[i].a;
      }

      object->CreateConstant((const XMFLOAT3 &)colorBuffer, sizeof(colorBuffer));

      // Create Root Signature after constants
      object->CreateRootSignature(true, false, false);

      object->CreatePipelineState(vbuf.GetLayout().GetD3DLayout(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
      // lookup table for cube face colors
   }

   AddBind(std::move(object));

   std::shared_ptr < Transform > trans = std::make_shared<Transform>(gfx, *this, 0, -1);
   UINT start = gfx.m_shape.getIndiceStart(type);
   UINT count = gfx.m_shape.getIndiceCount(type);
   trans->setIndices(index, start, count);
   ++index;

   AddBind(std::move(trans));
}

void DrawColorIndex::Update(float dt) noexcept
{
   boxRoll += boxRollRate * dt;
   boxPitch += boxPitchRate * dt;
   boxYaw += boxYawRate * dt;
   spaceRoll += spaceRollRate * dt;
   spacePitch += spacePitchRate * dt;
   spaceYaw += spaceYawRate * dt;
}

XMMATRIX DrawColorIndex::GetTransformXM() const noexcept
{
   return DirectX::XMMatrixRotationRollPitchYaw(boxPitch, boxYaw, boxRoll) *
      DirectX::XMMatrixTranslation(range, 0.0f, 0.0f) *
      DirectX::XMMatrixRotationRollPitchYaw(spacePitch, spaceYaw, spaceRoll);
}
