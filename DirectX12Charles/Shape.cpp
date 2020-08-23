#include "Shape.h"
#pragma once
#include "stdafx.h"
#define _USE_MATH_DEFINES
#include <math.h>

Shape::Shape()
{
   CreateCube();
   CreateCone(24);
   CratePrism();
   CreatePlane(1, 1);
   CreateCylinder(24);
   CreateSphere(12, 24);
}

void Shape::CreateCube()
{
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

   shapeIndiceStart[Cube] = startIndices;
   shapeIndiceCount[Cube] = (UINT)indices.size() - startIndices;
}

void Shape::CreateCone(int longDiv)
{
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

   shapeIndiceStart[Cone] = startIndices;
   shapeIndiceCount[Cone] = (UINT)indices.size() - startIndices;
}

void Shape::CratePrism()
{
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

   shapeIndiceStart[Prism] = startIndices;
   shapeIndiceCount[Prism] = (UINT)indices.size() - startIndices;
}

void Shape::CreatePlane(int divisions_x, int divisions_y)
{
   assert(divisions_x >= 1);
   assert(divisions_y >= 1);

   UINT startIndices = (UINT)indices.size();
   UINT startVertices = (UINT)vertices.size();

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
            XMStoreFloat3(&vertices[i].pos, v);
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

   shapeIndiceStart[Plane] = startIndices;
   shapeIndiceCount[Plane] = (UINT)indices.size() - startIndices;
}

void Shape::CreateCylinder(int longDiv)
{
   assert(longDiv >= 3);

   UINT startIndices = (UINT)indices.size();
   UINT startVertices = (UINT)vertices.size();

   const auto base = XMVectorSet(1.0f, 0.0f, -1.0f, 0.0f);
   const auto offset = XMVectorSet(0.0f, 0.0f, 2.0f, 0.0f);
   const float longitudeAngle = 2.0f * (float)M_PI / longDiv;

   // near center
   vertices.emplace_back();
   vertices.back().pos = { 0.0f,0.0f,-1.0f };
   const auto iCenterNear = (unsigned short)(vertices.size() - startVertices - 1);
   // far center
   vertices.emplace_back();
   vertices.back().pos = { 0.0f,0.0f,1.0f };
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

   shapeIndiceStart[Cylinder] = startIndices;
   shapeIndiceCount[Cylinder] = (UINT)indices.size() - startIndices;
}

void Shape::CreateSphere(int latDiv, int longDiv)
{
   assert(latDiv >= 3);
   assert(longDiv >= 3);

   UINT startIndices = (UINT)indices.size();
   UINT startVertices = (UINT)vertices.size();

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

   shapeIndiceStart[Sphere] = startIndices;
   shapeIndiceCount[Sphere] = (UINT)indices.size() - startIndices;
}
