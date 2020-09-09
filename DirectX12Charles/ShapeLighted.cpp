#include "ShapeLighted.h"
using namespace std;

ShapeLighted::ShapeLighted(Graphics &gfx, float range, ID3D12Resource *mylightView, int MaterialIndex)
   :
   range(range),
   MaterialIndex(MaterialIndex)
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

   material.materialColor = XMFLOAT3(randcolor(gen), randcolor(gen), randcolor(gen));

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
      auto model = gfx.shape.GetShapeData<Vertex>();

      std::vector< Vertex > vertices(verticesCount);
      for (UINT i = 0; i < verticesCount; i++)
      {
         int index = verticesStart + i;
         vertices[i] = model.vertices[index];
      }

      std::vector <unsigned short> indices(indicesCount);
      for (UINT i = 0; i < indicesCount; i++)
      {
         int index = indicesStart + i;
         indices[i] = model.indices[index] - verticesStart;
      }
      Scale(vertices, 0.5f, 0.5f, 1.0f);
      SetNormals(indices, vertices);

      std::unique_ptr<Object> object = std::make_unique< Object>(gfx);

      object->LoadVerticesBuffer(vertices);
      object->LoadIndicesBuffer(indices);
      object->CreateShader(L"LightedVS.cso", L"LightedPS.cso");

      // Define the vertex input layout.
      const std::vector < D3D12_INPUT_ELEMENT_DESC> inputElementDescs =
      {
          { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
          { "Normal", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
      };

      XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
      object->CreateConstant(position);

      // Create Root Signature after constants
      object->CreateRootSignature(true);

      object->CreatePipelineState(inputElementDescs, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

      object->SetLightView(mylightView);

      addStaticBind(std::move(object), (UINT)model.indices.size());
   }

   std::unique_ptr < Transform > trans = std::make_unique<Transform>(gfx, *this);
   trans->setIndices(0, indicesCount);

   AddBind(std::move(trans));
}

void ShapeLighted::Scale(std::vector< Vertex > &vertices, float x, float y, float z)
{
   FXMMATRIX matrix = XMMatrixScaling(x, y, z);
   for (UINT i = 0; i < vertices.size(); i++)
   {
      const XMVECTOR pos = DirectX::XMLoadFloat3(&vertices[i].pos);
      XMStoreFloat3(
         &vertices[i].pos,
         XMVector3Transform(pos, matrix)
      );
   }
}

void ShapeLighted::SetNormals(std::vector <unsigned short> &indices, std::vector< Vertex > &vertices) noexcept
{
   using namespace DirectX;
   assert(indices.size() % 3 == 0 && indices.size() > 0);
   for (size_t i = 0; i < indices.size(); i += 3)
   {
      auto &v0 = vertices[indices[i]];
      auto &v1 = vertices[indices[i + 1]];
      auto &v2 = vertices[indices[i + 2]];
      const auto p0 = XMLoadFloat3(&v0.pos);
      const auto p1 = XMLoadFloat3(&v1.pos);
      const auto p2 = XMLoadFloat3(&v2.pos);

      const auto n = XMVector3Normalize(XMVector3Cross((p1 - p0), (p2 - p0)));

      XMStoreFloat3(&v0.n, n);
      XMStoreFloat3(&v1.n, n);
      XMStoreFloat3(&v2.n, n);
   }
}

void ShapeLighted::Update(float dt) noexcept
{
   boxRoll += boxRollRate * dt;
   boxPitch += boxPitchRate * dt;
   boxYaw += boxYawRate * dt;
   spaceRoll += spaceRollRate * dt;
   spacePitch += spacePitchRate * dt;
   spaceYaw += spaceYawRate * dt;
}

XMMATRIX ShapeLighted::GetTransformXM() const noexcept
{
   return DirectX::XMMatrixRotationRollPitchYaw(boxPitch, boxYaw, boxRoll) *
      DirectX::XMMatrixTranslation(range, 0.0f, 0.0f) *
      DirectX::XMMatrixRotationRollPitchYaw(spacePitch, spaceYaw, spaceRoll);
}
