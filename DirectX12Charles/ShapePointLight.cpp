#include "ShapePointLight.h"
#include "imgui.h"

using namespace std;

ShapePointLight::ShapePointLight(Graphics &gfx, float size)
   :
   size(size)
{
   Shape::shapeType type = Shape::Sphere;

   UINT verticesStart = gfx.shape.getVerticesStart(type);
   UINT verticesCount = gfx.shape.getVerticesCount(type);
   UINT indicesStart = gfx.shape.getIndiceStart(type);
   UINT indicesCount = gfx.shape.getIndiceCount(type);

   if (!isStaticSet())
   {
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

         const XMVECTOR pos = DirectX::XMLoadFloat3(&vertices[i].pos);
         XMStoreFloat3(
            &vertices[i].pos,
            XMVector3Transform(pos, matrix)
         );
      }

      std::unique_ptr<Object> object = std::make_unique< Object>(gfx);

      std::vector <unsigned short> indices(indicesCount);
      for (UINT i = 0; i < indicesCount; i++)
      {
         int index = indicesStart + i;
         indices[i] = model.indices[index] - verticesStart;
      }

      object->CreateRootSignature(true, false);
      object->LoadVerticesBuffer(vertices);
      object->LoadIndicesBuffer(indices);
      object->CreateShader(L"PointLightVS.cso", L"PointLightPS.cso");
      // Define the vertex input layout.
      const std::vector < D3D12_INPUT_ELEMENT_DESC> inputElementDescs =
      {
          { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
      };
      struct PSColorConstant
      {
         XMFLOAT3 color = { 1.0f, 1.0f, 1.0f };
         float padding;
      } colorConst;
      object->CreateConstant(colorConst);

      object->CreatePipelineState(inputElementDescs, D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

      addStaticBind(std::move(object), (UINT)model.indices.size());
   }

   std::unique_ptr < Transform > trans = std::make_unique<Transform>(gfx, *this);
   trans->setIndices(0, indicesCount);
   lightView = trans->CreateLightPosition(position);
   AddBind(std::move(trans));
}

void ShapePointLight::CreateLightControl() noexcept
{
   if (ImGui::Begin("Light"))
   {
      ImGui::Text("Position");
      ImGui::SliderFloat("X", &position.x, -30.0f, 30.0f, "%.1f");
      ImGui::SliderFloat("Y", &position.y, -30.0f, 30.0f, "%.1f");
      ImGui::SliderFloat("Z", &position.z, -30.0f, 30.0f, "%.1f");
      if (ImGui::Button("Reset"))
      {
         Reset();
      }
   }
   ImGui::End();
}

void ShapePointLight::Reset() noexcept
{
   position = { 0.0f,0.0f,0.0f };
}

void ShapePointLight::Update(float dt) noexcept
{
}

void ShapePointLight::SetPosition(XMFLOAT3 pos) noexcept
{
   position = pos;
}

XMMATRIX ShapePointLight::GetTransformXM() const noexcept
{
   return XMMatrixTranslation(position.x, position.y, position.z);
}
