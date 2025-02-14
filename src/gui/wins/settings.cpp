#include "gui.h"

void USCGUI::drawSettings() {
  if (!up->settingsOpen) return;
  ImGui::OpenPopup("Settings");
  if (ImGui::BeginPopupModal("Settings",&up->settingsOpen,ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoTitleBar)) {
    ImVec2 viewSize = ImGui::GetMainViewport()->Size,
           winSize = ImGui::GetWindowSize();
    ImGui::SetWindowPos(ImVec2(
      (viewSize.x - winSize.x)/2.0f,(viewSize.y - winSize.y)/2.0f
    ));
    ImGui::SeparatorText("Settings");
    // ImGui::Separator();

    cf->drawSettings();

    if (ImGui::Button("Save")) {
      cf->saveConfig();
      up->settingsOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) up->settingsOpen = false;
    ImGui::EndPopup();
  }
}
