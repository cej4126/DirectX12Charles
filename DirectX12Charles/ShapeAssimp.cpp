#include "ShapeAssimp.h"
#include "imgui/imgui.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

using namespace std;

//#define FIX_ROTATION

ShapeAssimp::ShapeAssimp(Graphics &gfx, Shape::shapeType type, float range, ID3D12Resource *mylightView, int MaterialIndex)
   :
   gfx(gfx),
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

#ifdef FIX_ROTATION
   boxRoll = 0.0f * 3.1415f;
   boxPitch = 0.0 * 3.1415f;
   boxYaw = 0.0f * 3.1415f;
   boxRollRate = 0.0f;
   boxPitchRate = 0.5f;
   boxYawRate = 0.5f;

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

   if (!isStaticSet())
   {
      auto model = gfx.shape.GetShapeNormalData<Vertex>();

      std::unique_ptr<Object> object = std::make_unique< Object>(gfx);

      object->LoadVerticesBuffer(model.vertices);
      object->LoadIndicesBuffer(model.indices);
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
      object->CreateRootSignature(true, false);

      object->CreatePipelineState(inputElementDescs, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

      object->SetLightView(mylightView);

      addStaticBind(std::move(object), (UINT)model.indices.size());
   }

   std::unique_ptr < Transform > trans = std::make_unique<Transform>(gfx, *this);

   UINT start = gfx.shape.getIndiceStart(type);
   UINT count = gfx.shape.getIndiceCount(type);
   trans->setIndices(start, count);

   AddBind(std::move(trans));
}

void ShapeAssimp::Update(float dt) noexcept
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

XMMATRIX ShapeAssimp::GetTransformXM() const noexcept
{
   return DirectX::XMMatrixRotationRollPitchYaw(boxPitch, boxYaw, boxRoll) *
      DirectX::XMMatrixTranslation(range, 0.0f, 0.0f) *
      DirectX::XMMatrixRotationRollPitchYaw(spacePitch, spaceYaw, spaceRoll);
}

void ShapeAssimp::getMaterialData(Graphics::MaterialType &myMaterial) const noexcept
{
   myMaterial.materialColor = material.materialColor;
}

void ShapeAssimp::SpawnControlWindow() noexcept
{
   using namespace std::string_literals;

   ImGui::Text("Material Properties");
   bool mChanged = ImGui::ColorEdit3("Material Color", &material.materialColor.x);
   bool iChanged = ImGui::SliderFloat("Specular Intensity", &material.specularInensity, 0.05f, 4.0f, "%.2f");
   bool pChanged = ImGui::SliderFloat("Specular Power", &material.specularPower, 1.0f, 200.0f, "%.2f");

   ImGui::Text("Position");
   ImGui::SliderFloat("R", &range, 0.0f, 40.0f, "%f");
   ImGui::SliderAngle("Theta", &spacePitch, -180.0f, 180.0f);
   ImGui::SliderAngle("Phi", &spaceYaw, -180.0f, 180.0f);
   ImGui::Text("Orientation");
   ImGui::SliderAngle("Roll", &boxRoll, -180.0f, 180.0f);
   ImGui::SliderAngle("Pitch", &boxPitch, -180.0f, 180.0f);
   ImGui::SliderAngle("Yaw", &spaceYaw, -180.0f, 180.0f);

   if (mChanged || iChanged || pChanged)
   {
      SyncMaterial();
   }
}

void ShapeAssimp::SyncMaterial() noexcept
{
   gfx.CopyMaterialConstant(MaterialIndex, material);
}
