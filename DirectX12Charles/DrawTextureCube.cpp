#include "DrawTextureCube.h"
using namespace std;

DrawTextureCube::DrawTextureCube(Graphics &gfx, int &index, float range)
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
   Shape::shapeType type = Shape::PictureCube;

   UINT verticesStart = gfx.m_shape.getVerticesStart(type);
   UINT verticesCount = gfx.m_shape.getVerticesCount(type);
   UINT indicesStart = gfx.m_shape.getIndiceStart(type);
   UINT indicesCount = gfx.m_shape.getIndiceCount(type);

   struct Vertex
   {
      XMFLOAT3 pos;
      XMFLOAT2 tex;
   };

   std::shared_ptr<Object> object = Object::Resolve(gfx, "TextureCube");
   if (!object->isInitialized())
   {
      object->setInitialized();

      // Define the vertex input layout.
      using hw3dexp::VertexLayout;
      hw3dexp::VertexBuffer vbuf(std::move(
         VertexLayout{}
         .Append(VertexLayout::Position3D)
         .Append(VertexLayout::Texture2D)
      ));

      std::vector <unsigned short> indices(indicesCount);

      auto model = gfx.m_shape.getShapeTextureData<Vertex>();
      for (int i = 0; i < model.m_vertices.size(); i++)
      {
         vbuf.EmplaceBack(
            *reinterpret_cast<XMFLOAT3 *>(&model.m_vertices[i].pos),
            *reinterpret_cast<XMFLOAT2 *>(&model.m_vertices[i].tex));
      }

      object->LoadVerticesBuffer(vbuf);
      object->LoadIndicesBuffer(model.m_indices);
      object->CreateShader(L"TextureVS.cso", L"TexturePS.cso");

      // Create Root Signature after constants
      object->CreateRootSignature(false, false, true);
      object->CreatePipelineState(vbuf.GetLayout().GetD3DLayout(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
   }
   AddBind(std::move(object));

   std::shared_ptr<Texture> texture = Texture::Resolve(gfx, "cube.png");
   if (!texture->isInitialized())
   {
      texture->setInitialized();
      std::string filename = "..\\..\\DirectX12Charles\\Images\\cube.png";
      texture->CreateTexture(filename, 0, 1);
   }
   AddBind(std::move(texture));

   std::shared_ptr < Transform > trans = std::make_shared<Transform>(gfx, *this, 0, -1);
   UINT start = gfx.m_shape.getIndiceStart(type);
   UINT count = gfx.m_shape.getIndiceCount(type);
   trans->setIndices(index, start, count);
   ++index;

   AddBind(std::move(trans));
}

void DrawTextureCube::Update(float dt) noexcept
{
   boxRoll += boxRollRate * dt;
   boxPitch += boxPitchRate * dt;
   boxYaw += boxYawRate * dt;
   spaceRoll += spaceRollRate * dt;
   spacePitch += spacePitchRate * dt;
   spaceYaw += spaceYawRate * dt;
}

XMMATRIX DrawTextureCube::GetTransformXM() const noexcept
{
   return DirectX::XMMatrixRotationRollPitchYaw(boxPitch, boxYaw, boxRoll) *
      DirectX::XMMatrixTranslation(range, 0.0f, 0.0f) *
      DirectX::XMMatrixRotationRollPitchYaw(spacePitch, spaceYaw, spaceRoll);
}
