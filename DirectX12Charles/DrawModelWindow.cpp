#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>
#include "DrawModelWindow.h"
#include "DrawNode.h"

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
