#pragma once
#include "Graphics.h"
#include "Surface.h"
#include "Vertex.h"

namespace Bind
{
   class Bindable
   {
   public:
      virtual void Bind(Graphics &gfx) noexcept = 0;
      virtual ~Bindable() = default;
      virtual std::string GetUID() const noexcept
      {
         assert(false);
         return "";
      }

      bool isInitialized() { return initialized; }
      void setInitialized() { initialized = true; }
      int getIndex() { return m_index; }
      void setIndex(int index) { m_index = index; }

      virtual void CreateTexture(const Surface &surface, int slot) {}
      virtual void CreateNormal(const Surface &surface, int slot) {}
      virtual void CreateRootSignature(bool constantFlag, bool materialFlag, bool textureFlag) {}
      virtual void CreateShader(const std::wstring &vertexPath, const std::wstring &pixelPath) {}
      virtual void SetLightView(ID3D12Resource *mylightView) {};
      virtual void CreatePipelineState(const std::vector<D3D12_INPUT_ELEMENT_DESC> &inputElementDescs, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType) {}

      virtual void LoadVerticesBuffer(const hw3dexp::VertexBuffer &vertices) {}
      virtual void LoadIndicesBuffer(const std::vector<unsigned short> &indices) {};
      virtual void CreateConstant(const XMFLOAT3 &colorBuffer, int size) {};

   private:
      bool initialized = false;
      int m_index = -1;
   };
}
