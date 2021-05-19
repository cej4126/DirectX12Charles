#pragma once
#include "stdafx.h"

template<class T>
class ShapeData
{
public:
   ShapeData() = default;
   ShapeData(std::vector<T> vert_in, std::vector<unsigned short> indices_in)
      :
      m_vertices(std::move(vert_in)),
      m_indices(std::move(indices_in))
   {
      assert(m_vertices.size() > 2);
      assert(m_indices.size() % 3 == 0);
   }

   std::vector<T> m_vertices;
   std::vector<unsigned short> m_indices;
};
