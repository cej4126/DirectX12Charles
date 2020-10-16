#pragma once

#include "stdafx.h"
#include "Graphics.h"
#include "Vertex.h"
#include "DrawFunction.h"
#include "imgui/imgui.h"
#include "ModelObject.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "Transform.h"

class Mesh : public DrawBase <Mesh>
{
public:
   Mesh(Graphics &gfx, std::vector<std::unique_ptr<Bindable>> bindPtrs, int indicesCount);
   void Draw(Graphics &gfx, FXMMATRIX acculatedTransform) const noexcept;
   XMMATRIX GetTransformXM() const noexcept override
   {
      return XMLoadFloat4x4(&transform);
   }

private:
   mutable XMFLOAT4X4 transform;
};

class Node
{
   friend class Model;
public:
   Node(std::vector<Mesh * > meshPtrs, const DirectX::XMMATRIX &transform) noexcept
      :
      meshPtrs(std::move(meshPtrs))
   {
      DirectX::XMStoreFloat4x4(&this->transform, transform);
   }
   void Draw(Graphics &gfx, FXMMATRIX accumulatedTrans)
   {
      const auto built = DirectX::XMLoadFloat4x4(&transform) * accumulatedTrans;
      for (const auto pm : meshPtrs)
      {
         pm->Draw(gfx, built);
      }
      for (const auto &pc : childPtrs)
      {
         pc->Draw(gfx, built);
      }
   }

private:
   void AddChild(std::unique_ptr<Node> pChild) noexcept
   {
      childPtrs.push_back(std::move(pChild));
   }
private:
   std::vector<std::unique_ptr<Node>> childPtrs;
   std::vector<Mesh *> meshPtrs;
   DirectX::XMFLOAT4X4 transform;
};

class Model
{
public:
   Model(Graphics &gfx, const std::string fileName, ID3D12Resource *lightView, int MaterialIndex);
   std::unique_ptr<Mesh> ParseMesh(const aiMesh &mesh);
   std::unique_ptr<Node> ParseNode(const aiNode &node);
   void FirstCommand();
   void Draw(Graphics &gfx) const;
   UINT getMaterialIndex() { return MaterialIndex; }

private:
   Graphics &gfx;
   ID3D12Device *device;
   ID3D12GraphicsCommandList *commandList;
   ID3D12Resource *lightView;
   Bindable *object = nullptr;

   std::unique_ptr<Node> pRoot;
   std::vector<std::unique_ptr<Mesh>> meshPtrs;

   int MaterialIndex = -1;
   Graphics::MaterialType material;

   std::vector<std::unique_ptr<Bindable>> bindablePtrs;
};