#include "DrawModel.h"
#include "imgui/imgui.h"
#include <unordered_map>

DrawModel::DrawModel(Graphics &gfx, int &index, float size, const std::string fileName, ID3D12Resource *lightView, int &MaterialIndex)
   :
   gfx(gfx),
   filename(fileName),
   lightView(lightView),
   m_size(size),
   pWindow(std::make_unique<DrawModelWindow>())
{
   Assimp::Importer imp;

   const auto pScene = imp.ReadFile(fileName.c_str(),
      aiProcess_Triangulate |
      aiProcess_JoinIdenticalVertices |
      aiProcess_ConvertToLeftHanded |
      aiProcess_GenNormals |
      aiProcess_CalcTangentSpace);

   m_materialIndex = MaterialIndex;

   for (size_t i = 0; i < pScene->mNumMeshes; i++)
   {
      MeshPtrs.push_back(ParseMesh(index, *pScene->mMeshes[i], pScene->mMaterials));
      ++index;
      MaterialIndex = m_materialIndex;
   }

   int nextId = 0;
   pRoot = ParseNode(nextId, *pScene->mRootNode);
}

DrawModel::~DrawModel() noexcept
{}

std::unique_ptr<DrawNode> DrawModel::ParseNode(int &nextId, const aiNode &node) noexcept
{
   const auto transform = XMMatrixTranspose(XMLoadFloat4x4(
      reinterpret_cast<const XMFLOAT4X4 *>(&node.mTransformation)));

   std::vector<DrawMesh *> curMeshPtrs;
   curMeshPtrs.reserve(node.mNumMeshes);
   for (size_t i = 0; i < node.mNumMeshes; i++)
   {
      const auto MeshIndex = node.mMeshes[i];
      curMeshPtrs.push_back(MeshPtrs.at(MeshIndex).get());
   }

   auto pNode = std::make_unique<DrawNode>(nextId++, node.mName.C_Str(), std::move(curMeshPtrs), transform);
   for (size_t i = 0; i < node.mNumChildren; i++)
   {
      pNode->AddChild(ParseNode(nextId, *node.mChildren[i]));
   }
   return pNode;
}

void DrawModel::FirstCommand()
{
   if (m_materialIndex != -1)
   {
      gfx.CopyMaterialConstant(m_materialIndex, m_material);
   }
}

std::unique_ptr<DrawMesh> DrawModel::ParseMesh(int index, const aiMesh &mesh, const aiMaterial *const *pMaterials)
{
   using hw3dexp::VertexLayout;
   hw3dexp::VertexBuffer vbuf(std::move(
      VertexLayout{}
      .Append(VertexLayout::Position3D)
      .Append(VertexLayout::Normal)
      .Append(VertexLayout::Tangent)
      .Append(VertexLayout::Bitangent)
      .Append(VertexLayout::Texture2D)
   ));

   for (unsigned int i = 0; i < mesh.mNumVertices; i++)
   {
      XMFLOAT3 vertices = { mesh.mVertices[i].x * m_size, mesh.mVertices[i].y * m_size, mesh.mVertices[i].z * m_size };
      vbuf.EmplaceBack(
         *reinterpret_cast<XMFLOAT3 *>(&vertices),
         *reinterpret_cast<XMFLOAT3 *>(&mesh.mNormals[i]),
         *reinterpret_cast<XMFLOAT3 *>(&mesh.mTangents[i]),
         *reinterpret_cast<XMFLOAT3 *>(&mesh.mBitangents[i]),
         *reinterpret_cast<XMFLOAT2 *>(&mesh.mTextureCoords[0][i])
      );
   }

   std::vector<unsigned short> indices;
   indices.reserve((size_t)mesh.mNumFaces * 3);
   for (unsigned int i = 0; i < mesh.mNumFaces; i++)
   {
      const auto &face = mesh.mFaces[i];
      assert(face.mNumIndices == 3);
      indices.push_back(face.mIndices[0]);
      indices.push_back(face.mIndices[1]);
      indices.push_back(face.mIndices[2]);
   }

   // create the tag and path
   std::size_t pos = filename.find_last_of("/\\");
   std::string path = filename.substr(0, pos) + std::string("\\");
   pos = path.find_last_of("/\\");
   std::string tag = path.substr(pos + 1);
   aiString diffuseName;
   aiString specularName;
   aiString normalName;

   bool diffuse = false;
   bool specular = false;
   bool normal = false;
   auto &material = *pMaterials[mesh.mMaterialIndex];
   if (mesh.mMaterialIndex >= 0)
   {
      if (material.GetTexture(aiTextureType_DIFFUSE, 0, &diffuseName) == aiReturn_SUCCESS)
      {
         diffuse = true;
         tag += std::string("#") + diffuseName.C_Str();
      }

      if (material.GetTexture(aiTextureType_SPECULAR, 0, &specularName) == aiReturn_SUCCESS)
      {
         specular = true;
         tag += std::string("#") + specularName.C_Str();
      }

      if (material.GetTexture(aiTextureType_NORMALS, 0, &normalName) == aiReturn_SUCCESS)
      {
         normal = true;
         tag += std::string("#") + normalName.C_Str();
      }
   }

   auto object = ModelSpec::Resolve(gfx, tag);

   if (!object->isInitialized())
   {
      object->setInitialized();

      float shininess = 35.0f;
      if (diffuse)
      {
         object->CreateTexture(Surface::FromFile(path + diffuseName.C_Str()), 0);
      }

      object->LoadVerticesBuffer(vbuf);
      object->LoadIndicesBuffer(indices);

      if (specular)
      {
         object->CreateTexture(Surface::FromFile(path + specularName.C_Str()), 1);
      }
      else
      {
         material.Get(AI_MATKEY_SHININESS, shininess);
      }

      if (normal)
      {
         object->CreateTexture(Surface::FromFile(path + normalName.C_Str()), 2);
      }

      if (specular)
      {
         object->CreateShader(L"ModelVSNormal.cso", L"ModelPSSpecNormal.cso");
      }
      else
      {
         object->CreateShader(L"ModelVSNormal.cso", L"ModelPSNormal.cso");
         // copy at FirstCommand
         m_material.specularInensity = 0.8f;
         m_material.specularPower = shininess;
      }
      m_material.hasNormal = normal;

      //XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
      //object->CreateConstant(position);

      // Create Root Signature after constants
      object->CreateRootSignature(false, false, false);

      object->CreatePipelineState(vbuf.GetLayout().GetD3DLayout(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

      object->SetLightView(lightView);
   }

   bindablePtrs.push_back(std::move(object));
   return std::make_unique<DrawMesh>(gfx, index, std::move(bindablePtrs), (UINT)indices.size(), m_materialIndex);
}

void DrawModel::Draw(Graphics &gfx) const
{
   if (auto node = pWindow->GetSelectedNode())
   {
      node->SetAppliedTransform(pWindow->GetTransform());
   }
   pRoot->Draw(gfx, XMMatrixIdentity());
}

void DrawModel::ShowWindow(const char *windowName) noexcept
{
   pWindow->Show(windowName, *pRoot);
}

void DrawModel::SetPosition(FXMMATRIX tf)
{
   pRoot->SetAppliedTransform(tf);
}
