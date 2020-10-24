#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include "stdafx.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

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

   std::vector<T> vertices;
   std::vector<unsigned short>indices;
};


class Shape
{
public:
   Assimp::Importer imp;
   const aiScene *modelSuzanne;

   typedef enum
   {
      basic,
      normal,
      texture
   } ShapeBaseType;

   typedef enum
   {
      PictureCube,
      TextureCube,
      TextureCylinder,
      TextureCone,
      TexturePrism,
      TextureSuzanne,
      Cube,
      Cone,
      Prism,
      Cylinder,
      Sphere,
      Plane,

      ShapeCount
   } shapeType;
   //typedef enum
   //{
   //   Cube,
   //   Cone,
   //   Prism,
   //   Cylinder,
   //   Sphere,
   //   Plane,
   //   TextureCube,
   //   TextureCylinder,
   //   ShapeCount
   //} shapeType;

   struct Vertex
   {
      XMFLOAT3 pos = { 0.0f, 0.0f, 0.0f };
      XMFLOAT3 normal = { 0.0f, 0.0f, 0.0f };
      XMFLOAT2 tex = { 0.0f, 0.0f };
      Vertex(
         float x,
         float y,
         float z)
      {
         pos.x = x;
         pos.y = y;
         pos.z = z;
      }
      Vertex(
         float x,
         float y,
         float z,
         float nx,
         float ny,
         float nz)
      {
         pos.x = x;
         pos.y = y;
         pos.z = z;
         normal.x = nx;
         normal.y = ny;
         normal.z = nz;
      }
      Vertex(
         float x,
         float y,
         float z,
         float tu,
         float tv)
      {
         pos.x = x;
         pos.y = y;
         pos.z = z;
         tex.x = tu;
         tex.y = tv;
      }
      Vertex(
         XMVECTOR tpos,
         float tu,
         float tv)
      {
         XMStoreFloat3(&pos, tpos);
         tex.x = tu;
         tex.y = tv;
      }
      Vertex()
      {
         pos.x = 0.0f;
         pos.y = 0.0f;
         pos.z = 0.0f;
      }
   };

   typedef struct
   {
      UINT verticesStart;
      UINT verticesCount;
      UINT indiceStart;
      UINT indiceCount;
   } ShapeDataType;

   Shape();
   UINT getIndiceStart(shapeType type)
   {
      return shapedata[type].indiceStart;
   }
   UINT getIndiceCount(shapeType type) { return shapedata[type].indiceCount; }
   UINT getVerticesStart(shapeType type) { return shapedata[type].verticesStart; }
   UINT getVerticesCount(shapeType type) { return shapedata[type].verticesCount; }

   template<class V>
   ShapeData<V> GetShapeData()
   {
      std::vector<V> verts(vertices.size());
      for (size_t i = 0; i < vertices.size(); i++)
      {
         verts[i].pos = vertices[i].pos;
      }

      return{ verts, indices };
   }

   template<class V>
   ShapeData<V> GetShapeTextureData()
   {
      std::vector<V> verts(vertices.size());
      for (size_t i = 0; i < vertices.size(); i++)
      {
         verts[i].pos = vertices[i].pos;
         verts[i].tex = vertices[i].tex;
      }

      return{ verts, indices };
   }


   template<class V>
   ShapeData<V> GetShapeNormalData()
   {
      SetNormals();

      std::vector<V> verts(vertices.size());
      for (size_t i = 0; i < vertices.size(); i++)
      {
         verts[i].pos = vertices[i].pos;
         verts[i].normal = vertices[i].normal;
      }

      return{ verts, indices };
   }

private:
   void SetNormals();
   void CreateCone(int longDiv);
   void CreateCube();
   void CratePrism();
   void CreatePlane(int divisions_x, int divisions_y);
   void CreateCylinder(int longDiv);
   void CreateSphere(int latDiv, int longDiv);
   void CreateTextureCube();
   void CreatePictureCube();
   void CreateTextureCylinder(int longDiv);
   void CreateTextureSuzanne();
   void CreateTextureCone(int longDiv);
   void CreateTexturePrism();

   std::array<ShapeDataType, ShapeCount> shapedata;
   std::vector<Vertex> vertices;
   std::vector<unsigned short>indices;

};
