#pragma once

#include "stdafx.h"
#include "DrawNode.h"

class DrawModelWindow
{
public:

   void Show(const char *windowName, const DrawNode &root) noexcept;
   XMMATRIX GetTransform() const noexcept;
   DrawNode *GetSelectedNode() const noexcept;

private:
   static constexpr float PI = 3.14159265f;
   DrawNode *pSelectedNode;
   struct TransformParameters
   {
      float roll = 0.0f;
      float pitch = 0.0f;
      float yaw = 0.0f;
      float x = 0.0f;
      float y = 0.0f;
      float z = 0.0f;
   };
   std::unordered_map<int, TransformParameters> transforms;
};
