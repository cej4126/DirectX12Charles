#pragma once

#include "stdafx.h"
#include "Graphics.h"
#include "Vertex.h"
#include "DrawFunction.h"
#include "imgui/imgui.h"
#include "ModelSpec.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Transform.h"

class DrawMesh : public DrawFunction
{
public:
   DrawMesh(Graphics &gfx, int index, std::vector<std::shared_ptr<Bind::Bindable>> bindPtrs, int indicesCount, int& MaterialIndex);
   void Draw(Graphics &gfx, FXMMATRIX acculatedTransform) const noexcept;
   int getMaterialIndex() const noexcept
   {
      return MaterialIndex;
   }
   XMMATRIX GetTransformXM() const noexcept override { return XMLoadFloat4x4(&transform); }

private:
   mutable XMFLOAT4X4 transform;
   int MaterialIndex = 0;
};
