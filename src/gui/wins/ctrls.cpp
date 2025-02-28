#include "gui.h"
#include "imgui-knobs.h"
#include "imgui_toggle.h"

void USCGUI::drawGlobalControls() {
  if (!wo.globalControlsOpen) return;
  ImGui::Begin("Global Controls",&wo.globalControlsOpen);
  ImGui::Toggle("share parameters",&shareParams);

  bool trigShare = shareTrigger>0;
  if (ImGui::Toggle("share trigger",&trigShare)) {
    shareTrigger=-shareTrigger;
  }
  if (ImGui::BeginCombo("trigger##trigMode",triggerNames[trigNum-TRIG_FALLBACK])) {
    for (unsigned char i = TRIG_FALLBACK; i < TRIG_MAX; i++) {
      if (ImGui::Selectable(triggerNames[i-TRIG_FALLBACK],i == trigNum)) {
        trigNum = Triggers(i);
        setupTrigger(trigNum);
      }
    }
    ImGui::EndCombo();
  }
  
  ImGui::Toggle("enable fallback trigger", &doFallback);
  if (ImGui::Toggle("single shot trigger", &singleShot)) {
    updateAudio = true;
  }
  if (singleShot) {
    bool popColor = false;
    if (updateAudio) {
      ImGui::PushStyleColor(ImGuiCol_Button, 0xff00ff00);
      popColor = true;
    }
    if (ImGui::Button("trigger!##singleTrigButton")) {
      updateAudio = true;
    }
    if (popColor) ImGui::PopStyleColor();
  } else {
    ImGui::Toggle("update audio",&updateAudio);
  }

  if (devs.size() > 0) {
    if (ImGui::BeginCombo("device",devs[deviceNum].devName)) {
      for (int i = 0; i < devs.size(); i++) {
        if (ImGui::Selectable(devs[i].devName, deviceNum == i)) {
          deviceNum = i;
          device = devs[i].dev;
        }
      }
      ImGui::EndCombo();
    }
  }
  if (ImGui::Button("restart audio")) {
    err = ai->stop();
    printf(INFO_MSG "opening device %d: %s ..." MSG_END,device,Pa_GetDeviceInfo(device)->name);
    err = ai->init(device,/*audioLoopback*/0);
    channels = ai->getChannelCount();
    if (err != paNoError) {
      printf(ERROR_MSG "%d:cant init audio!%s" MSG_END, err, getErrorMsg(err));
      // try again
      if (err != paInvalidDevice) throw err;
      printf(INFO_MSG "trying default device..." MSG_END);
      device = Pa_GetDefaultInputDevice();
      err = ai->init(device,0);
      channels = ai->getChannelCount();
      if (err != paNoError) {
        printf(ERROR_MSG "%d:cant init audio!\n" MSG_END, err, getErrorMsg(err));
        throw err;
      }
      setAudioDeviceSetting(device);
    }
  }
  ImGui::End();
}

#define UPDATE_TIMEBASE if (tc[i].timebase < 0.0f) tc[i].timebase = 0.0f; \
          if (tc[i].timebase > (float)oscDataSize/(float)sampleRate*1000.0f) tc[i].timebase = (float)oscDataSize/(float)sampleRate*1000.0f; \
          tc[i].traceSize = sampleRate * tc[i].timebase / 1000; \
          // tc[i].traceOffset = ((tc[i].trigOffset + 1.0f)/2) * tc[i].traceSize; \
          // if (tc[i].traceOffset + tc[i].traceSize > oscDataSize) tc[i].traceOffset = oscDataSize - tc[i].traceSize; \
          // if (tc[i].traceSize != 0) { \
          //   tc[i].trigOffset = 2*((float)tc[i].traceOffset/(float)tc[i].traceSize)-1.0f; \
          // } else { \
          //   tc[i].trigOffset = 0; \
          // }

void USCGUI::drawChanControls() {
  for (unsigned char i = 0; i < channels; i++) {
    char strbuf[32];
    sprintf(strbuf,"Channel %d Controls",i+1);
    if (!wo.chanControlsOpen[i]) continue;
    ImGui::Begin(strbuf,&wo.chanControlsOpen[i]);
    ImGui::Toggle("enable", &tc[i].enable);

    ImGui::SameLine();
    bool trigShare = shareTrigger > 0;

    unsigned char mainCh = 0;
    if (trigShare) mainCh = shareTrigger - 1;

    bool disable = (trigShare && i != shareTrigger - 1) || (shareParams && i != mainCh);

    ImGui::BeginDisabled(!trigShare);
    ImGui::SameLine();
    sprintf(strbuf, "##chan%dtrig", i+1);
    if (trigShare) if (ImGui::RadioButton(strbuf,shareTrigger==i+1)) shareTrigger=i+1;
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("trigger to this channel");
    }
    ImGui::EndDisabled();

    ImGui::SameLine();
    // color picker
    sprintf(strbuf, "channel %d color", i+1);
    ImGui::ColorButton(strbuf, tc[i].color);
    sprintf(strbuf, "##chan%dcol", i+1);
    if (ImGui::BeginPopupContextItem(strbuf,ImGuiPopupFlags_MouseButtonLeft)) {
      sprintf(strbuf, "##chan%dcoledit", i+1);
      ImGui::ColorPicker4(strbuf,(float*)&tc[i].color);
      ImGui::EndPopup();
    }

    sprintf(strbuf, "##chan%dctrls", i+1);
    if (ImGui::BeginTable(strbuf, 4)) {
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("c4",ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
        // timebase knob
        ImGui::BeginDisabled(disable);
        tc[i].timebase = tc[i].traceSize * 1000.0f / sampleRate;
        if (ImGuiKnobs::Knob("timebase", &tc[i].timebase, 0, (float)oscDataSize/(float)sampleRate*1000.0f, 0.0f, "%g ms", ImGuiKnobVariant_Stepped, KNOBS_SIZE, 0, 15)) {
          UPDATE_TIMEBASE;
        }
        if (disable) if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
          if (shareParams) {
            ImGui::SetTooltip("shared with channel %d", mainCh + 1);
          } else {
            ImGui::SetTooltip("triggering on channel %d", shareTrigger);
          }
        }
      ImGui::TableNextColumn();
        // y scale knob
        ImGui::EndDisabled();
        if (ImGuiKnobs::Knob("y scale", &tc[i].yScale, 0.25f, 10.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, 0, 15)) {
          if (tc[i].yScale < 0.0f) tc[i].yScale = 0.0f;
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) tc[i].yScale = 1.0f;

      ImGui::TableNextColumn();
        // y offset knob
        if (ImGuiKnobs::Knob("y offset", &tc[i].yOffset, -1.0f, 1.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15)) {
          if (tc[i].yOffset < -1.0f) tc[i].yOffset = -1.0f;
          if (tc[i].yOffset >  1.0f) tc[i].yOffset =  1.0f;
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) tc[i].yOffset = 0.0f;

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      // trigger options here
      unsigned char counter=0;
      ImGui::BeginDisabled(disable);
      for (TriggerParam p : trigger[i]->getParams()) {
        p.draw();
        if (disable) {
          if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
            if (shareParams) {
              ImGui::SetTooltip("shared with channel %d", mainCh + 1);
            } else {
              ImGui::SetTooltip("triggering on channel %d", shareTrigger);
            }
          }
        }
        if (p.getType()!=TP_TOGGLE) {
          if (counter==3) ImGui::TableNextRow();
          ImGui::TableNextColumn();
          counter++;
          counter&=3;
        }
      }
      ImGui::EndDisabled();

      ImGui::EndTable();
    }
    
    ImGui::End();

    if (shareParams && i != mainCh) {
      tc[i].timebase = tc[mainCh].timebase;
      UPDATE_TIMEBASE;
      for (unsigned char j = 0; j < trigger[i]->getParams().size(); j++) {
        TriggerParam p = trigger[i]->getParams()[j];
        memcpy(p.getValue(),trigger[mainCh]->getParams()[j].getValue(),TriggerParamTypeSize[p.getType()]);
      }
    }
  }
}

void USCGUI::drawXYScopeControls() {
  if (!wo.xyScopeControlsOpen) return;
  if (channels > 1) {
    ImGui::Begin("XY Scope Controls",&wo.xyScopeControlsOpen);
    if (ImGui::BeginTable("##xycontrols",3)) {
      ImGui::TableNextRow();
      ImGui::TableNextColumn();

      xyp.persistence = xyp.sampleLen * 1000.0f / sampleRate;
      if (ImGuiKnobs::Knob("persistence", &xyp.persistence, 0.0f, 1000.0f, 0.0f,"%g ms", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15)) {
        xyp.sampleLen = sampleRate * xyp.persistence / 1000;
      }
      RIGHTCLICK_EXACT_INPUT(&xyp.persistence, ImGuiDataType_Float, {if (xyp.persistence<0.0f) {xyp.persistence=0.0f;} if (xyp.persistence>1000.0f) {xyp.persistence=1000.0f;} xyp.sampleLen = sampleRate * xyp.persistence / 1000;})
      ImGui::TableNextColumn();

      ImGuiKnobs::Knob("x scale", &xyp.xScale, 0.5f, 4.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput|ImGuiKnobFlags_ValueTooltip, 15);
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) xyp.xScale = 1.0f;
      RIGHTCLICK_EXACT_INPUT(&xyp.yScale, ImGuiDataType_Float, {if (xyp.yScale<0.25f) {xyp.yScale=0.25f;} if (xyp.yScale>4.0f) {xyp.yScale=4.0f;}})
      ImGui::TableNextColumn();
      ImGuiKnobs::Knob("y scale", &xyp.yScale, 0.5f, 4.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput|ImGuiKnobFlags_ValueTooltip, 15);
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) xyp.yScale = 1.0f;
      RIGHTCLICK_EXACT_INPUT(&xyp.xScale, ImGuiDataType_Float, {if (xyp.xScale<0.25f) {xyp.xScale=0.25f;} if (xyp.xScale>4.0f) {xyp.xScale=4.0f;}})
      ImGui::TableNextRow();

      ImGui::TableNextColumn();
      ImGuiKnobs::Knob("intensity", &xyp.color.w, 0.0f, 1.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
      ImGui::TableNextColumn();
      ImGuiKnobs::Knob("x offset", &xyp.xOffset, -1.0f, 1.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) xyp.xOffset = 0.0f;
      ImGui::TableNextColumn();
      ImGuiKnobs::Knob("y offset", &xyp.yOffset, -1.0f, 1.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) xyp.yOffset = 0.0f;
    ImGui::EndTable();
    }

    ImGui::Text("axis channels (x/y):");
    if (ImGui::InputScalarN("##axisChan", ImGuiDataType_U8, xyp.axisChan, 2, &step_one, NULL, "ch %d", 0)) {
      if (xyp.axisChan[0] < 1) xyp.axisChan[0] = 1;
      if (xyp.axisChan[0] > channels) xyp.axisChan[0] = channels;
      if (xyp.axisChan[1] < 1) xyp.axisChan[1] = 1;
      if (xyp.axisChan[1] > channels) xyp.axisChan[1] = channels;
    }
    ImGui::SameLine();
    if (ImGui::Button("swap")) {
      unsigned char temp = xyp.axisChan[0];
      xyp.axisChan[0] = xyp.axisChan[1];
      xyp.axisChan[1] = temp;
    }

    ImGui::ColorButton("color", xyp.color);
    if (ImGui::BeginPopupContextItem("##xycol",ImGuiPopupFlags_MouseButtonLeft)) {
      ImGui::ColorPicker4("##xycoledit",(float*)&xyp.color);
      ImGui::EndPopup();
    }

    ImGui::End();
  }
}