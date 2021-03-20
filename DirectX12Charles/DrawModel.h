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
#include "DrawNode.h"
#include "DrawModelWindow.h"

class DrawModel
{
public:
   DrawModel(Graphics &gfx, int &index, float size, const std::string fileName, ID3D12Resource *lightView, int &MaterialIndex);
   ~DrawModel() noexcept;

   void FirstCommand();
   void Draw(Graphics &gfx) const;
   void ShowWindow(const char *windowName = nullptr) noexcept;
   void SetPosition(FXMMATRIX tf);

private:
   std::unique_ptr<DrawMesh> ParseMesh(int index, const aiMesh &mesh, const aiMaterial *const *pMaterials);
   std::unique_ptr<DrawNode> ParseNode(int &nextId, const aiNode &node) noexcept;

   Graphics &gfx;
   ID3D12Device *device;
   ID3D12GraphicsCommandList *commandList;
   ID3D12Resource *lightView;
   Bind::Bindable *object = nullptr;
   float m_size = 1.0f;

   std::unique_ptr<DrawNode> pRoot;
   std::vector<std::unique_ptr<DrawMesh>> MeshPtrs;

   int m_materialIndex = -1;
   std::vector<Graphics::MaterialType> m_material;

   std::vector<std::shared_ptr<Bind::Bindable>> bindablePtrs;

   std::unique_ptr<class DrawModelWindow> pWindow;
   std::string filename;
};