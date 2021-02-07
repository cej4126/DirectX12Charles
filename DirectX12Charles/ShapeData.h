#pragma once
#include "stdafx.h"

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
