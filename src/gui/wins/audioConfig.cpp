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

void USCGUI::drawAudioConfig() {
  if (!wo.audioConfigOpen) return;
  ImGui::OpenPopup("Audio Configuration");
  if (ImGui::BeginPopupModal("Audio Configuration", &wo.audioConfigOpen, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_AlwaysAutoResize)) {
    char buf[256];
    snprintf(buf, sizeof(buf), "%d", audConf->sampleRate);
    if (ImGui::BeginCombo("sample rate", buf)) {
      FOR_RANGE(9) {
        snprintf(buf, sizeof(buf), "%d", sampleRates[z]);
        if (ImGui::Selectable(buf, audConf->sampleRate == sampleRates[z])) {
          audConf->sampleRate = sampleRates[z];
        }
      }
      ImGui::EndCombo();
    }
    snprintf(buf, sizeof(buf), "%d", audConf->frameSize);
    if (ImGui::BeginCombo("frame size", buf)) {
      FOR_RANGE(8) {
        snprintf(buf, sizeof(buf), "%d", frameSizes[z]);
        if (ImGui::Selectable(buf, audConf->frameSize == frameSizes[z])) {
          audConf->frameSize = frameSizes[z];
        }
      }
      ImGui::EndCombo();
    }
    if (devs->size() > 0) {
      if (ImGui::BeginCombo("input device",(*devs)[inputDeviceS].devName)) {
        for (int i = 0; i < devs->size(); i++) {
          if (!((*devs)[i].direction&DIR_IN)) continue;
          if (ImGui::Selectable((*devs)[i].devName, inputDeviceS == i)) {
            inputDeviceS = i;
            audConf->inputDevice = (*devs)[i].dev;
          }
        }
        ImGui::EndCombo();
      }
      if (ImGui::BeginCombo("output device",(*devs)[outputDeviceS].devName)) {
        for (int i = 0; i < devs->size(); i++) {
          if (!((*devs)[i].direction&DIR_OUT)) continue;
          if (ImGui::Selectable((*devs)[i].devName, outputDeviceS == i)) {
            outputDeviceS = i;
            audConf->outputDevice = (*devs)[i].dev;
          }
        }
        ImGui::EndCombo();
      }
    }
    if (ImGui::Button("restart audio")) {
      int e=0;
      e = ai->stopAudio();
      e = ai->deinitAudio();
      e = ai->initAudio();
      if (e) {
        errorPopup("Failed to initialize audio!\nError text: %s", ai->getLastIOError());
        audConf->sampleRate = sampleRate;
      } else {
        sampleRate = audConf->sampleRate;
        ai->startAudio();
      }
      if (e==0) {
        wo.audioConfigOpen = false;
        ImGui::CloseCurrentPopup();
      }
    }
    ImGui::SameLine();
    if (ImGui::Button("refresh devices")) {
      ai->getAvailDevices();
    }

    if (ImGui::BeginPopupModal("Error##ERRPOPUP", NULL, ImGuiWindowFlags_NoMove|ImGuiWindowFlags_NoResize)) {
      ImGui::Text("%.2048s", errorText);
      if (ImGui::Button("OK")) {
        ImGui::CloseCurrentPopup();
      }
      ImGui::EndPopup();
    }

    ImGui::EndPopup();
  }
}
