#include "gui.h"

void USCGUI::drawAbout() {
  if (!wo.aboutOpen) return;
  ImGui::OpenPopup("About");
  if (ImGui::BeginPopupModal("About",&wo.aboutOpen,ImGuiWindowFlags_NoTitleBar|ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize)) {
    ImGui::SetCursorPosX((ImGui::GetWindowWidth()-ImGui::CalcTextSize(PROGRAM_NAME " " PROGRAM_VER).x)/2.0f);
    ImGui::Text(PROGRAM_NAME " " PROGRAM_VER);
    ImGui::Separator();
    ImGui::Text("%s",aboutMsg);
    if (ImGui::Button("OK")) wo.aboutOpen=false;

    ImGui::EndPopup();
  }
}