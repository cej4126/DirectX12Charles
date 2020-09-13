#include "ShapeColorBlended.h"
using namespace std;

ShapeColorBlended::ShapeColorBlended(Graphics &gfx, Shape::shapeType type, float range)
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
      std::unique_ptr<Object> object = std::make_unique< Object>(gfx);

      struct Vertex
      {
         XMFLOAT3 pos;
         XMFLOAT4 color;
      };

      auto model = gfx.shape.GetShapeData<Vertex>();

      for (int i = 0; i < model.vertices.size(); i++)
      {
         float r = randcolor(gen);
         float b = randcolor(gen);
         float g = randcolor(gen);
         model.vertices[i].color = { r, b, g, 1.0f };
      }

      object->LoadVerticesBuffer(model.vertices);
      object->LoadIndicesBuffer(model.indices);
      object->CreateShader(L"ColorBlendedVS.cso", L"ColorBlendedPS.cso");
      // Define the vertex input layout.
      const std::vector < D3D12_INPUT_ELEMENT_DESC> inputElementDescs =
      {
          { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
          { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
      };

      // Create Root Signature after constants
      object->CreateRootSignature(false);

      object->CreatePipelineState(inputElementDescs, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

      addStaticBind(std::move(object), (UINT)model.indices.size());
   }

   std::unique_ptr < Transform > trans = std::make_unique<Transform>(gfx, *this);
   UINT start = gfx.shape.getIndiceStart(type);
   UINT count = gfx.shape.getIndiceCount(type);
   trans->setIndices(start, count);

   AddBind(std::move(trans));
}

void ShapeColorBlended::Update(float dt) noexcept
{
   boxRoll += boxRollRate * dt;
   boxPitch += boxPitchRate * dt;
   boxYaw += boxYawRate * dt;
   spaceRoll += spaceRollRate * dt;
   spacePitch += spacePitchRate * dt;
   spaceYaw += spaceYawRate * dt;
}

XMMATRIX ShapeColorBlended::GetTransformXM() const noexcept
{
   return DirectX::XMMatrixRotationRollPitchYaw(boxPitch, boxYaw, boxRoll) *
      DirectX::XMMatrixTranslation(range, 0.0f, 0.0f) *
      DirectX::XMMatrixRotationRollPitchYaw(spacePitch, spaceYaw, spaceRoll);
}
