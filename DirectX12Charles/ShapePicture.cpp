#include "ShapePicture.h"
using namespace std;

ShapePicture::ShapePicture(Graphics &gfx, float range)
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
   Shape::shapeType type = Shape::Plane;

   UINT verticesStart = gfx.shape.getVerticesStart(type);
   UINT verticesCount = gfx.shape.getVerticesCount(type);
   UINT indicesStart = gfx.shape.getIndiceStart(type);
   UINT indicesCount = gfx.shape.getIndiceCount(type);

   if (!isStaticSet())
   {
      std::unique_ptr<ObjectX12> object = std::make_unique< ObjectX12>(gfx);

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

      std::vector< Vertex > vertices(verticesCount);
      for (UINT i = 0; i < verticesCount; i++)
      {
         int index = verticesStart + i;
         vertices[i] = model.vertices[index];
      }
      vertices[0].tex = { 0.0f,0.0f };
      vertices[1].tex = { 1.0f,0.0f };
      vertices[2].tex = { 0.0f,1.0f };
      vertices[3].tex = { 1.0f,1.0f };
      std::vector <unsigned short> indices(indicesCount);
      for (UINT i = 0; i < indicesCount; i++)
      {
         int index = indicesStart + i;
         indices[i] = model.indices[index] - verticesStart;
      }

      object->CreateTexture(Surface::FromFile("..\\..\\DirectX12Charles\\Images\\picture3.jpg"));

      object->CreateRootSignature(false, true);
      object->LoadVerticesBuffer(vertices);
      object->LoadIndicesBuffer(indices);
      object->CreateShader(L"TextureVS.cso", L"TexturePS.cso");
      // Define the vertex input layout.
      const std::vector < D3D12_INPUT_ELEMENT_DESC> inputElementDescs =
      {
          { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
          { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
      };

      object->CreatePipelineState(inputElementDescs, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

      addStaticBind(std::move(object), (UINT)model.indices.size());
   }

   std::unique_ptr < TransformX12 > trans = std::make_unique<TransformX12>(gfx, *this);
   trans->setIndices(0, indicesCount);

   AddBind(std::move(trans));
}

void ShapePicture::Update(float dt) noexcept
{
   boxRoll += boxRollRate * dt;
   boxPitch += boxPitchRate * dt;
   boxYaw += boxYawRate * dt;
   spaceRoll += spaceRollRate * dt;
   spacePitch += spacePitchRate * dt;
   spaceYaw += spaceYawRate * dt;
}

XMMATRIX ShapePicture::GetTransformXM() const noexcept
{
   return DirectX::XMMatrixRotationRollPitchYaw(boxPitch, boxYaw, boxRoll) *
      DirectX::XMMatrixTranslation(range, 0.0f, 0.0f) *
      DirectX::XMMatrixRotationRollPitchYaw(spacePitch, spaceYaw, spaceRoll);
}
