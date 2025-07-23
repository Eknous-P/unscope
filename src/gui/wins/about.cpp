/*
unscope - an audio oscilloscope
Copyright (C) 2025 Eknous

unscope is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 2 of the License, or (at your option) any later
version.

unscope is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
unscope. If not, see <https://www.gnu.org/licenses/>. 
*/

#include "gui.h"
#include <imgui.h>

void USCGUI::drawAbout() {
  if (!wo.aboutOpen) return;
  ImGui::OpenPopup("About");
  ImGui::SetNextWindowSize(ImVec2(600.f, 270.f));
  if (ImGui::BeginPopupModal("About",&wo.aboutOpen,ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize)) {
    ImGui::SetCursorPosX((ImGui::GetWindowWidth()-ImGui::CalcTextSize(PROGRAM_NAME " " PROGRAM_VER).x)/2.0f);
    ImGui::Text(PROGRAM_NAME_AND_VER);

    if (ImGui::BeginChild("##aboutChild")) {
      if (ImGui::BeginTabBar("aboutTabs")) {
        if (ImGui::BeginTabItem("About")) {
          ImGui::Text("%s",aboutMsg);
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("3rd party")) {
          ImGui::Text("%s",thirdPartyMsg);
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("License")) {
          ImGui::Text("%s",licenseMsg);
          ImGui::EndTabItem();
        }
        ImGui::EndTabBar();
      }
    }
    ImGui::EndChild();

    // ImGui::SetCursorPosY(270.f - ImGui::GetStyle().FramePadding.y * 5.f - ImGui::CalcTextSize("OK").y);
    // if (ImGui::Button("OK##aboutOK")) wo.aboutOpen=false;

    ImGui::EndPopup();
  }
}
