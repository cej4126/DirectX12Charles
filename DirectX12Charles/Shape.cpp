#include "Shape.h"
#pragma once
#include "stdafx.h"
#define _USE_MATH_DEFINES
#include <math.h>

Shape::Shape()
{
   //for (int i = TextureCube; i < TextureCube+1; i++)
   for (int i = 0; i < ShapeCount; i++)
   {
      switch (i)
      {
         case Cube:
            createCube();
            break;
         case Cone:
            createCone(24);
            break;
         case Prism:
            createPrism();
            break;
         case Cylinder:
            createCylinder(24);
            break;
         case Sphere:
            createSphere(12, 24);
            break;
         case Plane:
            createPlane(1, 1);
            break;
         case TextureCube:
            createTextureCube();
            break;
         case TextureCylinder:
            createTextureCylinder(8);
            break;
         case TextureSuzanne:
            createTextureSuzanne();
            break;
         case TextureCone:
            createTextureCone(24);
            break;
         case TexturePrism:
            createTexturePrism();
            break;
         case PictureCube:
            createPictureCube();
            break;
      }
   }
}

void Shape::setNormals()
{
   using namespace DirectX;
   assert(m_indices.size() % 3 == 0 && m_indices.size() > 0);
   for (size_t i = 0; i < m_indices.size(); i += 3)
   {
      auto &v0 = m_vertices[m_indices[i]];
      auto &v1 = m_vertices[m_indices[i + 1]];
      auto &v2 = m_vertices[m_indices[i + 2]];
      const auto p0 = XMLoadFloat3(&v0.pos);
      const auto p1 = XMLoadFloat3(&v1.pos);
      const auto p2 = XMLoadFloat3(&v2.pos);

      const auto n = XMVector3Normalize(XMVector3Cross((p1 - p0), (p2 - p0)));

      XMStoreFloat3(&v0.normal, n);
      XMStoreFloat3(&v1.normal, n);
      XMStoreFloat3(&v2.normal, n);
   }
}

void Shape::createCube()
{
   shapeType type = Cube;
   UINT startIndices = (UINT)m_indices.size();
   UINT startVertices = (UINT)m_vertices.size();
   constexpr float side = 1.0f;

   m_vertices.emplace_back(-side, -side, -side); // 0
   m_vertices.emplace_back(side, -side, -side); // 1
   m_vertices.emplace_back(-side, side, -side); // 2
   m_vertices.emplace_back(side, side, -side); // 3
   m_vertices.emplace_back(-side, -side, side); // 4
   m_vertices.emplace_back(side, -side, side); // 5
   m_vertices.emplace_back(-side, side, side); // 6
   m_vertices.emplace_back(side, side, side); // 7

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
      m_indices.push_back(cubeindices[i] + startVertices);
   }

   m_shapedata[type].indiceStart = startIndices;
   m_shapedata[type].indiceCount = (UINT)m_indices.size() - startIndices;
   m_shapedata[type].verticesStart = startVertices;
   m_shapedata[type].verticesCount = (UINT)m_vertices.size() - startVertices;
}

void Shape::createCone(int longDiv)
{
   shapeType type = Cone;
   UINT startIndices = (UINT)m_indices.size();
   UINT startVertices = (UINT)m_vertices.size();

   assert(longDiv >= 3);

   const auto base = XMVectorSet(1.0f, 0.0f, -1.0f, 0.0f);
   const float longitudeAngle = 2.0f * (float)M_PI / longDiv;

   // base vertices
   for (int iLong = 0; iLong < longDiv; iLong++)
   {
      m_vertices.emplace_back();
      auto v = XMVector3Transform(
         base,
         XMMatrixRotationZ(longitudeAngle * iLong)
      );
      XMStoreFloat3(&m_vertices.back().pos, v);
   }
   // the center
   m_vertices.emplace_back();
   m_vertices.back().pos = { 0.0f,0.0f,-1.0f };
   const auto iCenter = (unsigned short)(m_vertices.size() - startVertices - 1);
   // the tip :darkness:
   m_vertices.emplace_back();
   m_vertices.back().pos = { 0.0f,0.0f,1.0f };
   const auto iTip = (unsigned short)(m_vertices.size() - startVertices - 1);

   // base indices
   for (unsigned short iLong = 0; iLong < longDiv; iLong++)
   {
      m_indices.push_back(iCenter + startVertices);
      m_indices.push_back(((iLong + 1) % longDiv) + startVertices);
      m_indices.push_back(iLong + startVertices);
   }

   // cone indices
   for (unsigned short iLong = 0; iLong < longDiv; iLong++)
   {
      m_indices.push_back(iLong + startVertices);
      m_indices.push_back(((iLong + 1) % longDiv) + startVertices);
      m_indices.push_back(iTip + startVertices);
   }

   m_shapedata[type].indiceStart = startIndices;
   m_shapedata[type].indiceCount = (UINT)m_indices.size() - startIndices;
   m_shapedata[type].verticesStart = startVertices;
   m_shapedata[type].verticesCount = (UINT)m_vertices.size() - startVertices;
}

void Shape::createPrism()
{
   shapeType type = Prism;
   UINT startIndices = (UINT)m_indices.size();
   UINT startVertices = (UINT)m_vertices.size();

   constexpr float edge = 2.0f;
   const float halfhight = (float)sqrt(2.0) * edge / 2.0f / 2.0f;
   constexpr float side = edge / 2.0f;

   m_vertices.emplace_back(-side, -halfhight, side); // 0
   m_vertices.emplace_back(0.0f, halfhight, side); // 1
   m_vertices.emplace_back(side, -halfhight, side); // 2
   m_vertices.emplace_back(-side, -halfhight, -side); // 3
   m_vertices.emplace_back(0.0f, halfhight, -side); // 4
   m_vertices.emplace_back(side, -halfhight, -side); // 5

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
      m_indices.push_back(prismindices[i] + startVertices);
   }

   m_shapedata[type].indiceStart = startIndices;
   m_shapedata[type].indiceCount = (UINT)m_indices.size() - startIndices;
   m_shapedata[type].verticesStart = startVertices;
   m_shapedata[type].verticesCount = (UINT)m_vertices.size() - startVertices;
}

void Shape::createPlane(int divisions_x, int divisions_y)
{
   shapeType type = Plane;
   UINT startIndices = (UINT)m_indices.size();
   UINT startVertices = (UINT)m_vertices.size();

   assert(divisions_x >= 1);
   assert(divisions_y >= 1);

   constexpr float width = 4.0f;
   constexpr float height = 4.0f;
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
            m_vertices.emplace_back(v, x / (nVertices_x - 1.0f), y / (nVertices_y - 1.0f));
         }
      }
   }

   m_indices.reserve((UINT)(sqrt(divisions_x * divisions_y) * 6));
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
            m_indices.push_back(indexArray[0] + startVertices);
            m_indices.push_back(indexArray[2] + startVertices);
            m_indices.push_back(indexArray[1] + startVertices);
            m_indices.push_back(indexArray[1] + startVertices);
            m_indices.push_back(indexArray[2] + startVertices);
            m_indices.push_back(indexArray[3] + startVertices);
         }
      }
   }

   m_shapedata[type].indiceStart = startIndices;
   m_shapedata[type].indiceCount = (UINT)m_indices.size() - startIndices;
   m_shapedata[type].verticesStart = startVertices;
   m_shapedata[type].verticesCount = (UINT)m_vertices.size() - startVertices;
}

void Shape::createCylinder(int longDiv)
{
   float size = 3.0f;
   shapeType type = Cylinder;
   UINT startIndices = (UINT)m_indices.size();
   UINT startVertices = (UINT)m_vertices.size();

   assert(longDiv >= 3);
   const auto base = XMVectorSet(size, 0.0f, -size, 0.0f);
   const auto offset = XMVectorSet(0.0f, 0.0f, 2.0f * size, 0.0f);
   const float longitudeAngle = 2.0f * (float)M_PI / longDiv;

   // near center
   m_vertices.emplace_back();
   m_vertices.back().pos = { 0.0f, 0.0f, -size };
   const auto iCenterNear = (unsigned short)(m_vertices.size() - startVertices - 1);
   // far center
   m_vertices.emplace_back();
   m_vertices.back().pos = { 0.0f, 0.0f, size };
   const auto iCenterFar = (unsigned short)(m_vertices.size() - startVertices - 1);

   // base vertices
   for (int iLong = 0; iLong < longDiv; iLong++)
   {
      // near base
      {
         m_vertices.emplace_back();
         auto v = XMVector3Transform(
            base,
            XMMatrixRotationZ(longitudeAngle * iLong)
         );
         XMStoreFloat3(&m_vertices.back().pos, v);
      }
      // far base
      {
         m_vertices.emplace_back();
         auto v = XMVector3Transform(
            base,
            XMMatrixRotationZ(longitudeAngle * iLong)
         );
         v = XMVectorAdd(v, offset);
         XMStoreFloat3(&m_vertices.back().pos, v);
      }
   }

   // side indices
   for (unsigned short iLong = 0; iLong < longDiv; iLong++)
   {
      const auto i = iLong * 2;
      const auto mod = longDiv * 2;
      m_indices.push_back(i + 2 + startVertices);
      m_indices.push_back(((i + 2) % mod + 2) + startVertices);
      m_indices.push_back(i + 1 + 2 + startVertices);
      m_indices.push_back(((i + 2) % mod + 2) + startVertices);
      m_indices.push_back(((i + 3) % mod + 2) + startVertices);
      m_indices.push_back(i + 1 + 2 + startVertices);
   }

   // base indices
   for (unsigned short iLong = 0; iLong < longDiv; iLong++)
   {
      const auto i = iLong * 2;
      const auto mod = longDiv * 2;
      m_indices.push_back(i + 2 + startVertices);
      m_indices.push_back(iCenterNear + startVertices);
      m_indices.push_back(((i + 2) % mod + 2) + startVertices);
      m_indices.push_back(iCenterFar + startVertices);
      m_indices.push_back(i + 1 + 2 + startVertices);
      m_indices.push_back(((i + 3) % mod + 2) + startVertices);
   }

   m_shapedata[type].indiceStart = startIndices;
   m_shapedata[type].indiceCount = (UINT)m_indices.size() - startIndices;
   m_shapedata[type].verticesStart = startVertices;
   m_shapedata[type].verticesCount = (UINT)m_vertices.size() - startVertices;
}

void Shape::createSphere(int latDiv, int longDiv)
{
   shapeType type = Sphere;
   UINT startIndices = (UINT)m_indices.size();
   UINT startVertices = (UINT)m_vertices.size();

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
         m_vertices.emplace_back();
         auto v = XMVector3Transform(
            latBase,
            XMMatrixRotationZ(longitudeAngle * iLong)
         );
         XMStoreFloat3(&m_vertices.back().pos, v);
      }
   }

   // add the cap vertices
   const auto iNorthPole = (unsigned short)m_vertices.size() - startVertices;
   m_vertices.emplace_back();
   XMStoreFloat3(&m_vertices.back().pos, base);
   const auto iSouthPole = (unsigned short)m_vertices.size() - startVertices;
   m_vertices.emplace_back();
   XMStoreFloat3(&m_vertices.back().pos, XMVectorNegate(base));

   const auto calcIdx = [latDiv, longDiv](unsigned short iLat, unsigned short iLong)
   {
      return iLat * longDiv + iLong;
   };
   for (unsigned short iLat = 0; iLat < latDiv - 2; iLat++)
   {
      for (unsigned short iLong = 0; iLong < longDiv - 1; iLong++)
      {
         m_indices.push_back(calcIdx(iLat, iLong) + startVertices);
         m_indices.push_back(calcIdx(iLat + 1, iLong) + startVertices);
         m_indices.push_back(calcIdx(iLat, iLong + 1) + startVertices);
         m_indices.push_back(calcIdx(iLat, iLong + 1) + startVertices);
         m_indices.push_back(calcIdx(iLat + 1, iLong) + startVertices);
         m_indices.push_back(calcIdx(iLat + 1, iLong + 1) + startVertices);
      }
      // wrap band
      m_indices.push_back(calcIdx(iLat, longDiv - 1) + startVertices);
      m_indices.push_back(calcIdx(iLat + 1, longDiv - 1) + startVertices);
      m_indices.push_back(calcIdx(iLat, 0) + startVertices);
      m_indices.push_back(calcIdx(iLat, 0) + startVertices);
      m_indices.push_back(calcIdx(iLat + 1, longDiv - 1) + startVertices);
      m_indices.push_back(calcIdx(iLat + 1, 0) + startVertices);
   }

   // cap fans
   for (unsigned short iLong = 0; iLong < longDiv - 1; iLong++)
   {
      // north
      m_indices.push_back(iNorthPole + startVertices);
      m_indices.push_back(calcIdx(0, iLong) + startVertices);
      m_indices.push_back(calcIdx(0, iLong + 1) + startVertices);
      // south
      m_indices.push_back(calcIdx(latDiv - 2, iLong + 1) + startVertices);
      m_indices.push_back(calcIdx(latDiv - 2, iLong) + startVertices);
      m_indices.push_back(iSouthPole + startVertices);
   }
   // wrap triangles
   // north
   m_indices.push_back(iNorthPole + startVertices);
   m_indices.push_back(calcIdx(0, longDiv - 1) + startVertices);
   m_indices.push_back(calcIdx(0, 0) + startVertices);
   // south
   m_indices.push_back(calcIdx(latDiv - 2, 0) + startVertices);
   m_indices.push_back(calcIdx(latDiv - 2, longDiv - 1) + startVertices);
   m_indices.push_back(iSouthPole + startVertices);

   m_shapedata[type].indiceStart = startIndices;
   m_shapedata[type].indiceCount = (UINT)m_indices.size() - startIndices;
   m_shapedata[type].verticesStart = startVertices;
   m_shapedata[type].verticesCount = (UINT)m_vertices.size() - startVertices;
}

void Shape::createTextureCube()
{
   shapeType type = TextureCube;
   UINT startIndices = (UINT)m_indices.size();
   UINT startVertices = (UINT)m_vertices.size();
   constexpr float side = 1.0f;

#ifdef PART_TEXTURE
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
   m_vertices.emplace_back(-side, side, -side, u3, v2); //  0
   m_vertices.emplace_back(side, side, -side, u3, v3); //  1
   m_vertices.emplace_back(side, -side, -side, u2, v3); //  2
   m_vertices.emplace_back(-side, -side, -side, u2, v2); //  3

   // Top
   m_vertices.emplace_back(-side, side, side, u4, v2); //  4
   m_vertices.emplace_back(side, side, side, u4, v3); //  5
   m_vertices.emplace_back(side, side, -side, u3, v3); //  6
   m_vertices.emplace_back(-side, side, -side, u3, v2); //  7

   // Back
   m_vertices.emplace_back(side, side, side, u3, v4); //  8
   m_vertices.emplace_back(-side, side, side, u3, v5); //  9
   m_vertices.emplace_back(-side, -side, side, u2, v5); // 10
   m_vertices.emplace_back(side, -side, side, u2, v4); // 11

   // Bottom
   m_vertices.emplace_back(-side, -side, -side, u2, v2); // 12
   m_vertices.emplace_back(side, -side, -side, u2, v3); // 13
   m_vertices.emplace_back(side, -side, side, u1, v3); // 14
   m_vertices.emplace_back(-side, -side, side, u1, v2); // 15

   // Right
   m_vertices.emplace_back(side, side, -side, u3, v3); // 16
   m_vertices.emplace_back(side, side, side, u3, v4); // 17
   m_vertices.emplace_back(side, -side, side, u2, v4); // 18
   m_vertices.emplace_back(side, -side, -side, u2, v3); // 19

   // Left
   m_vertices.emplace_back(-side, side, side, u3, v1); // 20
   m_vertices.emplace_back(-side, side, -side, u3, v2); // 21
   m_vertices.emplace_back(-side, -side, -side, u2, v2); // 22
   m_vertices.emplace_back(-side, -side, side, u2, v1); // 23
#else
   // Y
   // |
   //-Z - X
   // Front                X      Y      Z
   m_vertices.emplace_back(-side,  side, -side, 0.0f, 0.0f); //  0
   m_vertices.emplace_back( side,  side, -side, 1.0f, 0.0f); //  1
   m_vertices.emplace_back( side, -side, -side, 1.0f, 1.0f); //  2
   m_vertices.emplace_back(-side, -side, -side, 0.0f, 1.0f); //  3

   // Top
   m_vertices.emplace_back(-side,  side,  side, 0.0f, 0.0f); //  4
   m_vertices.emplace_back( side,  side,  side, 1.0f, 0.0f); //  5
   m_vertices.emplace_back( side,  side, -side, 1.0f, 1.0f); //  6
   m_vertices.emplace_back(-side,  side, -side, 0.0f, 1.0f); //  7

   // Back
   m_vertices.emplace_back( side,  side,  side, 0.0f, 0.0f); //  8
   m_vertices.emplace_back(-side,  side,  side, 1.0f, 0.0f); //  9
   m_vertices.emplace_back(-side, -side,  side, 1.0f, 1.0f); // 10
   m_vertices.emplace_back( side, -side,  side, 0.0f, 1.0f); // 11

   // Bottom
   m_vertices.emplace_back(-side, -side, -side, 0.0f, 0.0f); // 12
   m_vertices.emplace_back( side, -side, -side, 1.0f, 0.0f); // 13
   m_vertices.emplace_back( side, -side,  side, 1.0f, 1.0f); // 14
   m_vertices.emplace_back(-side, -side,  side, 0.0f, 1.0f); // 15

   // Right
   m_vertices.emplace_back( side,  side, -side, 0.0f, 0.0f); // 16
   m_vertices.emplace_back( side,  side,  side, 1.0f, 0.0f); // 17
   m_vertices.emplace_back( side, -side,  side, 1.0f, 1.0f); // 18
   m_vertices.emplace_back( side, -side, -side, 0.0f, 1.0f); // 19

   // Left
   m_vertices.emplace_back(-side,  side,  side, 0.0f, 0.0f); // 20
   m_vertices.emplace_back(-side,  side, -side, 1.0f, 0.0f); // 21
   m_vertices.emplace_back(-side, -side, -side, 1.0f, 1.0f); // 22
   m_vertices.emplace_back(-side, -side,  side, 0.0f, 1.0f); // 23
#endif
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
      //      m_indices.push_back(cubeindices[i] + startVertices);
      m_indices.emplace_back(cubeindices[i] + startVertices);
   }

   m_shapedata[type].indiceStart = startIndices;
   m_shapedata[type].indiceCount = (UINT)m_indices.size() - startIndices;
   m_shapedata[type].verticesStart = startVertices;
   m_shapedata[type].verticesCount = (UINT)m_vertices.size() - startVertices;
}


void Shape::createPictureCube()
{
   shapeType type = PictureCube;
   UINT startIndices = (UINT)m_indices.size();
   UINT startVertices = (UINT)m_vertices.size();
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
   m_vertices.emplace_back(-side, side, -side, u3, v2); //  0
   m_vertices.emplace_back(side, side, -side, u3, v3); //  1
   m_vertices.emplace_back(side, -side, -side, u2, v3); //  2
   m_vertices.emplace_back(-side, -side, -side, u2, v2); //  3

   // Top
   m_vertices.emplace_back(-side, side, side, u4, v2); //  4
   m_vertices.emplace_back(side, side, side, u4, v3); //  5
   m_vertices.emplace_back(side, side, -side, u3, v3); //  6
   m_vertices.emplace_back(-side, side, -side, u3, v2); //  7

   // Back
   m_vertices.emplace_back(side, side, side, u3, v4); //  8
   m_vertices.emplace_back(-side, side, side, u3, v5); //  9
   m_vertices.emplace_back(-side, -side, side, u2, v5); // 10
   m_vertices.emplace_back(side, -side, side, u2, v4); // 11

   // Bottom
   m_vertices.emplace_back(-side, -side, -side, u2, v2); // 12
   m_vertices.emplace_back(side, -side, -side, u2, v3); // 13
   m_vertices.emplace_back(side, -side, side, u1, v3); // 14
   m_vertices.emplace_back(-side, -side, side, u1, v2); // 15

   // Right
   m_vertices.emplace_back(side, side, -side, u3, v3); // 16
   m_vertices.emplace_back(side, side, side, u3, v4); // 17
   m_vertices.emplace_back(side, -side, side, u2, v4); // 18
   m_vertices.emplace_back(side, -side, -side, u2, v3); // 19

   // Left
   m_vertices.emplace_back(-side, side, side, u3, v1); // 20
   m_vertices.emplace_back(-side, side, -side, u3, v2); // 21
   m_vertices.emplace_back(-side, -side, -side, u2, v2); // 22
   m_vertices.emplace_back(-side, -side, side, u2, v1); // 23

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
      //      m_indices.push_back(cubeindices[i] + startVertices);
      m_indices.emplace_back(cubeindices[i] + startVertices);
   }

   m_shapedata[type].indiceStart = startIndices;
   m_shapedata[type].indiceCount = (UINT)m_indices.size() - startIndices;
   m_shapedata[type].verticesStart = startVertices;
   m_shapedata[type].verticesCount = (UINT)m_vertices.size() - startVertices;
}

void Shape::createTextureCylinder(int longDiv)
{
   float size = 1.0f;
   shapeType type = TextureCylinder;
   UINT startIndices = (UINT)m_indices.size();
   UINT startVertices = (UINT)m_vertices.size();

   assert(longDiv >= 3);
   //const auto offset = XMVectorSet(0.0f, 0.0f, 2.0f * size, 0.0f);
   const float longitudeAngle = 2.0f * (float)M_PI / longDiv;

   int verticeCount = 0;
   int indiceCount = 0;
   // Create top and bottom face
   for (int face = 0; face < 2; ++face)
   {
      unsigned short circleVerticeStart = verticeCount;
      // center
      float centerZ = size + (-2 * face) * size;
      m_vertices.emplace_back();
      m_vertices.back().pos = { 0.0f, 0.0f, centerZ };
      ++verticeCount;

      const auto arm = XMVectorSet(size, 0.0f, centerZ, 0.0f);

      for (int iLong = 0; iLong < longDiv; iLong++)
      {
         // Set vertices
         m_vertices.emplace_back();
         auto v = XMVector3Transform(
            arm,
            XMMatrixRotationZ(longitudeAngle * iLong)
         );
         XMStoreFloat3(&m_vertices.back().pos, v);
         ++verticeCount;
      }

      for (int iLong = 0; iLong < longDiv; iLong++)
      {
         // Set Indices
         if (face == 1)
         {
            m_indices.push_back(iLong + 1 + circleVerticeStart + startVertices);
            m_indices.push_back(circleVerticeStart + startVertices);
            m_indices.push_back((iLong + 1) % longDiv + circleVerticeStart + 1 + startVertices);
         }
         else
         {
            m_indices.push_back((iLong + 1) % longDiv + circleVerticeStart + 1 + startVertices);
            m_indices.push_back(circleVerticeStart + startVertices);
            m_indices.push_back(iLong + 1 + circleVerticeStart + startVertices);
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
         m_vertices.emplace_back();
         auto v1 = XMVector3Transform(
            arm[i % 2],
            XMMatrixRotationZ(longitudeAngle * (iLong + j))
         );
         XMStoreFloat3(&m_vertices.back().pos, v1);
         ++verticeCount;
      }

      m_indices.push_back(startVerticeStart + 0);
      m_indices.push_back(startVerticeStart + 1);
      m_indices.push_back(startVerticeStart + 3);

      m_indices.push_back(startVerticeStart + 0);
      m_indices.push_back(startVerticeStart + 3);
      m_indices.push_back(startVerticeStart + 2);

      indiceCount += 6;
   }

   m_shapedata[type].indiceStart = startIndices;
   m_shapedata[type].indiceCount = indiceCount;
   m_shapedata[type].verticesStart = startVertices;
   m_shapedata[type].verticesCount = verticeCount;
}

void Shape::createTextureCone(int longDiv)
{
   float size = 1.0f;
   shapeType type = TextureCone;
   UINT startIndices = (UINT)m_indices.size();
   UINT startVertices = (UINT)m_vertices.size();

   assert(longDiv >= 3);
   const float longitudeAngle = 2.0f * (float)M_PI / longDiv;

   int verticeCount = 0;
   int indiceCount = 0;
   // Create bottom face
   unsigned short circleVerticeStart = verticeCount;
   // center
   float centerZ = -size;
   m_vertices.emplace_back();
   m_vertices.back().pos = { 0.0f, 0.0f, centerZ };
   ++verticeCount;

   const auto arm = XMVectorSet(size, 0.0f, centerZ, 0.0f);

   for (int iLong = 0; iLong < longDiv; iLong++)
   {
      // Set vertices
      m_vertices.emplace_back();
      auto v = XMVector3Transform(
         arm,
         XMMatrixRotationZ(longitudeAngle * iLong)
      );
      XMStoreFloat3(&m_vertices.back().pos, v);
      ++verticeCount;
   }

   for (int iLong = 0; iLong < longDiv; iLong++)
   {
      // Set Indices
      m_indices.push_back(iLong + 1 + circleVerticeStart + startVertices);
      m_indices.push_back(circleVerticeStart + startVertices);
      m_indices.push_back((iLong + 1) % longDiv + circleVerticeStart + 1 + startVertices);
      indiceCount += 3;
   }

   // side
   XMVECTOR arm1 = XMVectorSet(size, 0.0f, -size, 0.0f);
   XMVECTOR center = XMVectorSet(0.0f, 0.0f, size, 0.0f);

   for (int iLong = 0; iLong < longDiv; iLong++)
   {
      int startVerticeStart = verticeCount;
      m_vertices.emplace_back();
      auto v1 = XMVector3Transform(arm1, XMMatrixRotationZ(longitudeAngle * iLong));
      XMStoreFloat3(&m_vertices.back().pos, v1);

      m_vertices.emplace_back();
      XMStoreFloat3(&m_vertices.back().pos, center);

      m_vertices.emplace_back();
      auto v2 = XMVector3Transform(arm1, XMMatrixRotationZ(longitudeAngle * (iLong + 1.0f)));
      XMStoreFloat3(&m_vertices.back().pos, v2);
      verticeCount += 3;

      m_indices.push_back(startVerticeStart + 0 + startVertices);
      m_indices.push_back(startVerticeStart + 2 + startVertices);
      m_indices.push_back(startVerticeStart + 1 + startVertices);

      indiceCount += 3;
   }

   m_shapedata[type].indiceStart = startIndices;
   m_shapedata[type].indiceCount = (UINT)m_indices.size() - startIndices;
   m_shapedata[type].verticesStart = startVertices;
   m_shapedata[type].verticesCount = (UINT)m_vertices.size() - startVertices;
}


void Shape::createTextureSuzanne()
{
   shapeType type = TextureSuzanne;
   UINT startIndices = (UINT)m_indices.size();
   UINT startVertices = (UINT)m_vertices.size();
   int verticeCount = 0;
   int indiceCount = 0;

   //auto modelSuzanne = imp.ReadFile("models\\suzanne.obj",
   auto modelSuzanne = m_imp.ReadFile("..\\..\\DirectX12Charles\\Models\\suzanne.obj",
      aiProcess_Triangulate |
      aiProcess_JoinIdenticalVertices);

   C_STRUCT aiMesh *pMesh = modelSuzanne->mMeshes[0];
   //vertices.reserve(pMesh->mNumVertices);
   float scale = 1.0f;
   for (unsigned int i = 0; i < pMesh->mNumVertices; i++)
   {
      m_vertices.push_back({ pMesh->mVertices[i].x * scale, pMesh->mVertices[i].y * scale, pMesh->mVertices[i].z * scale,
         pMesh->mNormals[i].x, pMesh->mNormals[i].y, pMesh->mNormals[i].z });
      ++verticeCount;
   }

   //indices.reserve(pMesh->mNumFaces * 3);
   for (unsigned int i = 0; i < pMesh->mNumFaces; i++)
   {
      const auto &face = pMesh->mFaces[i];
      assert(face.mNumIndices == 3);
      m_indices.push_back(face.mIndices[0] + startVertices);
      m_indices.push_back(face.mIndices[1] + startVertices);
      m_indices.push_back(face.mIndices[2] + startVertices);
      indiceCount += 3;
   }

   m_shapedata[type].indiceStart = startIndices;
   m_shapedata[type].indiceCount = indiceCount;
   m_shapedata[type].verticesStart = startVertices;
   m_shapedata[type].verticesCount = verticeCount;
}

void Shape::createTexturePrism()
{
   shapeType type = TexturePrism;
   UINT startIndices = (UINT)m_indices.size();
   UINT startVertices = (UINT)m_vertices.size();

   constexpr float edge = 2.0f;
   const float halfhight = (float)sqrt(2.0) * edge / 2.0f / 2.0f;
   constexpr float side = edge / 2.0f;
   //           ____
   //      ____    /\4
   //    1/\      /  \
   //    /  \  _3/__B_\5
   //  0/__F_\2 
   // Front
   m_vertices.emplace_back(-side, -halfhight,  -side); //  0 0
   m_vertices.emplace_back( 0.0f,  halfhight,  -side); //  1 1
   m_vertices.emplace_back( side, -halfhight,  -side); //  2 2
   // Back
   m_vertices.emplace_back(-side, -halfhight,  side); //  3 3
   m_vertices.emplace_back( 0.0f,  halfhight,  side); //  4 4
   m_vertices.emplace_back( side, -halfhight,  side); //  5 5
   // Right Side
   m_vertices.emplace_back( 0.0f,  halfhight, -side); //  6 1
   m_vertices.emplace_back( 0.0f,  halfhight,  side); //  7 4
   m_vertices.emplace_back( side, -halfhight,  side); //  8 5
   m_vertices.emplace_back( side, -halfhight, -side); //  9 2
   // Left Side
   m_vertices.emplace_back(-side, -halfhight, -side); // 10 0
   m_vertices.emplace_back(-side, -halfhight,  side); // 11 3
   m_vertices.emplace_back( 0.0f,  halfhight,  side); // 12 4
   m_vertices.emplace_back( 0.0f,  halfhight, -side); // 13 1
   // Bottom
   m_vertices.emplace_back(-side, -halfhight, -side); // 14 0
   m_vertices.emplace_back(-side, -halfhight,  side); // 15 3
   m_vertices.emplace_back( side, -halfhight, -side); // 16 2
   m_vertices.emplace_back( side, -halfhight,  side); // 17 5


   const unsigned short prismindices[]
   {
       0, 1, 2,           // front face
       3, 5, 4,           // back face
       6, 7, 8,  6, 8, 9, // right face
      10,11,12, 10,12,13, // left face
      14,16,17, 14,17,15  // bottom face
   };

   for (int i = 0; i < _countof(prismindices); i++)
   {
      m_indices.push_back(prismindices[i] + startVertices);
   }

   m_shapedata[type].indiceStart = startIndices;
   m_shapedata[type].indiceCount = (UINT)m_indices.size() - startIndices;
   m_shapedata[type].verticesStart = startVertices;
   m_shapedata[type].verticesCount = (UINT)m_vertices.size() - startVertices;
}
