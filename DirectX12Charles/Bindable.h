#pragma once
#include "Graphics.h"
#include "Surface.h"
#include "Vertex.h"

namespace Bind
{
   class Bindable
   {
   public:
      virtual void Bind(Graphics &gfx, int drawStep) noexcept = 0;
      virtual ~Bindable() = default;
      virtual std::string GetUID() const noexcept
      {
         assert(false);
         return "";
      }

      bool isInitialized() { return initialized; }
      void setInitialized() { initialized = true; }

      virtual void CreateTexture(const Surface &surface, int slot) {}
      virtual void CreateRootSignature(bool constantFlag, bool materialFlag, bool textureFlag) {}
      virtual void CreateShader(const std::wstring &vertexPath, const std::wstring &pixelPath) {}
      virtual void SetLightView(ID3D12Resource *mylightView) {};
      virtual void CreatePipelineState(const std::vector<D3D12_INPUT_ELEMENT_DESC> &inputElementDescs, D3D12_PRIMITIVE_TOPOLOGY_TYPE topologyType) {}

      virtual void LoadVerticesBufferTest(const hw3dexp::VertexBuffer& vertices) {}
      virtual void LoadIndicesBuffer(const std::vector<unsigned short> &indices) {};
      virtual void CreateConstant(const XMFLOAT3 &colorBuffer) {};

   private:
      bool initialized = false;
   };
}
