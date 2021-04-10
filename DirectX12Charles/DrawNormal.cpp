#include "DrawNormal.h"
#include "imgui/imgui.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

DrawNormal::DrawNormal(Graphics &gfx, int &index, Shape::shapeType type, float size,
   const std::string &texturefilename,
   const std::string &normalfilename,
   ID3D12Resource *lightView, int &MaterialIndex)
   :
   m_gfx(gfx),
   m_materialIndex(MaterialIndex)
{
   m_size = size;

   UINT verticesStart = gfx.shape.getVerticesStart(type);
   UINT verticesCount = gfx.shape.getVerticesCount(type);
   UINT indicesStart = gfx.shape.getIndiceStart(type);
   UINT indicesCount = gfx.shape.getIndiceCount(type);

   std::size_t namePos = texturefilename.find_last_of("/\\");
   std::string tag = "Normal#" + texturefilename.substr(namePos + 1);
   bool specular = false;
   if (!normalfilename.empty())
   {
      specular = true;
      namePos = normalfilename.find_last_of("/\\");
      tag += normalfilename.substr(namePos + 1);
   }

   std::shared_ptr<NormalObject> object = NormalObject::Resolve(gfx, tag);
   if (!object->isInitialized())
   {
      object->setInitialized();

      // Define the vertex input layout.
      using hw3dexp::VertexLayout;
      hw3dexp::VertexBuffer vbuf(std::move(
         VertexLayout{}
         .Append(VertexLayout::Position3D)
         .Append(VertexLayout::Normal)
         .Append(VertexLayout::Texture2D)
      ));

      struct Vertex
      {
         XMFLOAT3 pos;
         XMFLOAT3 normal;
         XMFLOAT2 tex;
      };
      auto model = gfx.shape.GetShapeTextureNormalData<Vertex>();

      for (int i = 0; i < model.vertices.size(); i++)
      {
         vbuf.EmplaceBack(
            *reinterpret_cast<XMFLOAT3 *>(&model.vertices[i].pos),
            *reinterpret_cast<XMFLOAT3 *>(&model.vertices[i].normal),
            *reinterpret_cast<XMFLOAT2 *>(&model.vertices[i].tex));
      }

      object->LoadVerticesBuffer(vbuf);
      object->LoadIndicesBuffer(model.indices);

      if (specular)
      {
         object->CreateShader(L"ModelVS.cso", L"PhongPSNormal.cso");
      }
      else
      {
         object->CreateShader(L"ModelVS.cso", L"ModelPS.cso");
      }

      // Create Root Signature after constants
      object->CreateRootSignature(false, false, false);
      object->CreatePipelineState(vbuf.GetLayout().GetD3DLayout(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

      object->SetLightView(lightView);

      m_material.specularInensity = 0.18f;
      m_material.specularPower = 18.0f;
      //m_material.specularInensity = 0.8f;
      //m_material.specularPower = 45.0f;
      ++MaterialIndex;

   }
   AddBind(std::move(object));


   std::shared_ptr<Texture> texture = Texture::Resolve(gfx, tag);
   if (!texture->isInitialized())
   {
      texture->setInitialized();
      texture->CreateTexture(texturefilename, 0, 3);
      if (specular)
      {
         texture->CreateTexture(normalfilename, 1, 3);
      }
   }
   AddBind(std::move(texture));

   std::shared_ptr < Transform > trans = std::make_shared<Transform>(gfx, *this, NormalObject::VIEW_CB, NormalObject::VIEW_PS_CB);
   UINT start = gfx.shape.getIndiceStart(type);
   UINT count = gfx.shape.getIndiceCount(type);
   trans->setIndices(index, start, count);
   ++index;

   AddBind(std::move(trans));
}

void DrawNormal::SetPos(DirectX::XMFLOAT3 pos) noexcept
{
   m_pos = pos;
   //this->m_pos = pos;
}

DirectX::XMMATRIX DrawNormal::GetTransformXM() const noexcept
{
   return DirectX::XMMatrixScaling(m_size, m_size, m_size) *
      DirectX::XMMatrixRotationRollPitchYaw(m_rot.x, m_rot.y, m_rot.z) *
      DirectX::XMMatrixTranslation(m_pos.x, m_pos.y, m_pos.z);
}

void DrawNormal::SyncMaterial() noexcept
{
   m_gfx.CopyMaterialConstant(m_materialIndex, m_material);
}

void DrawNormal::SpawnControlWindow(const std::string &name) noexcept
{
   DirectX::XMFLOAT3 pos;
   pos.x = m_pos.x;
   pos.y = m_pos.y;
   pos.z = m_pos.z;

   //using namespace std::string_literals;
   if (ImGui::Begin(name.c_str()))
   {
      ImGui::Text("Material");
      bool iChanged = ImGui::SliderFloat("Specular Intensity", &m_material.specularInensity, 0.05f, 4.0f, "%.2f");
      bool pChanged = ImGui::SliderFloat("Specular Power", &m_material.specularPower, 1.0f, 200.0f, "%.2f");

      ImGui::Text("Position");
      ImGui::SliderFloat("X", &m_pos.x, -50.0f, 50.0f, "%0.1f");
      ImGui::SliderFloat("Y", &m_pos.y, -50.0f, 50.0f, "%0.1f");
      ImGui::SliderFloat("Z", &m_pos.z, -50.0f, 50.0f, "%0.1f");

      ImGui::Text("Orientation");
      ImGui::SliderAngle("Roll", &m_rot.x, -180.0f, 180.0f);
      ImGui::SliderAngle("Pitch", &m_rot.y, -180.0f, 180.0f);
      ImGui::SliderAngle("Yaw", &m_rot.z, -180.0f, 180.0f);

      if (iChanged || pChanged)
      {
         SyncMaterial();
      }
   }
   ImGui::End();

}
