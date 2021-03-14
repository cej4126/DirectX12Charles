#include "DrawLighted.h"
#include "imgui/imgui.h"

using namespace std;

//#define FIX_ROTATION

DrawLighted::DrawLighted(Graphics &gfx, int &index, Shape::shapeType type, float range, ID3D12Resource *mylightView, int &MaterialIndex)
   :
   gfx(gfx),
   range(range),
   m_materialIndex(MaterialIndex)
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

   if (MaterialIndex < 7)
   {
      int i = MaterialIndex + 1;
      float b = (float)(i & 1);
      float g = (float)((i >> 1) & 1);
      float r = (float)((i >> 2) & 1);
      material.materialColor = XMFLOAT3(r, g, b);
   }
   else
   {
      material.materialColor = XMFLOAT3(randcolor(gen), randcolor(gen), randcolor(gen));
   }
   ++MaterialIndex;

#ifdef FIX_ROTATION
   boxRoll = 0.0f * 3.1415f;
   boxPitch = 0.0 * 3.1415f;
   boxYaw = 0.0f * 3.1415f;
   boxRollRate = 0.0f;
   boxPitchRate = 20.0f / 180.0f * 3.1415f;
   boxYawRate = 0.0f / 180.0f * 3.1415f;

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

   std::string tag = "Lighted";
   std::shared_ptr<Object> object = Object::Resolve(gfx, tag);

   if (!object->isInitialized())
   {
      object->setInitialized();

      using hw3dexp::VertexLayout;
      hw3dexp::VertexBuffer vbuf(std::move(
         VertexLayout{}
         .Append(VertexLayout::Position3D)
         .Append(VertexLayout::Normal)
      ));
      auto model = gfx.shape.GetShapeNormalData<Vertex>();
      for (unsigned int i = 0; i < model.vertices.size(); i++)
      {
         vbuf.EmplaceBack(
            *reinterpret_cast<XMFLOAT3 *>(&model.vertices[i].pos),
            *reinterpret_cast<XMFLOAT3 *>(&model.vertices[i].normal));
      }

      object->LoadVerticesBuffer(vbuf);
      object->LoadIndicesBuffer(model.indices);
      object->CreateShader(L"LightedVS.cso", L"LightedPS.cso");

      // Create Root Signature after constants
      object->CreateRootSignature(true, true, false);

      object->CreatePipelineState(vbuf.GetLayout().GetD3DLayout(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

      object->SetLightView(mylightView);
   }

   AddBind(std::move(object));

   std::shared_ptr < Transform > trans = std::make_shared<Transform>(gfx, *this, 0, -1);

   UINT start = gfx.shape.getIndiceStart(type);
   UINT count = gfx.shape.getIndiceCount(type);
   trans->setIndices(index, start, count);
   ++index;

   AddBind(std::move(trans));
}

void DrawLighted::Update(float dt) noexcept
{
   float pi_2 = (float)(2.0 * M_PI);
   boxRoll += boxRollRate * dt;
   boxRoll = remainder(boxRoll, pi_2);

   boxPitch += boxPitchRate * dt;
   boxPitch = remainder(boxPitch, pi_2);

   boxYaw += boxYawRate * dt;
   boxYaw = remainder(boxYaw, pi_2);

   spaceRoll += spaceRollRate * dt;
   spaceRoll = remainder(spaceRoll, pi_2);

   spacePitch += spacePitchRate * dt;
   spacePitch = remainder(spacePitch, pi_2);

   spaceYaw += spaceYawRate * dt;
   spaceYaw = remainder(spaceYaw, pi_2);
}

XMMATRIX DrawLighted::GetTransformXM() const noexcept
{
   return DirectX::XMMatrixRotationRollPitchYaw(boxPitch, boxYaw, boxRoll) *
      DirectX::XMMatrixTranslation(range, 0.0f, 0.0f) *
      DirectX::XMMatrixRotationRollPitchYaw(spacePitch, spaceYaw, spaceRoll);
}

void DrawLighted::getMaterialData(Graphics::MaterialType &myMaterial) const noexcept
{
   myMaterial.materialColor = material.materialColor;
}

void DrawLighted::SpawnControlWindow() noexcept
{
   using namespace std::string_literals;

   ImGui::Text("Material Properties");
   bool mChanged = ImGui::ColorEdit3("Material Color", &material.materialColor.x);
   bool iChanged = ImGui::SliderFloat("Specular Intensity", &material.specularInensity, 0.05f, 4.0f, "%.2f");
   bool pChanged = ImGui::SliderFloat("Specular Power", &material.specularPower, 1.0f, 200.0f, "%.2f");


   ImGui::Text("Position");
   ImGui::SliderFloat("R", &range, 5.0f, 40.0f, "%f");
   ImGui::SliderAngle("Box Roll Rate", &boxRollRate, -180.0f, 180.0f);
   ImGui::SliderAngle("Box Pitch Rate", &boxPitchRate, -180.0f, 180.0f);
   ImGui::SliderAngle("Box Yaw Rate", &boxYawRate, -180.0f, 180.0f);
   ImGui::Text("Orientation");
   ImGui::SliderAngle("Space Roll Rate", &spaceRollRate, -180.0f, 180.0f);
   ImGui::SliderAngle("Space Pitch Rate", &spacePitchRate, -180.0f, 180.0f);
   ImGui::SliderAngle("Space Yaw Rate", &spaceYawRate, -180.0f, 180.0f);

   if (mChanged || iChanged || pChanged)
   {
      SyncMaterial();
   }
}

void DrawLighted::SyncMaterial() noexcept
{
   gfx.CopyMaterialConstant(m_materialIndex, material);
}
