#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include "stdafx.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "ShapeData.h"

using namespace DirectX;

class Shape
{
public:
   Assimp::Importer m_imp;
   //const aiScene *m_modelSuzanne;

   enum ShapeBaseType
   {
      basic,
      normal,
      texture
   };

   enum shapeType
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
   };

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

   struct ShapeDataType
   {
      UINT verticesStart;
      UINT verticesCount;
      UINT indiceStart;
      UINT indiceCount;
   };

   Shape();
   UINT getIndiceStart(shapeType type) { return m_shapedata[type].indiceStart; }
   UINT getIndiceCount(shapeType type) { return m_shapedata[type].indiceCount; }
   UINT getVerticesStart(shapeType type) { return m_shapedata[type].verticesStart; }
   UINT getVerticesCount(shapeType type) { return m_shapedata[type].verticesCount; }

   template<class V>
   ShapeData<V> getShapeData()
   {
      std::vector<V> verts(m_vertices.size());
      for (size_t i = 0; i < m_vertices.size(); i++)
      {
         verts[i].pos = m_vertices[i].pos;
      }

      return{ verts, m_indices };
   }

   template<class V>
   ShapeData<V> getShapeTextureData()
   {
      std::vector<V> verts(m_vertices.size());
      for (size_t i = 0; i < m_vertices.size(); i++)
      {
         verts[i].pos = m_vertices[i].pos;
         verts[i].tex = m_vertices[i].tex;
      }

      return{ verts, m_indices };
   }


   template<class V>
   ShapeData<V> GetShapeNormalData()
   {
      setNormals();

      std::vector<V> verts(m_vertices.size());
      for (size_t i = 0; i < m_vertices.size(); i++)
      {
         verts[i].pos = m_vertices[i].pos;
         verts[i].normal = m_vertices[i].normal;
      }

      return{ verts, m_indices };
   }

   template<class V>
   ShapeData<V> getShapeTextureNormalData()
   {
      setNormals();

      std::vector<V> verts(m_vertices.size());
      for (size_t i = 0; i < m_vertices.size(); i++)
      {
         verts[i].pos = m_vertices[i].pos;
         verts[i].normal = m_vertices[i].normal;
         verts[i].tex = m_vertices[i].tex;
      }

      return{ verts, m_indices };
   }


private:
   void setNormals();
   void createCone(int longDiv);
   void createCube();
   void createPrism();
   void createPlane(int divisions_x, int divisions_y);
   void createCylinder(int longDiv);
   void createSphere(int latDiv, int longDiv);
   void createTextureCube();
   void createPictureCube();
   void createTextureCylinder(int longDiv);
   void createTextureSuzanne();
   void createTextureCone(int longDiv);
   void createTexturePrism();

   std::array<ShapeDataType, ShapeCount> m_shapedata;
   std::vector<Vertex> m_vertices;
   std::vector<unsigned short> m_indices;

};
