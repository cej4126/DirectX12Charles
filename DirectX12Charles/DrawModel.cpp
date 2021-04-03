#include "DrawModel.h"
#include "imgui/imgui.h"
#include <unordered_map>
#include <sstream>
#include "ModelSpec.h"

#include "test.h"


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
      MeshPtrs.push_back(ParseMesh(index, *pScene->mMeshes[i], pScene->mMaterials, MaterialIndex));
      ++index;
   }
   test.addTestData(0, 0, "");

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
      for (int i = 0; i < m_material.size(); ++i)
      {
         gfx.CopyMaterialConstant(m_materialIndex + i, m_material.at(i));
      }
   }
}

std::unique_ptr<DrawMesh> DrawModel::ParseMesh(int index, const aiMesh &mesh, const aiMaterial *const *pMaterials, int &materialIndex)
{

   // create the tag and path
   std::size_t pos = filename.find_last_of("/\\");
   std::string path = filename.substr(0, pos) + std::string("\\");
   pos = path.find_last_of("/\\");
   std::string tag = "model";
   //tag += index;
   tag += path.substr(pos + 1);
   aiString diffuseName;
   aiString specularName;
   aiString normalName;
   Graphics::MaterialType material;

   bool diffuse = false;
   bool specular = false;
   bool normal = false;
   bool alphaGloss = false;

   XMFLOAT4 specularColor = { 0.18f, 0.18f, 0.18f, 1.0f };
   XMFLOAT4 diffuseColor = { 0.45f, 0.45f, 0.85f, 1.0f };

   auto &sceneMaterial = *pMaterials[mesh.mMaterialIndex];
   if (mesh.mMaterialIndex >= 0)
   {

      if (sceneMaterial.GetTexture(aiTextureType_DIFFUSE, 0, &diffuseName) == aiReturn_SUCCESS)
      {
         diffuse = true;
         tag += std::string("#") + diffuseName.C_Str();
      }
      else
      {
         sceneMaterial.Get(AI_MATKEY_COLOR_DIFFUSE, reinterpret_cast<aiColor3D &>(diffuseColor));
      }


      if (sceneMaterial.GetTexture(aiTextureType_SPECULAR, 0, &specularName) == aiReturn_SUCCESS)
      {
         specular = true;
         tag += std::string("#") + specularName.C_Str();
      }
      else
      {
         sceneMaterial.Get(AI_MATKEY_COLOR_SPECULAR, reinterpret_cast<aiColor3D &>(diffuseColor));
      }

      if (sceneMaterial.GetTexture(aiTextureType_NORMALS, 0, &normalName) == aiReturn_SUCCESS)
      {
         normal = true;
         tag += std::string("#") + normalName.C_Str();
      }
   }

   auto object = ModelSpec::Resolve(gfx, tag);

   int size = 0;

   if (!object->isInitialized())
   {
      object->setInitialized();

      float shininess = 35.0f;
      if (diffuse)
      {
         Surface surface = Surface::FromFile(path + diffuseName.C_Str());
         object->CreateTexture(surface, 0);
      }

      if (specular)
      {
         Surface surface = Surface::FromFile(path + specularName.C_Str());
         object->CreateTexture(surface, 1);
         alphaGloss = surface.AlphaLoaded();
      }

      if (!alphaGloss)
      {
         sceneMaterial.Get(AI_MATKEY_SHININESS, shininess);
      }

      if (normal)
      {
         Surface surface = Surface::FromFile(path + normalName.C_Str());
         object->CreateTexture(surface, 2);
         alphaGloss = surface.AlphaLoaded();
      }


      if (diffuse && normal && specular)
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
         size = (int)indices.size();

         object->LoadVerticesBuffer(vbuf);
         object->LoadIndicesBuffer(indices);

         object->CreateShader(L"ModelNormalTexVS.cso", L"ModelNormalSpecPS.cso");
         material.hasNormal = true;
         material.hasSpecular = true;
         material.specularPower = shininess;
         material.hasGloss = alphaGloss;
         material.specularColor = { 0.75f,0.75f,0.75f, 1.0f };
         material.specularWeight = 0.671f;


         // Create Root Signature after constants
         object->CreateRootSignature(ModelSpec::Model_Type::MODEL_DIFF_NORMAL_SPEC);
         object->CreatePipelineState(vbuf.GetLayout().GetD3DLayout(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
      }
      else if (diffuse && normal)
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
         size = (int)indices.size();

         object->LoadVerticesBuffer(vbuf);
         object->LoadIndicesBuffer(indices);

         object->CreateShader(L"ModelNormalTexVS.cso", L"ModelNormalTexPS.cso");
         material.specularInensity = (specularColor.x + specularColor.y + specularColor.z) / 3.0f;
         material.specularPower = shininess;
         material.hasNormal = true;
         material.hasGloss = alphaGloss;
         material.specularColor = XMFLOAT4(0.75f, 0.75f, 0.75f, 1.0f);
         material.specularWeight = 0.671f;
         material.hasSpecular = true;

         // Create Root Signature after constants
         object->CreateRootSignature(ModelSpec::Model_Type::MODEL_DIFF_NORMAL);
         object->CreatePipelineState(vbuf.GetLayout().GetD3DLayout(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
      }
      else if (diffuse && !normal && specular)
      {
         using hw3dexp::VertexLayout;
         hw3dexp::VertexBuffer vbuf(std::move(
            VertexLayout{}
            .Append(VertexLayout::Position3D)
            .Append(VertexLayout::Normal)
            .Append(VertexLayout::Texture2D)
         ));

         for (unsigned int i = 0; i < mesh.mNumVertices; i++)
         {
            XMFLOAT3 vertices = { mesh.mVertices[i].x * m_size, mesh.mVertices[i].y * m_size, mesh.mVertices[i].z * m_size };
            vbuf.EmplaceBack(
               *reinterpret_cast<XMFLOAT3 *>(&vertices),
               *reinterpret_cast<XMFLOAT3 *>(&mesh.mNormals[i]),
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
         size = (int)indices.size();

         object->LoadVerticesBuffer(vbuf);
         object->LoadIndicesBuffer(indices);

         object->CreateShader(L"ModelVS.cso", L"ModelSpecPS.cso");
         material.hasNormal = true;
         material.hasSpecular = true;
         material.specularPower = 2.0f;
         material.hasGloss = alphaGloss;
         material.specularColor = { 1.0f, 1.0f, 1.0f, 1.0f };
         material.specularWeight = 1.0f;

         // Create Root Signature after constants
         object->CreateRootSignature(ModelSpec::Model_Type::MODEL_DIFF_NORMAL_SPEC);
         object->CreatePipelineState(vbuf.GetLayout().GetD3DLayout(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
      }
      else if (diffuse)
      {
         using hw3dexp::VertexLayout;
         hw3dexp::VertexBuffer vbuf(std::move(
            VertexLayout{}
            .Append(VertexLayout::Position3D)
            .Append(VertexLayout::Normal)
            .Append(VertexLayout::Texture2D)
         ));

         for (unsigned int i = 0; i < mesh.mNumVertices; i++)
         {
            XMFLOAT3 vertices = { mesh.mVertices[i].x * m_size, mesh.mVertices[i].y * m_size, mesh.mVertices[i].z * m_size };
            vbuf.EmplaceBack(
               *reinterpret_cast<XMFLOAT3 *>(&vertices),
               *reinterpret_cast<XMFLOAT3 *>(&mesh.mNormals[i]),
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
         size = (int)indices.size();

         object->LoadVerticesBuffer(vbuf);
         object->LoadIndicesBuffer(indices);

         object->CreateShader(L"ModelNormalDiffuseVS.cso", L"ModelNormalDiffusePS.cso");
         material.specularInensity = (specularColor.x + specularColor.y + specularColor.z) / 3.0f;
         material.specularPower = shininess;
         material.hasNormal = true;
         material.hasGloss = alphaGloss;

         // Create Root Signature after constants
         object->CreateRootSignature(ModelSpec::Model_Type::MODEL_DIFF);
         object->CreatePipelineState(vbuf.GetLayout().GetD3DLayout(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
      }
      else if (!diffuse && !normal && !specular)
      {
         using hw3dexp::VertexLayout;
         hw3dexp::VertexBuffer vbuf(std::move(
            VertexLayout{}
            .Append(VertexLayout::Position3D)
            .Append(VertexLayout::Normal)
         ));

         for (unsigned int i = 0; i < mesh.mNumVertices; i++)
         {
            XMFLOAT3 vertices = { mesh.mVertices[i].x * m_size, mesh.mVertices[i].y * m_size, mesh.mVertices[i].z * m_size };
            vbuf.EmplaceBack(
               *reinterpret_cast<XMFLOAT3 *>(&vertices),
               *reinterpret_cast<XMFLOAT3 *>(&mesh.mNormals[i]));
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
         size = (int)indices.size();

         object->LoadVerticesBuffer(vbuf);
         object->LoadIndicesBuffer(indices);

         object->CreateShader(L"ModelNormalVS.cso", L"ModelNormalPS.cso");
         material.materialColor = diffuseColor;
         material.specularColor = specularColor;
         material.specularInensity = (specularColor.x + specularColor.y + specularColor.z) / 3.0f;
         material.specularPower = shininess;
         material.hasNormal = false;
         material.hasGloss = alphaGloss;

         // Create Root Signature after constants
         object->CreateRootSignature(ModelSpec::Model_Type::MODEL_NONE);
         object->CreatePipelineState(vbuf.GetLayout().GetD3DLayout(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
      }
      else
      {
         throw std::runtime_error("ERROR in file");
      }
      object->setSize(size);

      object->SetLightView(lightView);

      m_material.push_back(material);
      ++materialIndex;
   }
   else
   {
      size = object->getSize();
   }

   bindablePtrs.push_back(std::move(object));
   return std::make_unique<DrawMesh>(gfx, index, std::move(bindablePtrs), (UINT)size, materialIndex);
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
