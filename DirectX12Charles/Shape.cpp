#include "Shape.h"
#pragma once
#include "stdafx.h"
#define _USE_MATH_DEFINES
#include <math.h>

Shape::Shape()
{
   for (int i = 0; i < ShapeCount; i++)
   {
      switch (i)
      {
         //case Cube:
         //   CreateCube();
         //   break;
         //case Cone:
         //   CreateCone(24);
         //   break;
         //case Prism:
         //   CratePrism();
         //   break;
         //case Cylinder:
         //   CreateCylinder(24);
         //   break;
         case Sphere:
            CreateSphere(12, 24);
            break;
         //case Plane:
         //   CreatePlane(1, 1);
         //   break;
         case TextureCube:
            CreateTextureCube();
            break;
         case TextureCylinder:
            CreateTextureCylinder(8);
            break;
         case TextureSuzanne:
            CreateTextureSuzanne();
      }
   }
}


void Shape::SetNormals()
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

void Shape::CreateCube()
{
   shapeType type = Cube;
   UINT startIndices = (UINT)indices.size();
   UINT startVertices = (UINT)vertices.size();
   constexpr float side = 1.0f;

   vertices.emplace_back(-side, -side, -side); // 0
   vertices.emplace_back(side, -side, -side); // 1
   vertices.emplace_back(-side, side, -side); // 2
   vertices.emplace_back(side, side, -side); // 3
   vertices.emplace_back(-side, -side, side); // 4
   vertices.emplace_back(side, -side, side); // 5
   vertices.emplace_back(-side, side, side); // 6
   vertices.emplace_back(side, side, side); // 7

   const unsigned short cubeindices[]
   {
         0,2,1, 2,3,1,
         1,3,5, 3,7,5,
         2,6,3, 3,6,7,
         4,5,7, 4,7,6,
         0,4,2, 2,4,6,
         0,1,4, 1,5,4
   };
   for (int i = 0; i < _countof(cubeindices); i++)
   {
      indices.push_back(cubeindices[i] + startVertices);
   }

   shapedata[type].indiceStart = startIndices;
   shapedata[type].indiceCount = (UINT)indices.size() - startIndices;
   shapedata[type].verticesStart = startVertices;
   shapedata[type].verticesCount = (UINT)vertices.size() - startVertices;
}

void Shape::CreateCone(int longDiv)
{
   shapeType type = Cone;
   UINT startIndices = (UINT)indices.size();
   UINT startVertices = (UINT)vertices.size();

   assert(longDiv >= 3);

   const auto base = XMVectorSet(1.0f, 0.0f, -1.0f, 0.0f);
   const float longitudeAngle = 2.0f * (float)M_PI / longDiv;

   // base vertices
   for (int iLong = 0; iLong < longDiv; iLong++)
   {
      vertices.emplace_back();
      auto v = XMVector3Transform(
         base,
         XMMatrixRotationZ(longitudeAngle * iLong)
      );
      XMStoreFloat3(&vertices.back().pos, v);
   }
   // the center
   vertices.emplace_back();
   vertices.back().pos = { 0.0f,0.0f,-1.0f };
   const auto iCenter = (unsigned short)(vertices.size() - startVertices - 1);
   // the tip :darkness:
   vertices.emplace_back();
   vertices.back().pos = { 0.0f,0.0f,1.0f };
   const auto iTip = (unsigned short)(vertices.size() - startVertices - 1);

   // base indices
   for (unsigned short iLong = 0; iLong < longDiv; iLong++)
   {
      indices.push_back(iCenter + startVertices);
      indices.push_back(((iLong + 1) % longDiv) + startVertices);
      indices.push_back(iLong + startVertices);
   }

   // cone indices
   for (unsigned short iLong = 0; iLong < longDiv; iLong++)
   {
      indices.push_back(iLong + startVertices);
      indices.push_back(((iLong + 1) % longDiv) + startVertices);
      indices.push_back(iTip + startVertices);
   }

   shapedata[type].indiceStart = startIndices;
   shapedata[type].indiceCount = (UINT)indices.size() - startIndices;
   shapedata[type].verticesStart = startVertices;
   shapedata[type].verticesCount = (UINT)vertices.size() - startVertices;
}

void Shape::CratePrism()
{
   shapeType type = Prism;
   UINT startIndices = (UINT)indices.size();
   UINT startVertices = (UINT)vertices.size();

   constexpr float edge = 2.0f;
   const float halfhight = (float)sqrt(2.0) * edge / 2.0f / 2.0f;
   constexpr float side = edge / 2.0f;

   vertices.emplace_back(-side, -halfhight, side); // 0
   vertices.emplace_back(0.0f, halfhight, side); // 1
   vertices.emplace_back(side, -halfhight, side); // 2
   vertices.emplace_back(-side, -halfhight, -side); // 3
   vertices.emplace_back(0.0f, halfhight, -side); // 4
   vertices.emplace_back(side, -halfhight, -side); // 5

   const unsigned short prismindices[]
   {
      3,4,5,        // front face
      1,0,2,        // back face
      0,1,3, 3,1,4, // left face
      4,1,5, 1,2,5, // 5,1,4, 5,2,1, // right face
      0,3,2, 3,5,2  // bottom face
   };

   for (int i = 0; i < _countof(prismindices); i++)
   {
      indices.push_back(prismindices[i] + startVertices);
   }

   shapedata[type].indiceStart = startIndices;
   shapedata[type].indiceCount = (UINT)indices.size() - startIndices;
   shapedata[type].verticesStart = startVertices;
   shapedata[type].verticesCount = (UINT)vertices.size() - startVertices;
}

void Shape::CreatePlane(int divisions_x, int divisions_y)
{
   shapeType type = Plane;
   UINT startIndices = (UINT)indices.size();
   UINT startVertices = (UINT)vertices.size();

   assert(divisions_x >= 1);
   assert(divisions_y >= 1);

   constexpr float width = 2.0f;
   constexpr float height = 2.0f;
   const int nVertices_x = divisions_x + 1;
   const int nVertices_y = divisions_y + 1;
   {
      const float side_x = width / 2.0f;
      const float side_y = height / 2.0f;
      const float divisionSize_x = width / float(divisions_x);
      const float divisionSize_y = height / float(divisions_y);
      const auto bottomLeft = XMVectorSet(-side_x, -side_y, 0.0f, 0.0f);

      for (int y = 0, i = 0; y < nVertices_y; y++)
      {
         const float y_pos = float(y) * divisionSize_y;
         for (int x = 0; x < nVertices_x; x++, i++)
         {
            const auto v = XMVectorAdd(
               bottomLeft,
               XMVectorSet(float(x) * divisionSize_x, y_pos, 0.0f, 0.0f)
            );
            vertices.emplace_back();
            XMStoreFloat3(&vertices.back().pos, v);
         }
      }
   }

   indices.reserve((UINT)(sqrt(divisions_x * divisions_y) * 6));
   {
      const auto vxy2i = [nVertices_x](size_t x, size_t y)
      {
         return (unsigned short)(y * nVertices_x + x);
      };
      for (size_t y = 0; y < divisions_y; y++)
      {
         for (size_t x = 0; x < divisions_x; x++)
         {
            const std::array<unsigned short, 4> indexArray =
            { vxy2i(x,y), vxy2i(x + 1,y), vxy2i(x,y + 1), vxy2i(x + 1,y + 1) };
            indices.push_back(indexArray[0] + startVertices);
            indices.push_back(indexArray[2] + startVertices);
            indices.push_back(indexArray[1] + startVertices);
            indices.push_back(indexArray[1] + startVertices);
            indices.push_back(indexArray[2] + startVertices);
            indices.push_back(indexArray[3] + startVertices);
         }
      }
   }

   shapedata[type].indiceStart = startIndices;
   shapedata[type].indiceCount = (UINT)indices.size() - startIndices;
   shapedata[type].verticesStart = startVertices;
   shapedata[type].verticesCount = (UINT)vertices.size() - startVertices;
}

void Shape::CreateCylinder(int longDiv)
{
   float size = 3.0f;
   shapeType type = Cylinder;
   UINT startIndices = (UINT)indices.size();
   UINT startVertices = (UINT)vertices.size();

   assert(longDiv >= 3);
   const auto base = XMVectorSet(size, 0.0f, -size, 0.0f);
   const auto offset = XMVectorSet(0.0f, 0.0f, 2.0f * size, 0.0f);
   const float longitudeAngle = 2.0f * (float)M_PI / longDiv;

   // near center
   vertices.emplace_back();
   vertices.back().pos = { 0.0f, 0.0f, -size };
   const auto iCenterNear = (unsigned short)(vertices.size() - startVertices - 1);
   // far center
   vertices.emplace_back();
   vertices.back().pos = { 0.0f, 0.0f, size };
   const auto iCenterFar = (unsigned short)(vertices.size() - startVertices - 1);

   // base vertices
   for (int iLong = 0; iLong < longDiv; iLong++)
   {
      // near base
      {
         vertices.emplace_back();
         auto v = XMVector3Transform(
            base,
            XMMatrixRotationZ(longitudeAngle * iLong)
         );
         XMStoreFloat3(&vertices.back().pos, v);
      }
      // far base
      {
         vertices.emplace_back();
         auto v = XMVector3Transform(
            base,
            XMMatrixRotationZ(longitudeAngle * iLong)
         );
         v = XMVectorAdd(v, offset);
         XMStoreFloat3(&vertices.back().pos, v);
      }
   }

   // side indices
   for (unsigned short iLong = 0; iLong < longDiv; iLong++)
   {
      const auto i = iLong * 2;
      const auto mod = longDiv * 2;
      indices.push_back(i + 2 + startVertices);
      indices.push_back(((i + 2) % mod + 2) + startVertices);
      indices.push_back(i + 1 + 2 + startVertices);
      indices.push_back(((i + 2) % mod + 2) + startVertices);
      indices.push_back(((i + 3) % mod + 2) + startVertices);
      indices.push_back(i + 1 + 2 + startVertices);
   }

   // base indices
   for (unsigned short iLong = 0; iLong < longDiv; iLong++)
   {
      const auto i = iLong * 2;
      const auto mod = longDiv * 2;
      indices.push_back(i + 2 + startVertices);
      indices.push_back(iCenterNear + startVertices);
      indices.push_back(((i + 2) % mod + 2) + startVertices);
      indices.push_back(iCenterFar + startVertices);
      indices.push_back(i + 1 + 2 + startVertices);
      indices.push_back(((i + 3) % mod + 2) + startVertices);
   }

   shapedata[type].indiceStart = startIndices;
   shapedata[type].indiceCount = (UINT)indices.size() - startIndices;
   shapedata[type].verticesStart = startVertices;
   shapedata[type].verticesCount = (UINT)vertices.size() - startVertices;
}

void Shape::CreateSphere(int latDiv, int longDiv)
{
   shapeType type = Sphere;
   UINT startIndices = (UINT)indices.size();
   UINT startVertices = (UINT)vertices.size();

   assert(latDiv >= 3);
   assert(longDiv >= 3);

   constexpr float radius = 1.0f;
   const auto base = XMVectorSet(0.0f, 0.0f, radius, 0.0f);
   const float lattitudeAngle = (float)M_PI / latDiv;
   const float longitudeAngle = 2.0f * (float)M_PI / longDiv;

   for (int iLat = 1; iLat < latDiv; iLat++)
   {
      const auto latBase = XMVector3Transform(
         base,
         XMMatrixRotationX(lattitudeAngle * iLat)
      );
      for (int iLong = 0; iLong < longDiv; iLong++)
      {
         vertices.emplace_back();
         auto v = XMVector3Transform(
            latBase,
            XMMatrixRotationZ(longitudeAngle * iLong)
         );
         XMStoreFloat3(&vertices.back().pos, v);
      }
   }

   // add the cap vertices
   const auto iNorthPole = (unsigned short)vertices.size() - startVertices;
   vertices.emplace_back();
   XMStoreFloat3(&vertices.back().pos, base);
   const auto iSouthPole = (unsigned short)vertices.size() - startVertices;
   vertices.emplace_back();
   XMStoreFloat3(&vertices.back().pos, XMVectorNegate(base));

   const auto calcIdx = [latDiv, longDiv](unsigned short iLat, unsigned short iLong)
   {
      return iLat * longDiv + iLong;
   };
   for (unsigned short iLat = 0; iLat < latDiv - 2; iLat++)
   {
      for (unsigned short iLong = 0; iLong < longDiv - 1; iLong++)
      {
         indices.push_back(calcIdx(iLat, iLong) + startVertices);
         indices.push_back(calcIdx(iLat + 1, iLong) + startVertices);
         indices.push_back(calcIdx(iLat, iLong + 1) + startVertices);
         indices.push_back(calcIdx(iLat, iLong + 1) + startVertices);
         indices.push_back(calcIdx(iLat + 1, iLong) + startVertices);
         indices.push_back(calcIdx(iLat + 1, iLong + 1) + startVertices);
      }
      // wrap band
      indices.push_back(calcIdx(iLat, longDiv - 1) + startVertices);
      indices.push_back(calcIdx(iLat + 1, longDiv - 1) + startVertices);
      indices.push_back(calcIdx(iLat, 0) + startVertices);
      indices.push_back(calcIdx(iLat, 0) + startVertices);
      indices.push_back(calcIdx(iLat + 1, longDiv - 1) + startVertices);
      indices.push_back(calcIdx(iLat + 1, 0) + startVertices);
   }

   // cap fans
   for (unsigned short iLong = 0; iLong < longDiv - 1; iLong++)
   {
      // north
      indices.push_back(iNorthPole + startVertices);
      indices.push_back(calcIdx(0, iLong) + startVertices);
      indices.push_back(calcIdx(0, iLong + 1) + startVertices);
      // south
      indices.push_back(calcIdx(latDiv - 2, iLong + 1) + startVertices);
      indices.push_back(calcIdx(latDiv - 2, iLong) + startVertices);
      indices.push_back(iSouthPole + startVertices);
   }
   // wrap triangles
   // north
   indices.push_back(iNorthPole + startVertices);
   indices.push_back(calcIdx(0, longDiv - 1) + startVertices);
   indices.push_back(calcIdx(0, 0) + startVertices);
   // south
   indices.push_back(calcIdx(latDiv - 2, 0) + startVertices);
   indices.push_back(calcIdx(latDiv - 2, longDiv - 1) + startVertices);
   indices.push_back(iSouthPole + startVertices);

   shapedata[type].indiceStart = startIndices;
   shapedata[type].indiceCount = (UINT)indices.size() - startIndices;
   shapedata[type].verticesStart = startVertices;
   shapedata[type].verticesCount = (UINT)vertices.size() - startVertices;
}


void Shape::CreateTextureCube()
{
   shapeType type = TextureCube;
   UINT startIndices = (UINT)indices.size();
   UINT startVertices = (UINT)vertices.size();
   constexpr float side = 1.0f;

   const float u1 = 0.0f;
   const float u2 = 1.0f / 3.0f;
   const float u3 = 2.0f / 3.0f;
   const float u4 = 1.0f;
   const float v1 = 0.0f;
   const float v2 = 1.0f / 4.0f;
   const float v3 = 2.0f / 4.0f;
   const float v4 = 3.0f / 4.0f;
   const float v5 = 1.0f;
   // Y
   // |
   //-Z - X
   // Front                X      Y      Z
   vertices.emplace_back(-side, side, -side, u3, v2); //  0
   vertices.emplace_back(side, side, -side, u3, v3); //  1
   vertices.emplace_back(side, -side, -side, u2, v3); //  2
   vertices.emplace_back(-side, -side, -side, u2, v2); //  3

   // Top
   vertices.emplace_back(-side, side, side, u4, v2); //  4
   vertices.emplace_back(side, side, side, u4, v3); //  5
   vertices.emplace_back(side, side, -side, u3, v3); //  6
   vertices.emplace_back(-side, side, -side, u3, v2); //  7

   // Back
   vertices.emplace_back(side, side, side, u3, v4); //  8
   vertices.emplace_back(-side, side, side, u3, v5); //  9
   vertices.emplace_back(-side, -side, side, u2, v5); // 10
   vertices.emplace_back(side, -side, side, u2, v4); // 11

   // Bottom
   vertices.emplace_back(-side, -side, -side, u2, v2); // 12
   vertices.emplace_back(side, -side, -side, u2, v3); // 13
   vertices.emplace_back(side, -side, side, u1, v3); // 14
   vertices.emplace_back(-side, -side, side, u1, v2); // 15

   // Right
   vertices.emplace_back(side, side, -side, u3, v3); // 16
   vertices.emplace_back(side, side, side, u3, v4); // 17
   vertices.emplace_back(side, -side, side, u2, v4); // 18
   vertices.emplace_back(side, -side, -side, u2, v3); // 19

   // Left
   vertices.emplace_back(-side, side, side, u3, v1); // 20
   vertices.emplace_back(-side, side, -side, u3, v2); // 21
   vertices.emplace_back(-side, -side, -side, u2, v2); // 22
   vertices.emplace_back(-side, -side, side, u2, v1); // 23

   const unsigned short cubeindices[]
   {
       0, 2, 3,  0, 1, 2,  // Front
       4, 6, 7,  4, 5, 6,  // Top
       8,10,11,  8, 9,10,  // Back
      12,14,15, 12,13,14,  // Bottom
      16,18,19, 16,17,18,  // Right
      20,22,23, 20,21,22   // Left
   };
   for (int i = 0; i < _countof(cubeindices); i++)
   {
//      indices.push_back(cubeindices[i] + startVertices);
      indices.emplace_back(cubeindices[i] + startVertices);
   }

   shapedata[type].indiceStart = startIndices;
   shapedata[type].indiceCount = (UINT)indices.size() - startIndices;
   shapedata[type].verticesStart = startVertices;
   shapedata[type].verticesCount = (UINT)vertices.size() - startVertices;
}

void Shape::CreateTextureCylinder(int longDiv)
{
   float size = 1.0f;
   shapeType type = TextureCylinder;
   UINT startIndices = (UINT)indices.size();
   UINT startVertices = (UINT)vertices.size();

   assert(longDiv >= 3);
   const auto offset = XMVectorSet(0.0f, 0.0f, 2.0f * size, 0.0f);
   const float longitudeAngle = 2.0f * (float)M_PI / longDiv;

   int verticeCount = 0;
   int indiceCount = 0;
   // Create top and bottom face
   for (int face = 0; face < 2; ++face)
   {
      unsigned short circleVerticeStart = verticeCount;
      // center
      float centerZ = size + (-2 * face) * size;
      vertices.emplace_back();
      vertices.back().pos = { 0.0f, 0.0f, centerZ };
      ++verticeCount;

      const auto arm = XMVectorSet(size, 0.0f, centerZ, 0.0f);

      for (int iLong = 0; iLong < longDiv; iLong++)
      {
         // Set vertices
         vertices.emplace_back();
         auto v = XMVector3Transform(
            arm,
            XMMatrixRotationZ(longitudeAngle * iLong)
         );
         XMStoreFloat3(&vertices.back().pos, v);
         ++verticeCount;
      }

      for (int iLong = 0; iLong < longDiv; iLong++)
      {
         // Set Indices
         if (face == 1)
         {
            indices.push_back(iLong + 1 + circleVerticeStart + startVertices);
            indices.push_back(circleVerticeStart + startVertices);
            indices.push_back((iLong + 1) % longDiv + circleVerticeStart + 1 + startVertices);
         }
         else
         {
            indices.push_back((iLong + 1) % longDiv + circleVerticeStart + 1 + startVertices);
            indices.push_back(circleVerticeStart + startVertices);
            indices.push_back(iLong + 1 + circleVerticeStart + startVertices);
         }
         indiceCount += 3;
      }
   }

   // side
   XMVECTOR arm[2] =
   {
      XMVectorSet(size, 0.0f, size, 0.0f),
      XMVectorSet(size, 0.0f, -size, 0.0f)
   };

   for (int iLong = 0; iLong < longDiv; iLong++)
   {
      int startVerticeStart = verticeCount + startVertices;
      for (int i = 0; i < 4; i++)
      {
         int j = i / 2;
         // Set vertices
         vertices.emplace_back();
         auto v1 = XMVector3Transform(
            arm[i % 2],
            XMMatrixRotationZ(longitudeAngle * (iLong + j))
         );
         XMStoreFloat3(&vertices.back().pos, v1);
         ++verticeCount;
      }

      indices.push_back(startVerticeStart + 0);
      indices.push_back(startVerticeStart + 1);
      indices.push_back(startVerticeStart + 3);

      indices.push_back(startVerticeStart + 0);
      indices.push_back(startVerticeStart + 3);
      indices.push_back(startVerticeStart + 2);

      indiceCount += 6;
   }

   shapedata[type].indiceStart = startIndices;
   shapedata[type].indiceCount = indiceCount;
   shapedata[type].verticesStart = startVertices;
   shapedata[type].verticesCount = verticeCount;
}

void Shape::CreateTextureSuzanne()
{
   shapeType type = TextureSuzanne;
   UINT startIndices = (UINT)indices.size();
   UINT startVertices = (UINT)vertices.size();
   int verticeCount = 0;
   int indiceCount = 0;

   //auto modelSuzanne = imp.ReadFile("models\\suzanne.obj",
   auto modelSuzanne = imp.ReadFile("suzanne.obj",
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

   shapedata[type].indiceStart = startIndices;
   shapedata[type].indiceCount = indiceCount;
   shapedata[type].verticesStart = startVertices;
   shapedata[type].verticesCount = verticeCount;

}
