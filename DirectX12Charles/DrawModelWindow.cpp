#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "DrawModelWindow.h"
#include "DrawNode.h"

XMFLOAT3 DrawModelWindow::ExtractEulerAngles(const XMFLOAT4X4 &mat)
{
   DirectX::XMFLOAT3 euler;

   euler.x = asinf(-mat._32);                  // Pitch
   if (cosf(euler.x) > 0.0001)                // Not at poles
   {
      euler.y = atan2f(mat._31, mat._33);      // Yaw
      euler.z = atan2f(mat._12, mat._22);      // Roll
   }
   else
   {
      euler.y = 0.0f;                           // Yaw
      euler.z = atan2f(-mat._21, mat._11);     // Roll
   }

   return euler;
}

XMFLOAT3 DrawModelWindow::ExtractTranslation(const XMFLOAT4X4 &matrix)
{
   return { matrix._41,matrix._42,matrix._43 };
}

void DrawModelWindow::Show(const char *windowName, const DrawNode &root) noexcept
{
   windowName = windowName ? windowName : "Model";

   // need an ints to track node indices and selected node
   int nodeIndexTracker = 0;

   if (ImGui::Begin(windowName))
   {
      ImGui::Columns(2, nullptr, true);
      root.ShowTree(pSelectedNode);

      ImGui::NextColumn();
      if (pSelectedNode != nullptr)
      {
         const auto id = pSelectedNode->GetId();
         auto i = transforms.find(id);
         if (i == transforms.end())
         {
            const auto &applied = pSelectedNode->GetAppliedTransform();
            const auto angle = ExtractEulerAngles(applied);
            const auto translation = ExtractTranslation(applied);

            TransformParameters tp = { angle.z, angle.x, angle.y, translation.x, translation.y, translation.z };
            std::tie(i, std::ignore) = transforms.insert({ id, tp });
         }

         auto &tranform = transforms[pSelectedNode->GetId()];
         ImGui::Text("Orientation");
         ImGui::SliderAngle("Roll", &tranform.roll, -180.0f, 180.0f);
         ImGui::SliderAngle("Pitch", &tranform.pitch, -180.0f, 180.0f);
         ImGui::SliderAngle("Yaw", &tranform.yaw, -180.0f, 180.0f);

         ImGui::Text("Position");
         ImGui::SliderFloat("X", &tranform.x, -20.0f, 20.0f);
         ImGui::SliderFloat("Y", &tranform.y, -20.0f, 20.0f);
         ImGui::SliderFloat("Z", &tranform.z, -20.0f, 20.0f);
      }
   }
   ImGui::End();
}


XMMATRIX DrawModelWindow::GetTransform() const noexcept
{
   assert(pSelectedNode != nullptr);
   const auto &transform = transforms.at(pSelectedNode->GetId());
   return XMMatrixRotationRollPitchYaw(transform.pitch, transform.yaw, transform.roll) *
      XMMatrixTranslation(transform.x, transform.y, transform.z);
}

DrawNode *DrawModelWindow::GetSelectedNode() const noexcept
{
   return pSelectedNode;
}
