#include "ShapeAssimp.h"
#pragma once
#include "stdafx.h"
#define _USE_MATH_DEFINES
#include <math.h>

ShapeAssimp::ShapeAssimp()
{
   CreateTextureSuzanne();
}

void ShapeAssimp::SetNormals()
{
   using namespace DirectX;
   assert(indices.size() % 3 == 0 && indices.size() > 0);
   for (size_t i = 0; i < indices.size(); i += 3)
   {
      auto &v0 = vertices[indices[i]];
      auto &v1 = vertices[indices[i + 1]];
      auto &v2 = vertices[indices[i + 2]];
      const auto p0 = XMLoadFloat3(&v0.pos);
      const auto p1 = XMLoadFloat3(&v1.pos);
      const auto p2 = XMLoadFloat3(&v2.pos);

      const auto n = XMVector3Normalize(XMVector3Cross((p1 - p0), (p2 - p0)));

      XMStoreFloat3(&v0.normal, n);
      XMStoreFloat3(&v1.normal, n);
      XMStoreFloat3(&v2.normal, n);
   }
}

void ShapeAssimp::CreateTextureSuzanne()
{
   UINT startIndices = (UINT)indices.size();
   UINT startVertices = (UINT)vertices.size();
   int verticeCount = 0;
   int indiceCount = 0;

   //auto modelSuzanne = imp.ReadFile("models\\suzanne.obj",
   auto modelSuzanne = imp.ReadFile("..\\..\\DirectX12Charles\\Models\\suzanne.obj",
      aiProcess_Triangulate |
      aiProcess_JoinIdenticalVertices);

   C_STRUCT aiMesh *pMesh = modelSuzanne->mMeshes[0];
   //vertices.reserve(pMesh->mNumVertices);
   float scale = 1.0f;
   for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
   {
      vertices.push_back({ pMesh->mVertices[i].x * scale, pMesh->mVertices[i].y * scale, pMesh->mVertices[i].z * scale,
         pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z });
      ++verticeCount;
   }

   //indices.reserve(pMesh->mNumFaces * 3);
   for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
   {
      const auto &face = pMesh->mFaces[i];
      assert(face.mNumIndices == 3);
      indices.push_back(face.mIndices[0] + startVertices);
      indices.push_back(face.mIndices[1] + startVertices);
      indices.push_back(face.mIndices[2] + startVertices);
      indiceCount += 3;
   }

   m_indiceStart = startIndices;
   m_indiceCount = indiceCount;
   m_verticesStart = startVertices;
   m_verticesCount = verticeCount;
}
