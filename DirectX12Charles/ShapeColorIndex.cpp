#include "ShapeColorIndex.h"
using namespace std;

//#define FIX_ROTATION

ShapeColorIndex::ShapeColorIndex(Graphics &gfx, Shape::shapeType type, float range)
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

      std::shared_ptr<Object> object = std::make_shared< Object>(gfx);

      struct Vertex
      {
         XMFLOAT3 pos;
      };
      auto model = gfx.shape.GetShapeData<Vertex>();

      object->LoadVerticesBuffer(model.vertices);
      object->LoadIndicesBuffer(model.indices);
      object->CreateShader(L"ColorIndexVS.cso", L"ColorIndexPS.cso");

      // Define the vertex input layout.
      const std::vector < D3D12_INPUT_ELEMENT_DESC> inputElementDescs =
      {
          { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
      };

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

      object->CreateConstant(colorBuffer);

      // Create Root Signature after constants
      object->CreateRootSignature(false, false);

      object->CreatePipelineState(inputElementDescs, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
      // lookup table for cube face colors


      AddBind(std::move(object));

   std::shared_ptr < Transform > trans = std::make_shared<Transform>(gfx, *this);
   UINT start = gfx.shape.getIndiceStart(type);
   UINT count = gfx.shape.getIndiceCount(type);
   trans->setIndices(start, count);

   AddBind(std::move(trans));
}

void ShapeColorIndex::Update(float dt) noexcept
{
   boxRoll += boxRollRate * dt;
   boxPitch += boxPitchRate * dt;
   boxYaw += boxYawRate * dt;
   spaceRoll += spaceRollRate * dt;
   spacePitch += spacePitchRate * dt;
   spaceYaw += spaceYawRate * dt;
}

XMMATRIX ShapeColorIndex::GetTransformXM() const noexcept
{
   return DirectX::XMMatrixRotationRollPitchYaw(boxPitch, boxYaw, boxRoll) *
      DirectX::XMMatrixTranslation(range, 0.0f, 0.0f) *
      DirectX::XMMatrixRotationRollPitchYaw(spacePitch, spaceYaw, spaceRoll);
}
