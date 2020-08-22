#pragma once
#include "stdafx.h"
#define _USE_MATH_DEFINES
#include <math.h>

using namespace DirectX;

template<class T>
class ShapeData
{
public:
   ShapeData() = default;
   ShapeData(std::vector<T> vert_in, std::vector<unsigned short> indices_in)
      :
      vertices(std::move(vert_in)),
      indices(std::move(indices_in))
   {
      assert(vertices.size() > 2);
      assert(indices.size() % 3 == 0);
   }
   //void Transform(DirectX::FXMMATRIX matrix)
   //{
   //   for (auto &v : vertices)
   //   {
   //      const DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&v.pos);
   //      DirectX::XMStoreFloat3(
   //         &v.pos,
   //         DirectX::XMVector3Transform(pos, matrix)
   //      );
   //   }
   //}

   std::vector<T> vertices;
   std::vector<unsigned short>indices;
};


class Shape
{
public:
   typedef enum
   {
      Cube,
      Cone,
      Prism,
      //Plane,
      Cylinder,
      Sphere,
      ShapeCount
   } shapeType;

   struct Vertex
   {
      XMFLOAT3 pos;
      Vertex(
         float x,
         float y,
         float z)
      {
         pos.x = x;
         pos.y = y;
         pos.z = z;
      }
      Vertex()
      {
         pos.x = 0.0f;
         pos.y = 0.0f;
         pos.z = 0.0f;
      }
   };


   Shape();
   UINT getStartIndex(shapeType type) { return shapeIndiceStart[type];  }
   UINT getStartCount(shapeType type) { return shapeIndiceCount[type]; }

   template<class V>
   ShapeData<V> GetShapeData()
   {
      std::vector<V> verts(vertices.size());
      for (size_t i = 0; i < vertices.size(); i++)
      {
         verts[i].pos = vertices[i].pos;
      }

//      return{ std::move(verts), std::move(indices) };
      return{ verts, indices };
   }

private:
   void CreateCone(int longDiv);
   void CreateCube();
   void CratePrism();
   void CreatePlane(int divisions_x, int divisions_y);
   void CreateCylinder(int longDiv);
   void CreateSphere(int latDiv, int longDiv);


   std::array<UINT, ShapeCount> shapeIndiceStart;
   std::array<UINT, ShapeCount> shapeIndiceCount;
   std::vector<Vertex> vertices;
   std::vector<unsigned short>indices;

};
