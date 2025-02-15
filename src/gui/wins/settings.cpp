#include "gui.h"

void USCGUI::drawSettings() {
  if (!up->settingsOpen) return;
  ImGui::OpenPopup("Settings");
  if (ImGui::BeginPopupModal("Settings",&up->settingsOpen,ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoTitleBar)) {
    ImVec2 viewSize = ImGui::GetMainViewport()->Size,
           viewPos = ImGui::GetMainViewport()->Pos,
           winSize = ImGui::GetWindowSize();
    ImGui::SetWindowPos((viewSize - winSize) / 2.0f + viewPos);
    ImGui::SeparatorText("Settings");
    // ImGui::Separator();
    ImGui::Text("you will have to restart to see the changes");

    ImVec2 childSize = ImGui::GetContentRegionAvail() - ImVec2(0.0f, ImGui::GetFrameHeightWithSpacing());
    if (ImGui::BeginChild("##settingsItems", childSize)) {
      cf->drawSettings();
    }
    ImGui::EndChild();

    if (ImGui::Button("Save")) {
      cf->saveConfig();
      up->settingsOpen = false;
    }
    ImGui::SameLine();
    if (ImGui::Button("Cancel")) {
      cf->loadConfig();
      up->settingsOpen = false;
    }
    ImGui::EndPopup();
  }
}
