#pragma once
#define _USE_MATH_DEFINES
#include <math.h>
#include "stdafx.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "ShapeData.h"

using namespace DirectX;

//template<class T>
//class ShapeData
//{
//public:
//   ShapeData() = default;
//   ShapeData(std::vector<T> vert_in, std::vector<unsigned short> indices_in)
//      :
//      vertices(std::move(vert_in)),
//      indices(std::move(indices_in))
//   {
//      assert(vertices.size() > 2);
//      assert(indices.size() % 3 == 0);
//   }
//
//   std::vector<T> vertices;
//   std::vector<unsigned short>indices;
//};

class ShapeAssimp
{
public:
   Assimp::Importer m_imp;
   //const aiScene *modelSuzanne;

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

   ShapeAssimp();
   UINT getIndiceStart() { return m_indiceStart; }
   UINT getIndiceCount() { return m_indiceCount; }
   UINT getVerticesStart() { return m_verticesStart; }
   UINT getVerticesCount() { return m_verticesCount; }

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
   ShapeData<V> getShapeNormalData()
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

private:
   void setNormals();
   void createTextureSuzanne();

   UINT m_verticesStart;
   UINT m_verticesCount;
   UINT m_indiceStart;
   UINT m_indiceCount;
   std::vector<Vertex> m_vertices;
   std::vector<unsigned short>m_indices;

};
