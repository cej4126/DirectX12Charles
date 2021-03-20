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
#include "DrawMesh.h"

class DrawNode
{
//   friend class DrawModel;

public:
   DrawNode(int id, const std::string &name, std::vector<DrawMesh * > MeshPtrs, const DirectX::XMMATRIX &transform) noexcept;
   void Draw(Graphics &gfx, FXMMATRIX accumulatedTrans);
   void SetAppliedTransform(FXMMATRIX transform) noexcept;
   const XMFLOAT4X4 &GetAppliedTransform() const noexcept;
   int GetId() const noexcept;
   void ShowTree(DrawNode *&pSelectedNode) const noexcept;

   void AddChild(std::unique_ptr<DrawNode> pChild) noexcept;
private:

   std::string name;
   int id;
   std::vector<std::unique_ptr<DrawNode>> childPtrs;
   std::vector<DrawMesh *> MeshPtrs;
   DirectX::XMFLOAT4X4 transform;
   DirectX::XMFLOAT4X4 appliedTransform;
};
