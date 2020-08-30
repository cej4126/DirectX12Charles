#include "ShapeTextureCube.h"
using namespace std;

ShapeTextureCube::ShapeTextureCube(Graphics &gfx, float range)
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
   Shape::shapeType type = Shape::TextureCube;

   UINT verticesStart = gfx.shape.getVerticesStart(type);
   UINT verticesCount = gfx.shape.getVerticesCount(type);
   UINT indicesStart = gfx.shape.getIndiceStart(type);
   UINT indicesCount = gfx.shape.getIndiceCount(type);

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

      std::vector< Vertex > vertices(verticesCount);
      for (UINT i = 0; i < verticesCount; i++)
      {
         int index = verticesStart + i;
         vertices[i] = model.vertices[index];
      }

      const float u1 = 0.0f;
      const float u2 = 1.0f / 3.0f;
      const float u3 = 2.0f / 3.0f;
      const float u4 = 1.0f;
      const float v1 = 0.0f;
      const float v2 = 1.0f / 4.0f;
      const float v3 = 2.0f / 4.0f;
      const float v4 = 3.0f / 4.0f;
      const float v5 = 1.0f;
      // Y
      // |
      // Z - X
      // Front
      vertices[0].tex = { u3, v2 };
      vertices[1].tex = { u3, v3 };
      vertices[2].tex = { u2, v3 };
      vertices[3].tex = { u2, v2 };

      // Top
      vertices[4].tex = { u4, v2 };
      vertices[5].tex = { u4, v3 };
      vertices[6].tex = { u3, v3 };
      vertices[7].tex = { u3, v2 };

      // Back
      vertices[8].tex = { u3, v4 };
      vertices[9].tex = { u3, v5 };
      vertices[10].tex = { u2, v5 };
      vertices[11].tex = { u2, v4 };

      // Bottom
      vertices[12].tex = { u2, v2 };
      vertices[13].tex = { u2, v3 };
      vertices[14].tex = { u1, v3 };
      vertices[15].tex = { u1, v2 };

      // Right
      vertices[16].tex = { u3, v3 };
      vertices[17].tex = { u3, v4 };
      vertices[18].tex = { u2, v4 };
      vertices[19].tex = { u2, v3 };

      // Left
      vertices[20].tex = { u3, v1 };
      vertices[21].tex = { u3, v2 };
      vertices[22].tex = { u2, v2 };
      vertices[23].tex = { u2, v1 };

      std::unique_ptr<Object> object = std::make_unique< Object>(gfx);

      std::vector <unsigned short> indices(indicesCount);
      for (UINT i = 0; i < indicesCount; i++)
      {
         int index = indicesStart + i;
         indices[i] = model.indices[index] - verticesStart;
      }

      object->CreateTexture(Surface::FromFile("..\\..\\DirectX12Charles\\Images\\cube.png"));

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

   std::unique_ptr < Transform > trans = std::make_unique<Transform>(gfx, *this);
   trans->setIndices(0, indicesCount);

   AddBind(std::move(trans));
}

void ShapeTextureCube::Update(float dt) noexcept
{
   boxRoll += boxRollRate * dt;
   boxPitch += boxPitchRate * dt;
   boxYaw += boxYawRate * dt;
   spaceRoll += spaceRollRate * dt;
   spacePitch += spacePitchRate * dt;
   spaceYaw += spaceYawRate * dt;
}

XMMATRIX ShapeTextureCube::GetTransformXM() const noexcept
{
   return DirectX::XMMatrixRotationRollPitchYaw(boxPitch, boxYaw, boxRoll) *
      DirectX::XMMatrixTranslation(range, 0.0f, 0.0f) *
      DirectX::XMMatrixRotationRollPitchYaw(spacePitch, spaceYaw, spaceRoll);
}
