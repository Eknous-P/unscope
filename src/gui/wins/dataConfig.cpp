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
#include <data.h>
#include <imgui.h>

void USCGUI::drawAudioConfig(bool* open) {
  if (!*open) return;
  if (ImGui::Begin("Data I/O controls",open)) {
    if (ImGui::BeginTable("driverList", 2)) {
      ImGui::TableSetupColumn("c1", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("c2", ImGuiTableColumnFlags_WidthStretch);
      for (int i=0; i<data->getDriverCount(); i++) {
        DataDriver* drv = data->getDriver(i);
        ImGui::PushID(i);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        // remove button
        if (ImGui::Button("×")) {
          data->removeDriver(i);
          ImGui::PopID();
          continue;
        }
        ImGui::TableNextColumn();
        if (ImGui::BeginChild("driverCtrls", ImVec2(ImGui::GetContentRegionAvail().x, 200.0f), ImGuiChildFlags_Borders)) {
          if (ImGui::BeginMenuBar()) {
            if (ImGui::BeginMenu(drv->getName())) {
              ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
          }
          for (int j=0; j<drv->getParams().size(); j++) {
            drv->getParams()[j].draw();
          }
          if (ImGui::Button("refresh device list")) data->dispatchDriverCommand(i, DRIVER_ENUMERATE_DEVICES);
          ImGui::SameLine();
          if (ImGui::Button("restart")) data->dispatchDriverCommand(i, DRIVER_RESTART);
        }
        ImGui::EndChild();
        ImGui::PopID();
      }
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Button("+");
      if (ImGui::BeginPopupContextItem("new driver", ImGuiPopupFlags_MouseButtonLeft)) {
        if (ImGui::BeginCombo("Select Driver...##driverSelect", dataDriverNames[newDriver])) {
          char comboStrBuf[1024];
          int m=0;
          for (int i=1; i<DATA_MAX; i++) {
            if (ImGui::Selectable(dataDriverNames[i])) {
              newDriver = (DataDrivers)i;
            }
          }
          ImGui::EndCombo();
        }
        if (ImGui::Button("Add Driver")) {
          data->addDriver(newDriver);
          newDriver = DATA_PORTAUDIO;
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
      ImGui::EndTable();
    }
  }
  ImGui::End();
}
