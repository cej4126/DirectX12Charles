#include "DrawPictureCube.h"
using namespace std;

//#define FIX_ROTATION
DrawPictureCube::DrawPictureCube(Graphics &gfx, int &index, Shape::shapeType type, float range, const std::string &filename)
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

   UINT verticesStart = gfx.shape.getVerticesStart(type);
   UINT verticesCount = gfx.shape.getVerticesCount(type);
   UINT indicesStart = gfx.shape.getIndiceStart(type);
   UINT indicesCount = gfx.shape.getIndiceCount(type);

   std::size_t pos = filename.find_last_of("/\\");
   std::string tag = "cube#" + filename.substr(pos + 1);

   std::shared_ptr<Object> object = Object::Resolve(gfx, tag);
   if (!object->isInitialized())
   {
      object->setInitialized();

      // Define the vertex input layout.
      //using hw3dexp::VertexLayout;
      hw3dexp::VertexBuffer vbuf(std::move(
         hw3dexp::VertexLayout{}
         .Append(hw3dexp::VertexLayout::Position3D)
         .Append(hw3dexp::VertexLayout::Texture2D)
      ));

      struct Vertex
      {
         XMFLOAT3 pos;
         XMFLOAT2 tex;
      };
      auto model = gfx.shape.GetShapeTextureData<Vertex>();

      for (int i = 0; i < model.vertices.size(); i++)
      {
         vbuf.EmplaceBack(
            *reinterpret_cast<XMFLOAT3 *>(&model.vertices[i].pos),
            *reinterpret_cast<XMFLOAT2 *>(&model.vertices[i].tex));
      }

      object->LoadVerticesBuffer(vbuf);
      object->LoadIndicesBuffer(model.indices);
      object->CreateShader(L"TextureVS.cso", L"TexturePS.cso");

      // Create Root Signature after constants
      object->CreateRootSignature(false, false, true);
      object->CreatePipelineState(vbuf.GetLayout().GetD3DLayout(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
   }
   AddBind(std::move(object));

   std::shared_ptr<Texture> texture = Texture::Resolve(gfx, tag);
   if (!texture->isInitialized())
   {
      texture->setInitialized();
      texture->CreateTexture(filename, 0, 1);
   }
   AddBind(std::move(texture));

   std::shared_ptr < Transform > trans = std::make_shared<Transform>(gfx, *this);
   UINT start = gfx.shape.getIndiceStart(type);
   UINT count = gfx.shape.getIndiceCount(type);
   trans->setIndices(index, start, count);
   ++index;

   AddBind(std::move(trans));
}

void DrawPictureCube::Update(float dt) noexcept
{
   boxRoll += boxRollRate * dt;
   boxPitch += boxPitchRate * dt;
   boxYaw += boxYawRate * dt;
   spaceRoll += spaceRollRate * dt;
   spacePitch += spacePitchRate * dt;
   spaceYaw += spaceYawRate * dt;
}

XMMATRIX DrawPictureCube::GetTransformXM() const noexcept
{
   return DirectX::XMMatrixRotationRollPitchYaw(boxPitch, boxYaw, boxRoll) *
      DirectX::XMMatrixTranslation(range, 0.0f, 0.0f) *
      DirectX::XMMatrixRotationRollPitchYaw(spacePitch, spaceYaw, spaceRoll);
}
