#pragma once
#include "stdafx.h"
#define _USE_MATH_DEFINES
#include <math.h>

using namespace DirectX;

template<class T>
class Geometry
{
public:
   Geometry() = default;
   Geometry(std::vector<T> vert_in, std::vector<unsigned short> indices_in)
      :
      vectices(std::move(vert_in)),
      indices(std::move(indices_in))
   {
      assert(vectices.size() > 2);
      assert(indices.size() % 3 == 0);
   }
   void Transform(DirectX::FXMMATRIX matrix)
   {
      for (auto &v : vectices)
      {
         const DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&v.pos);
         DirectX::XMStoreFloat3(
            &v.pos,
            DirectX::XMVector3Transform(pos, matrix)
         );
      }
   }

   std::vector<T> vectices;
   std::vector<unsigned short>indices;
};


class Cone
{
public:
	template<class V>
	static Geometry<V> MakeTesselated(int longDiv)
	{
		assert(longDiv >= 3);

		const auto base = XMVectorSet(1.0f, 0.0f, -1.0f, 0.0f);
		const float longitudeAngle = 2.0f * (float)M_PI / longDiv;

		// base vertices
		std::vector<V> vertices;
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
		const auto iCenter = (unsigned short)(vertices.size() - 1);
		// the tip :darkness:
		vertices.emplace_back();
		vertices.back().pos = { 0.0f,0.0f,1.0f };
		const auto iTip = (unsigned short)(vertices.size() - 1);


		// base indices
		std::vector<unsigned short> indices;
		for (unsigned short iLong = 0; iLong < longDiv; iLong++)
		{
			indices.push_back(iCenter);
			indices.push_back((iLong + 1) % longDiv);
			indices.push_back(iLong);
		}

		// cone indices
		for (unsigned short iLong = 0; iLong < longDiv; iLong++)
		{
			indices.push_back(iLong);
			indices.push_back((iLong + 1) % longDiv);
			indices.push_back(iTip);
		}

		return { std::move(vertices),std::move(indices) };
	}
	template<class V>
	static Geometry<V> Make()
	{
		return MakeTesselated<V>(24);
	}
};

class Cube
{
public:
	template<class V>
	static Geometry<V> Make()
	{
		constexpr float side = 1.0f / 2.0f;

		std::vector<XMFLOAT3> vertices;
		vertices.emplace_back(-side, -side, -side); // 0
		vertices.emplace_back(side, -side, -side); // 1
		vertices.emplace_back(-side, side, -side); // 2
		vertices.emplace_back(side, side, -side); // 3
		vertices.emplace_back(-side, -side, side); // 4
		vertices.emplace_back(side, -side, side); // 5
		vertices.emplace_back(-side, side, side); // 6
		vertices.emplace_back(side, side, side); // 7

		std::vector<V> verts(vertices.size());
		for (size_t i = 0; i < vertices.size(); i++)
		{
			verts[i].pos = vertices[i];
		}
		return{
			std::move(verts),{
				0,2,1, 2,3,1,
				1,3,5, 3,7,5,
				2,6,3, 3,6,7,
				4,5,7, 4,7,6,
				0,4,2, 2,4,6,
				0,1,4, 1,5,4
			}
		};
	}
};

class Prism
{
public:
	template<class V>
	static Geometry<V> Make()
	{
		constexpr float edge = 1.0f;
		const float halfhight = (float)sqrt(2.0) * edge / 2.0f /2.0f;
		constexpr float side = edge / 2.0f;

		std::vector<XMFLOAT3> vertices;
		vertices.emplace_back(-side, -halfhight, side); // 0
		vertices.emplace_back(0.0f, halfhight, side); // 1
		vertices.emplace_back( side, -halfhight, side); // 2
		vertices.emplace_back(-side, -halfhight, -side); // 3
		vertices.emplace_back(0.0f, halfhight, -side); // 4
		vertices.emplace_back(side, -halfhight, -side); // 5

		std::vector<V> verts(vertices.size());
		for (size_t i = 0; i < vertices.size(); i++)
		{
			verts[i].pos = vertices[i];
		}
		return{
			std::move(verts),{
				3,4,5, // 5,4,3,          // front face
				1,0,2, //2,0,1,          // back face
				0,1,3, 3,1,4, // left face
				4,1,5, 1,2,5, // 5,1,4, 5,2,1, // right face
				0,3,2, 3,5,2  // bottom face
			}
		};
	}
};

class Plane
{
public:
	template<class V>
	static Geometry<V> MakeTesselated(int divisions_x, int divisions_y)
	{
		assert(divisions_x >= 1);
		assert(divisions_y >= 1);

		constexpr float width = 2.0f;
		constexpr float height = 2.0f;
		const int nVertices_x = divisions_x + 1;
		const int nVertices_y = divisions_y + 1;
		std::vector<V> vertices(nVertices_x * nVertices_y);

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

		std::vector<unsigned short> indices;
		indices.reserve(sqrt(divisions_x * divisions_y) * 6);
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
					{ vxy2i(x,y),vxy2i(x + 1,y),vxy2i(x,y + 1),vxy2i(x + 1,y + 1) };
					indices.push_back(indexArray[0]);
					indices.push_back(indexArray[2]);
					indices.push_back(indexArray[1]);
					indices.push_back(indexArray[1]);
					indices.push_back(indexArray[2]);
					indices.push_back(indexArray[3]);
				}
			}
		}

		return{ std::move(vertices),std::move(indices) };
	}
	template<class V>
	static Geometry<V> Make()
	{
		return MakeTesselated<V>(1, 1);
	}
};

class Cylinder
{
public:
	template<class V>
	static Geometry<V> MakeTesselated(int longDiv)
	{
		assert(longDiv >= 3);

		const auto base = XMVectorSet(1.0f, 0.0f, -1.0f, 0.0f);
		const auto offset = XMVectorSet(0.0f, 0.0f, 2.0f, 0.0f);
		const float longitudeAngle = 2.0f * (float)M_PI / longDiv;

		// near center
		std::vector<V> vertices;
		vertices.emplace_back();
		vertices.back().pos = { 0.0f,0.0f,-1.0f };
		const auto iCenterNear = (unsigned short)(vertices.size() - 1);
		// far center
		vertices.emplace_back();
		vertices.back().pos = { 0.0f,0.0f,1.0f };
		const auto iCenterFar = (unsigned short)(vertices.size() - 1);

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
		std::vector<unsigned short> indices;
		for (unsigned short iLong = 0; iLong < longDiv; iLong++)
		{
			const auto i = iLong * 2;
			const auto mod = longDiv * 2;
			indices.push_back(i + 2);
			indices.push_back((i + 2) % mod + 2);
			indices.push_back(i + 1 + 2);
			indices.push_back((i + 2) % mod + 2);
			indices.push_back((i + 3) % mod + 2);
			indices.push_back(i + 1 + 2);
		}

		// base indices
		for (unsigned short iLong = 0; iLong < longDiv; iLong++)
		{
			const auto i = iLong * 2;
			const auto mod = longDiv * 2;
			indices.push_back(i + 2);
			indices.push_back(iCenterNear);
			indices.push_back((i + 2) % mod + 2);
			indices.push_back(iCenterFar);
			indices.push_back(i + 1 + 2);
			indices.push_back((i + 3) % mod + 2);
		}

		return { std::move(vertices),std::move(indices) };
	}
	template<class V>
	static Geometry<V> Make()
	{
		return MakeTesselated<V>(24);
	}
};

class Sphere
{
public:
	template<class V>
	static Geometry<V> MakeTesselated(int latDiv, int longDiv)
	{
		assert(latDiv >= 3);
		assert(longDiv >= 3);

		constexpr float radius = 1.0f;
		const auto base = XMVectorSet(0.0f, 0.0f, radius, 0.0f);
		const float lattitudeAngle = (float)M_PI / latDiv;
		const float longitudeAngle = 2.0f * (float)M_PI / longDiv;

		std::vector<V> vertices;
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
		const auto iNorthPole = (unsigned short)vertices.size();
		vertices.emplace_back();
		XMStoreFloat3(&vertices.back().pos, base);
		const auto iSouthPole = (unsigned short)vertices.size();
		vertices.emplace_back();
		XMStoreFloat3(&vertices.back().pos, XMVectorNegate(base));

		const auto calcIdx = [latDiv, longDiv](unsigned short iLat, unsigned short iLong)
		{ return iLat * longDiv + iLong; };
		std::vector<unsigned short> indices;
		for (unsigned short iLat = 0; iLat < latDiv - 2; iLat++)
		{
			for (unsigned short iLong = 0; iLong < longDiv - 1; iLong++)
			{
				indices.push_back(calcIdx(iLat, iLong));
				indices.push_back(calcIdx(iLat + 1, iLong));
				indices.push_back(calcIdx(iLat, iLong + 1));
				indices.push_back(calcIdx(iLat, iLong + 1));
				indices.push_back(calcIdx(iLat + 1, iLong));
				indices.push_back(calcIdx(iLat + 1, iLong + 1));
			}
			// wrap band
			indices.push_back(calcIdx(iLat, longDiv - 1));
			indices.push_back(calcIdx(iLat + 1, longDiv - 1));
			indices.push_back(calcIdx(iLat, 0));
			indices.push_back(calcIdx(iLat, 0));
			indices.push_back(calcIdx(iLat + 1, longDiv - 1));
			indices.push_back(calcIdx(iLat + 1, 0));
		}

		// cap fans
		for (unsigned short iLong = 0; iLong < longDiv - 1; iLong++)
		{
			// north
			indices.push_back(iNorthPole);
			indices.push_back(calcIdx(0, iLong));
			indices.push_back(calcIdx(0, iLong + 1));
			// south
			indices.push_back(calcIdx(latDiv - 2, iLong + 1));
			indices.push_back(calcIdx(latDiv - 2, iLong));
			indices.push_back(iSouthPole);
		}
		// wrap triangles
		// north
		indices.push_back(iNorthPole);
		indices.push_back(calcIdx(0, longDiv - 1));
		indices.push_back(calcIdx(0, 0));
		// south
		indices.push_back(calcIdx(latDiv - 2, 0));
		indices.push_back(calcIdx(latDiv - 2, longDiv - 1));
		indices.push_back(iSouthPole);

		return { std::move(vertices),std::move(indices) };
	}
	template<class V>
	static Geometry<V> Make()
	{
		return MakeTesselated<V>(12, 24);
	}
};