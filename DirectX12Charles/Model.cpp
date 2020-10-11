#include "Model.h"

Mesh::Mesh(Graphics &gfx, std::vector<std::unique_ptr<Bindable>> bindPtrs, int indicesCount)
{
   for (auto &pb : bindPtrs)
   {
      AddBind(std::move(pb));
   }

   std::unique_ptr < Transform > trans = std::make_unique<Transform>(gfx, *this);
   trans->setIndices(0, indicesCount);
   AddBind(std::move(trans));
}

void Mesh::Draw(Graphics &gfx, FXMMATRIX acculatedTransform) const noexcept
{
   XMStoreFloat4x4(&transform, acculatedTransform);
   DrawFunction::Draw(gfx, 0);
}


Model::Model(Graphics &gfx, const std::string fileName, ID3D12Resource *lightView, int MaterialIndex)
   :
   gfx(gfx),
   MaterialIndex(MaterialIndex),
   lightView(lightView)
{
   Assimp::Importer imp;

   const auto pScene = imp.ReadFile(fileName.c_str(),
      aiProcess_Triangulate |
      aiProcess_JoinIdenticalVertices);

   for (size_t i = 0; i < pScene->mNumMeshes; i++)
   {
      meshPtrs.push_back(ParseMesh(*pScene->mMeshes[i]));
   }

   pRoot = ParseNode(*pScene->mRootNode);
}

std::unique_ptr<Node> Model::ParseNode(const aiNode &node)
{
   const auto transform = XMMatrixTranspose(DirectX::XMLoadFloat4x4(
      reinterpret_cast<const XMFLOAT4X4 *>(&node.mTransformation)));

   std::vector<Mesh *> curMeshPtrs;
   curMeshPtrs.reserve(node.mNumMeshes);
   for (size_t i = 0; i < node.mNumMeshes; i++)
   {
      const auto meshIndex = node.mMeshes[i];
      curMeshPtrs.push_back(meshPtrs.at(meshIndex).get());
   }

   auto pNode = std::make_unique<Node>(std::move(curMeshPtrs), transform);
   for (size_t i = 0; i < node.mNumChildren; i++)
   {
      pNode->AddChild(ParseNode(*node.mChildren[i]));
   }
   return pNode;
}

std::unique_ptr<Mesh> Model::ParseMesh(const aiMesh &mesh)
{
   using hw3dexp::VertexLayout;
   hw3dexp::VertexBuffer vbuf(std::move(
      VertexLayout{}
      .Append(VertexLayout::Position3D)
      .Append(VertexLayout::Normal)
   ));

   for (unsigned int i = 0; i < mesh.mNumVertices; i++)
   {
      vbuf.EmplaceBack(
         *reinterpret_cast<XMFLOAT3 *>(&mesh.mVertices[i]),
         *reinterpret_cast<XMFLOAT3 *>(&mesh.mNormals[i])
      );
   }

   std::vector<unsigned short> indices;
   indices.reserve(mesh.mNumFaces * 3);
   for (unsigned int i = 0; i < mesh.mNumFaces; i++)
   {
      const auto &face = mesh.mFaces[i];
      assert(face.mNumIndices == 3);
      indices.push_back(face.mIndices[0]);
      indices.push_back(face.mIndices[1]);
      indices.push_back(face.mIndices[2]);
   }

   std::unique_ptr<ModelObject> object = std::make_unique<ModelObject>(gfx);

   object->LoadVerticesBuffer(vbuf);

   object->LoadIndicesBuffer(indices);
   object->CreateShader(L"LightedVS.cso", L"LightedPS.cso");

   XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
   object->CreateConstant(position);

   // Create Root Signature after constants
   object->CreateRootSignature(true, false);

   object->CreatePipelineState(vbuf.GetLayout().GetD3DLayout(), D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);

   object->SetLightView(lightView);

   bindablePtrs.push_back(std::move(object));
   return std::make_unique<Mesh>(gfx, std::move(bindablePtrs), (UINT)indices.size());
}

void Model::Draw(Graphics &gfx) const
{
   pRoot->Draw(gfx, DirectX::XMMatrixIdentity());
}

