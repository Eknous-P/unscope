/*
unscope - an audio oscilloscope
Copyright (C) 2025-2026 Eknous

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

void USCGUI::drawAbout(bool* open) {
  if (!*open) return;
  ImGui::OpenPopup("About");
  const ImVec2 size = ImVec2(600.f, 300.f);
  ImGui::SetNextWindowSize(size);
  // ImGui::SetNextWindowPos( (ImVec2(windowWidth,windowHeight)-size)/2.0f);
  if (ImGui::BeginPopupModal("About",open,ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize|ImGuiWindowFlags_NoSavedSettings)) {
    ImGui::SetCursorPosX((ImGui::GetWindowWidth()-ImGui::CalcTextSize(PROGRAM_NAME " " PROGRAM_VER).x)/2.0f);
    ImGui::TextUnformatted(PROGRAM_NAME_AND_VER);

    if (ImGui::BeginChild("##aboutChild")) {
      if (ImGui::BeginTabBar("aboutTabs")) {
        if (ImGui::BeginTabItem("About")) {
          if (ImGui::BeginTable("aboutTable", 2)) {
            ImGui::TableSetupColumn("txt", ImGuiTableColumnFlags_WidthStretch);
            ImGui::TableSetupColumn("img", ImGuiTableColumnFlags_WidthFixed);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::TextUnformatted(aboutMsg);
            ImGui::TableNextColumn();
            ImGui::Dummy({200,200});
            ImGui::EndTable();
          }
          ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("3rd party")) {
          ImGui::TextUnformatted("libraies:");
          ImGui::Indent();
#define _A(n,u) \
  ImGui::TextUnformatted( #n " -"); \
  ImGui::SameLine(); \
  ImGui::TextLinkOpenURL("https://" #u);

          _A(ImGui, github.com/ocornut/imgui)
          _A(PortAudio, github.com/PortAudio/portaudio)
          _A(imgui-knobs, github.com/altschuler/imgui-knobs)
          _A(imgui_toggle, github.com/cmdwtf/imgui_toggle)
#ifdef NON_SYS_SDL
          _A(SDL, github.com/libsdl-org/SDL)
#endif
          _A(pffft, github.com/marton78/pffft)
          _A(yaml-cpp, github.com/jbeder/yaml-cpp)
          ImGui::Unindent();
          ImGui::TextUnformatted("icons:");
          ImGui::Indent();
          _A(Fontaudio, github.com/fefanto/fontaudio)
          ImGui::Text("with"); ImGui::SameLine();
          _A(IconFontCppHeaders,github.com/juliettef/IconFontCppHeaders)
          ImGui::Unindent();
          ImGui::EndTabItem();
#undef _A
        }
        if (ImGui::BeginTabItem("License")) {
          ImGui::TextUnformatted(licenseMsg);
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
