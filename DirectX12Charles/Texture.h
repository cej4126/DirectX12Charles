#pragma once
#include "stdafx.h"
#include "Bindable.h"
#include "Graphics.h"
#include "Surface.h"

class Texture : public Bind::Bindable
{
public:
   Texture(Graphics &gfx, std::string tag);

   static std::shared_ptr<Texture> Resolve(Graphics &gfx, const std::string &tag);
   static std::string GenerateUID(const std::string &path);
   std::string GetUID() const noexcept override;

   void Bind(Graphics &gfx) noexcept override;
   void CreateTexture(std::string path, int slot, int rootPara);
   bool getAlphaGloss() { return m_alphaGloss; }

private:
   Graphics &gfx;
   ID3D12Device *device;
   ID3D12GraphicsCommandList *commandList;
   static const int NUMBER_OF_VIEW = 3;

   std::string tag;
   int m_rootPara = -1;
   bool m_alphaGloss = false;
   Microsoft::WRL::ComPtr < ID3D12Resource > textureBuffer[NUMBER_OF_VIEW];
   Microsoft::WRL::ComPtr < ID3D12DescriptorHeap >mainDescriptorHeap;
   Microsoft::WRL::ComPtr < ID3D12Resource > textureBufferUploadHeap[NUMBER_OF_VIEW];

   Microsoft::WRL::ComPtr <ID3D12RootSignature> rootSignature;
};

