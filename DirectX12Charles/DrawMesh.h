#pragma once

#include "stdafx.h"
#include "Graphics.h"
#include "Vertex.h"
#include "DrawFunction.h"
#include "imgui/imgui.h"
//#include "ModelObject.h"
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

class Node
{
   friend class Model;

public:
   Node(int id, const std::string& name, std::vector<DrawMesh * > MeshPtrs, const DirectX::XMMATRIX &transform) noexcept;
   void Draw(Graphics &gfx, FXMMATRIX accumulatedTrans);
   void SetAppliedTransform(FXMMATRIX transform) noexcept;
   int GetId() const noexcept;
   void ShowTree(Node *&pSelectedNode) const noexcept;

private:
   void AddChild(std::unique_ptr<Node> pChild) noexcept;

   std::string name;
   int id;
   std::vector<std::unique_ptr<Node>> childPtrs;
   std::vector<DrawMesh *> MeshPtrs;
   DirectX::XMFLOAT4X4 transform;
   DirectX::XMFLOAT4X4 appliedTransform;
};

class Model
{
public:
   Model(Graphics &gfx, int &index, float size, const std::string fileName, ID3D12Resource *lightView, int& MaterialIndex);
   ~Model() noexcept;

   void FirstCommand();
   void Draw(Graphics &gfx) const;
   void ShowWindow(const char *windowName = nullptr) noexcept;
   void SetPosition(FXMMATRIX tf);

private:
   std::unique_ptr<DrawMesh> ParseMesh(int index, const aiMesh& mesh, const aiMaterial *const *pMaterials);
   std::unique_ptr<Node> ParseNode(int & nextId, const aiNode &node) noexcept;

   Graphics &gfx;
   ID3D12Device *device;
   ID3D12GraphicsCommandList *commandList;
   ID3D12Resource *lightView;
   Bind::Bindable *object = nullptr;
   float m_size = 1.0f;

   std::unique_ptr<Node> pRoot;
   std::vector<std::unique_ptr<DrawMesh>> MeshPtrs;

   int m_materialIndex = -1;
   Graphics::MaterialType m_material;

   std::vector<std::shared_ptr<Bind::Bindable>> bindablePtrs;

   std::unique_ptr<class ModelWindow> pWindow;
   std::string filename;
};