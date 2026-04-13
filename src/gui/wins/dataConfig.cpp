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
#include <IconsFontaudio.h>

void USCGUI::drawDataConfig(bool* open) {
  if (!*open) return;
  if (ImGui::Begin("Data I/O Configuration",open)) {
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
        if (ImGui::BeginChild("driverCtrls", ImVec2(), ImGuiChildFlags_Borders)) {
          ImGui::SeparatorText(drv->getDriverInfo().name);
          for (int j=0; j<drv->getParams()->size(); j++) {
            drv->getParams()->at(j).draw();
          }
          if (ImGui::Button("refresh device list"))
            if (data->dispatchDriverCommand(i, DRIVER_ENUMERATE_DEVICES)) {
              printf(ERROR_MSG "failed to get any devices!" MSG_END);
            }
          ImGui::BeginDisabled(drv->getState()&DRIVERSTATE_ACTIVE);
          if (ImGui::Button("open device"))
            data->dispatchDriverCommand(i, DRIVER_ACTIVATE);
          ImGui::EndDisabled();
          ImGui::SameLine();
  
          ImGui::BeginDisabled(!(drv->getState()&DRIVERSTATE_ACTIVE));
          bool playing = drv->getState()&DRIVERSTATE_PLAY;
          if (playing) ImGui::PushStyleColor(ImGuiCol_Button, colors.widgetActiveColor);
          if (ImGui::Button(playing?ICON_FAD_PAUSE:ICON_FAD_PLAY))
            data->dispatchDriverCommand(i, playing?DRIVER_PAUSE:DRIVER_PLAY);
          if (playing) ImGui::PopStyleColor();
          
          ImGui::SameLine();

          if (ImGui::Button(ICON_FAD_STOP)) {
            data->dispatchDriverCommand(i, DRIVER_DEACTIVATE);
          }
          ImGui::EndDisabled();
        }
        ImGui::EndChild();
        ImGui::PopID();
      }
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      ImGui::Button(ICON_FAD_FORWARD);
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
