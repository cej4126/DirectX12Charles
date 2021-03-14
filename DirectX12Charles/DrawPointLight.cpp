#include "DrawPointLight.h"
#include "imgui/imgui.h"

using namespace std;

DrawPointLight::DrawPointLight(Graphics &gfx, int &index, float size)
   :
   size(size),
   gfx(gfx)
{
   ResetLightData();
   Shape::shapeType type = Shape::Sphere;

   UINT verticesStart = gfx.shape.getVerticesStart(type);
   UINT verticesCount = gfx.shape.getVerticesCount(type);
   UINT indicesStart = gfx.shape.getIndiceStart(type);
   UINT indicesCount = gfx.shape.getIndiceCount(type);

   std::shared_ptr<Object> object = Object::Resolve(gfx, "PointLight");
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

      auto model = gfx.shape.GetShapeData<Vertex>();

      std::vector< Vertex > vertices(verticesCount);

      FXMMATRIX matrix = XMMatrixScaling(size, size, size);
      for (UINT i = 0; i < verticesCount; i++)
      {
         int index = verticesStart + i;
         vertices[i] = model.vertices[index];

         XMVECTOR pos = DirectX::XMLoadFloat3(&vertices[i].pos);
         XMStoreFloat3(
            &vertices[i].pos,
            XMVector3Transform(pos, matrix)
         );

         vbuf.EmplaceBack(
            *reinterpret_cast<XMFLOAT3 *>(&vertices[i].pos));
      }

      std::vector <unsigned short> indices(indicesCount);
      for (UINT i = 0; i < indicesCount; i++)
      {
         int index = indicesStart + i;
         indices[i] = model.indices[index] - verticesStart;
      }

      object->LoadVerticesBuffer(vbuf);
      object->LoadIndicesBuffer(indices);
      object->CreateShader(L"PointLightVS.cso", L"PointLightPS.cso");

      struct PSColorConstant
      {
         XMFLOAT3 color = { 1.0f, 1.0f, 1.0f };
         float padding;
      } colorConst;
      object->CreateConstant((const XMFLOAT3 &)colorConst, sizeof(colorConst));

      // Create Root Signature after constants
      object->CreateRootSignature(true, false, false);

      object->CreatePipelineState(vbuf.GetLayout().GetD3DLayout(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
   }
   AddBind(std::move(object));

   std::shared_ptr < Transform > trans = std::make_shared<Transform>(gfx, *this, 0, -1);
   trans->setIndices(index, 0, indicesCount);
   ++index;
   lightView = trans->CreateLightPosition(gfx.lightData);
   AddBind(std::move(trans));
}

void DrawPointLight::CreateLightControl() noexcept
{
   if (ImGui::Begin("Light"))
   {
      ImGui::Text("Position");
      ImGui::SliderFloat("X", &gfx.lightData.position.x, -30.0f, 30.0f, "%0.1f");
      ImGui::SliderFloat("Y", &gfx.lightData.position.y, -30.0f, 30.0f, "%0.1f");
      ImGui::SliderFloat("Z", &gfx.lightData.position.z, -30.0f, 30.0f, "%0.1f");

      ImGui::Text("Intensity/Color");
      //ImGui::SliderFloat("Intensity", &gfx.lightData.diffuseIntensity, 0.01f, 2.0f, "%.2f", 2);
      ImGui::SliderFloat("Intensity", &gfx.lightData.diffuseIntensity, 0.01f, 5.0f, "%0.2f");
      ImGui::ColorEdit3("Diffuse Color", &gfx.lightData.diffuseColor.x);
      ImGui::ColorEdit3("Ambient", &gfx.lightData.ambient.x);

      ImGui::Text("Falloff");
      //ImGui::SliderFloat("Constant", &gfx.lightData.attConst, 0.05f, 10.0f, "%.2f", 4);
      //ImGui::SliderFloat("Linear", &gfx.lightData.attLin, 0.0001f, 4.0f, "%.4f", 8);
      //ImGui::SliderFloat("Quadratic", &gfx.lightData.attQuad, 0.0000001f, 10.0f, "%.7f", 10);
      ImGui::SliderFloat("Constant", &gfx.lightData.attConst, 0.05f, 1.0f, "%.2f");
      ImGui::SliderFloat("Linear", &gfx.lightData.attLin, 0.0001f, 0.1f, "%.4f");
      ImGui::SliderFloat("Quadratic", &gfx.lightData.attQuad, 0.0000001f, 0.1f, "%.7f");

      if (ImGui::Button("Reset"))
      {
         ResetLightData();
      }
   }
   ImGui::End();
}

void DrawPointLight::ResetLightData() noexcept
{
   gfx.lightData =
   {
      XMFLOAT3(1.5f, 14.0f, -4.5f), 0.0f,
      XMFLOAT3(0.05f, 0.05f, 0.05f), 0.0f,
      XMFLOAT3(1.0f, 1.0f, 1.0f), 0.0f,
      1.0f,
      1.0f,
      0.045f,
      0.0075f
   };
}

void DrawPointLight::Update(float dt) noexcept
{
}

XMMATRIX DrawPointLight::GetTransformXM() const noexcept
{
   return XMMatrixTranslation(gfx.lightData.position.x, gfx.lightData.position.y, gfx.lightData.position.z);
}
