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

#include "shared.h"
#include "gui.h"
#include "imgui-knobs.h"
#include "imgui_toggle.h"
#include <algorithm> // std::find
#include <iterator> // std::advance

void USCGUI::drawChannelManager(bool* open) {
  if (!*open) return;
  if (ImGui::Begin("Global Controls",open)) {
    if (ImGui::BeginTable("Scopes", 7)) {
      ImGui::TableSetupColumn("c1", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("c2", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("c3", ImGuiTableColumnFlags_WidthStretch);
      ImGui::TableSetupColumn("c4", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("c5", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("c6", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("c7", ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
      ImGui::TableNextColumn();
      ImGui::TableNextColumn();
      ImGui::Text("buffer");
      ImGui::TableNextColumn();
      ImGui::Text("trigger");
      ImGui::TableNextColumn();
      for (int i=0; i<scopes.size(); i++) {
        ImGui::PushID(i);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        // remove button
        if (ImGui::Button("×##scopeRemove")) {
          // first erase the channel pointer from the window its bound to
          vector<ScopeChannel*>& winChans = scopeWindows[scopes[i]->whichWindow].channels;
          auto whichWinChan = std::find(winChans.begin(), winChans.end(), scopes[i]);
          winChans.erase(whichWinChan);
          // then erase the pointer from the channels vector
          auto whichChan = scopes.begin();
          std::advance(whichChan,i);
          scopes.erase(whichChan);
          // (maybe i shouldnt need to keep all the pinters in a single vector first...)
          ImGui::PopID();
          continue;
        }
        ImGui::TableNextColumn();
        // buffer list
        if (ImGui::BeginCombo("##scopeBufferSelect", scopes[i]->getBuffer()->getName().c_str())) {
          ImGui::BeginDisabled();
          ImGui::Selectable("select a buffer...");
          ImGui::EndDisabled();
          ImGui::Separator();
          char comboStrBuf[1024];
          int m=0;
          for (int j=0; j<data->getDriverCount(); j++) {
            for (int k=0; k<data->getDriver(j)->getBufferCount(); k++) {
              DataBuffer* buf = data->getDriver(j)->getBuffer(k);
              snprintf(comboStrBuf, 1024, "%d: %s - %s", m++, data->getDriver(j)->getName(), buf->getName().c_str());
              if (ImGui::Selectable(comboStrBuf, scopes[i]->getBuffer() == buf)) {
                scopes[i]->setBuffer(buf);
              }
            }
          }
          ImGui::EndCombo();
        }
        ImGui::TableNextColumn();
        // trigger list
        if (ImGui::BeginCombo("##scopeTriggerSelect", triggerNames[scopes[i]->getTriggerNum()])) {
          for (int j=0; j<TRIG_MAX; j++) {
            if (ImGui::Selectable(triggerNames[j], j==scopes[i]->getTriggerNum())) {
              scopes[i]->setTriger((Triggers)j);
            }
          }
          ImGui::EndCombo();
        }
        ImGui::TableNextColumn();
        // show waveform
        ImGui::Checkbox("##scopeShowWaveform", &scopes[i]->showWaveform);
        ImGui::TableNextColumn();
        // trigger to this
        bool sharingOnThis = shareTrigger-1 == i; 
        if (ImGui::Button("S##scopeShareTrig")) {
          if (sharingOnThis) {
            shareTrigger = 0;
          } else {
            shareTrigger = i + 1;
          }
        }
        ImGui::TableNextColumn();
        // show controls
        ImGui::Checkbox("##scopeShowControls", &scopes[i]->showControls);
        ImGui::TableNextColumn();
        // color picker
        ImGui::ColorButton("color", scopes[i]->color);
        if (ImGui::BeginPopupContextItem("##xycol",ImGuiPopupFlags_MouseButtonLeft)) {
          ImGui::ColorPicker4("##xycoledit",(float*)&scopes[i]->color);
          ImGui::EndPopup();
        }
        ImGui::PopID();
      }
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      if (ImGui::Button("+##scopeAdd")) {
        
      }
      if (ImGui::BeginPopupContextItem("new scope", ImGuiPopupFlags_MouseButtonLeft)) {
        char comboStrBuf[256];
        if (ImGui::BeginCombo("Select Buffer...##scopeBufferSelect", newScopeBuffer==NULL?"":newScopeBuffer->getName().c_str())) {
          int m=0;
          for (int j=0; j<data->getDriverCount(); j++) {
            for (int k=0; k<data->getDriver(j)->getBufferCount(); k++) {
              DataBuffer* buf = data->getDriver(j)->getBuffer(k);
              snprintf(comboStrBuf, 256, "%d: %s - %s", m++, data->getDriver(j)->getName(), buf->getName().c_str());
              if (ImGui::Selectable(comboStrBuf, newScopeBuffer == buf)) {
                newScopeBuffer = buf;
              }
            }
          }
          if (ImGui::Selectable("Dummy Buffer", newScopeBuffer == &dummyBuffer)) {
            newScopeBuffer = &dummyBuffer;
          }
          ImGui::EndCombo();
        }
        snprintf(comboStrBuf, 64, "Scope Window %d", newScopeWin+1);
        if (ImGui::BeginCombo("Select Window...##scopeWindowSelect", comboStrBuf)) {
          for (int j=0; j<scopeWindows.size(); j++) {
            snprintf(comboStrBuf, 64, "Scope Window %d", j+1);
            if (ImGui::Selectable(comboStrBuf, false)) {
              newScopeWin = j;
            }
          }
          ImGui::EndCombo();
        }
        if (ImGui::Button("Add Scope")) {
          ScopeChannel* scope = new ScopeChannel(newScopeBuffer);
          scope->whichWindow = newScopeWin;
          scopes.push_back(scope);
          scopeWindows[newScopeWin].channels.push_back(scope);
          newScopeBuffer = NULL;
          newScopeWin = 0;
          ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
      }
      ImGui::EndTable();
    }
  }  
  ImGui::End();
}

void USCGUI::ScopeChannel::drawControls(bool trigControlsDisabled) {
  char strbuf[64];
  snprintf(strbuf, 64, "chan%pCtrl", this);
  if (ImGui::BeginTable(strbuf, 4)) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    float timeScaleMax=1000.0f*buffer->getSize()/buffer->getSampleRate();
    // timebase
    if (ImGuiKnobs::Knob(
      "timebase", &timeScale,
      0.0f, timeScaleMax, 0.0f,
      "%g ms", ImGuiKnobVariant_Stepped, KNOBS_SIZE,
      0, 15)) {
      if (timeScale < 0.0f) timeScale = 0.0f;
      if (timeScale > timeScaleMax) timeScale = timeScaleMax;
      samples = msToSamples(timeScale, buffer->getSampleRate());
    }
    ImGui::TableNextColumn();
    // y scale
    if (ImGuiKnobs::Knob(
      "y scale", &yScale,
      0.0f, 10.0f, 0.0f,
      "%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE,
      ImGuiKnobFlags_NoInput, 15)) {
      if (yScale <  0.0f) yScale =  0.0f;
      if (yScale > 10.0f) yScale = 10.0f;
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) yScale = 1.0f;
    ImGui::TableNextColumn();
    // x offset
    if (ImGuiKnobs::Knob(
      "x offset", &xOffset,
      -1.0f, 1.0f, 0.0f,
      "%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE,
      ImGuiKnobFlags_NoInput, 15)) {
      if (xOffset < -1.0f) xOffset = -1.0f;
      if (xOffset >  1.0f) xOffset =  1.0f;
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) xOffset = 0.0f;
    ImGui::TableNextColumn();
    // y offset
    if (ImGuiKnobs::Knob(
      "y offset", &yOffset,
      -1.0f, 1.0f, 0.0f,
      "%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE,
      ImGuiKnobFlags_NoInput, 15)) {
      if (yOffset < -1.0f) yOffset = -1.0f;
      if (yOffset >  1.0f) yOffset =  1.0f;
    }
    if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) yOffset = 0.0f;
    
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    ImGui::BeginDisabled(trigControlsDisabled);
    {
      int counter=0;
      for (int i=0; i<trigger->getParams().size(); i++) {
        trigger->getParams()[i].draw();
        counter++;
        ImGui::TableNextColumn();
        if (counter==3) {
          counter = 0;
          ImGui::TableNextRow();
        }
      }
    }
    ImGui::EndDisabled();

    ImGui::EndTable();
  }
}

void USCGUI::drawChanControls() {
  if (scopeContainChannelControls) return;
  char strbuf[64];
  for (int i=0; i<scopes.size(); i++) {
    snprintf(strbuf, 64, "Channel %d controls", i+1);
    if (ImGui::Begin(strbuf, &scopes[i]->showControls)) {
      scopes[i]->drawControls(shareTrigger<0 || (shareTrigger-1 == i));
    }
    ImGui::End();
  }
}

void USCGUI::XYScope::drawControls(USCData* data) {
  if (!showControls) return;
  ImGui::ColorButton("color", color);
  if (ImGui::BeginPopupContextItem("##xycol",ImGuiPopupFlags_MouseButtonLeft)) {
    ImGui::ColorPicker4("##xycoledit",(float*)&color);
    ImGui::EndPopup();
  }
  // buffer selectors
  if (!buffersOK) {
    ImGui::PushStyleColor(ImGuiCol_Border, 0xff0000ff);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
  }
  char comboStrBuf[1024];
  int m=0;
  if (ImGui::BeginCombo("x channel", xChan->getName().c_str())) {
    for (int j=0; j<data->getDriverCount(); j++) {
      for (int k=0; k<data->getDriver(j)->getBufferCount(); k++) {
        DataBuffer* buf = data->getDriver(j)->getBuffer(k);
        snprintf(comboStrBuf, 1024, "%d: %s - %s", m++, data->getDriver(j)->getName(), buf->getName().c_str());
        if (ImGui::Selectable(comboStrBuf, xChan->getBuffer() == buf)) {
          setBuffers(buf, yChan);
        }
      }
    }
    ImGui::EndCombo();
  }
  ImGui::SameLine();
  if (ImGui::Button("swap")) {
    DataBuffer* temp = xChan;
    xChan = yChan;
    yChan = temp;
  }
  m=0;
  if (ImGui::BeginCombo("y channel", yChan->getName().c_str())) {
    for (int j=0; j<data->getDriverCount(); j++) {
      for (int k=0; k<data->getDriver(j)->getBufferCount(); k++) {
        DataBuffer* buf = data->getDriver(j)->getBuffer(k);
        snprintf(comboStrBuf, 1024, "%d: %s - %s", m++, data->getDriver(j)->getName(), buf->getName().c_str());
        if (ImGui::Selectable(comboStrBuf, yChan->getBuffer() == buf)) {
          setBuffers(xChan, buf);
        }
      }
    }
    ImGui::EndCombo();
  }
  if (!buffersOK) {
    ImGui::PopStyleColor();
    ImGui::PopStyleVar();
    ImGui::Text("cannot use buffer of different sizes/sample rates!");
  }
  // knobs
  if (ImGui::BeginTable("xyScopeCtrls", 3)) {
    ImGui::TableNextRow();
    ImGui::TableNextColumn();
    float maxTime = (xChan->getSize() / xChan->getSampleRate()) * 1000.0f;
    if (maxTime > 1000.0f) maxTime = 1000.0f;
    if (ImGuiKnobs::Knob("persistence", &persistence,
      0.0f, maxTime, 0.0f,
      NULL, ImGuiKnobVariant_Stepped,
      KNOBS_SIZE, 0, 15)) {
        if (persistence < 0.0f) persistence = 0.0f;
        if (persistence > maxTime) persistence = maxTime;
        sampleLen = msToSamples(persistence, xChan->getSampleRate());
    }
    RIGHTCLICK_EXACT_INPUT(&persistence, ImGuiDataType_Float, {
      if (persistence < 0.0f) persistence = 0.0f;
      if (persistence > maxTime) persistence = maxTime;
      sampleLen = msToSamples(persistence, xChan->getSampleRate());
    })
    ImGui::TableNextColumn();
    ImGuiKnobs::Knob("x scale", &xScale,
      0.5f, 4.0f, 0.0f,
      "%g", ImGuiKnobVariant_Stepped,
      KNOBS_SIZE, ImGuiKnobFlags_NoInput|ImGuiKnobFlags_ValueTooltip, 15);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) xScale = 1.0f;
    RIGHTCLICK_EXACT_INPUT(&xScale, ImGuiDataType_Float, {
      if (xScale < 0.25f) xScale=0.25f;
      if (xScale > 4.0f) xScale=4.0f;
    })
    ImGui::TableNextColumn();
    ImGuiKnobs::Knob("y scale", &yScale,
      0.5f, 4.0f, 0.0f,
      "%g", ImGuiKnobVariant_Stepped,
      KNOBS_SIZE, ImGuiKnobFlags_NoInput|ImGuiKnobFlags_ValueTooltip, 15);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) yScale = 1.0f;
    RIGHTCLICK_EXACT_INPUT(&yScale, ImGuiDataType_Float, {
      if (yScale < 0.25f) yScale=0.25f;
      if (yScale > 4.0f) yScale=4.0f;
    })
    ImGui::TableNextRow();
    ImGui::EndTable();

    ImGui::TableNextColumn();
    ImGuiKnobs::Knob("intensity", &color.w,
      0.0f, 1.0f, 0.0f,
      "%g", ImGuiKnobVariant_Stepped,
      KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
    ImGui::TableNextColumn();
    ImGuiKnobs::Knob("x offset", &xOffset,
      -1.0f, 1.0f, 0.0f,
      "%g", ImGuiKnobVariant_Stepped,
      KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) xOffset = 0.0f;
    ImGui::TableNextColumn();
    ImGuiKnobs::Knob("y offset", &yOffset,
      -1.0f, 1.0f, 0.0f,
      "%g", ImGuiKnobVariant_Stepped,
      KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) yOffset = 0.0f;

    ImGui::EndTable();
  }
}

void USCGUI::drawXYScopeControls(bool* open) {
  if (!*open) return;
  if (ImGui::Begin("XY Scope Controls", open)) {
    xyScope.drawControls(data);
  }
  ImGui::End();
}
