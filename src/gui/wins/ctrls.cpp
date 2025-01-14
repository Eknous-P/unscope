#include "gui.h"
#include "imgui-knobs.h"

#define KNOBS_SIZE 50.0f

void USCGUI::drawGlobalControls() {
  if (!wo.globalControlsOpen) return;
  ImGui::Begin("Global Controls",&wo.globalControlsOpen);
  ImGui::Checkbox("share parameters",&shareParams);

  bool trigShare = shareTrigger>0;
  if (ImGui::Checkbox("share trigger",&trigShare)) {
    shareTrigger=-shareTrigger;
  }

  if (ImGui::BeginCombo("trigger mode",triggerModeNames[triggerMode])) {
    for (unsigned char i = 0; i < 4; i++) {
      if (ImGui::Selectable(triggerModeNames[i],i == triggerMode)) {
        triggerMode = TriggerModes(i);
        if (triggerMode!=TRIGGER_SINGLE) updateAudio = true;
      }
    }
    ImGui::EndCombo();
  }
  if (triggerMode==TRIGGER_SINGLE) {
    if (ai->didTrigger(shareTrigger>0?shareTrigger-1:255)) updateAudio = false;
    if (ImGui::Button("trigger")) updateAudio = true;
  } else {
    ImGui::Checkbox("update audio",&updateAudio);
  }

  if (devs.size() > 0) {
    if (ImGui::BeginCombo("device",devs[deviceNum].devName.c_str())) {
      for (int i = 0; i < devs.size(); i++) {
        if (ImGui::Selectable(devs[i].devName.c_str(), deviceNum == i)) {
          deviceNum = i;
          device = devs[i].dev;
        }
      }
      ImGui::EndCombo();
    }
  }
  if (ImGui::Button("restart audio")) {
    err = ai->stop();
    printf(INFO_MSG "opening device %d: %s ...\n" MSG_END,device,Pa_GetDeviceInfo(device)->name);
    err = ai->init(device,/*audioLoopback*/0);
    channels = ai->getChannelCount();
    if (err != paNoError) {
      printf(ERROR_MSG "%d:cant init audio!\n%s" MSG_END, err, getErrorMsg(err));
      // try again
      if (err != paInvalidDevice) throw err;
      printf(INFO_MSG "trying default device...\n" MSG_END);
      device = Pa_GetDefaultInputDevice();
      err = ai->init(device,0);
      channels = ai->getChannelCount();
      if (err != paNoError) {
        printf(ERROR_MSG "%d:cant init audio!\n%s" MSG_END, err, getErrorMsg(err));
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
          tc[i].traceOffset = ((tc[i].trigOffset + 1.0f)/2) * tc[i].traceSize; \
          if (tc[i].traceOffset + tc[i].traceSize > oscDataSize) tc[i].traceOffset = oscDataSize - tc[i].traceSize; \
          if (tc[i].traceSize != 0) { \
            tc[i].trigOffset = 2*((float)tc[i].traceOffset/(float)tc[i].traceSize)-1.0f; \
          } else { \
            tc[i].trigOffset = 0; \
          }

#define RIGHTCLICK_EXACT_INPUT(v,d,f) \
        if (ImGui::BeginPopupContextItem(#v "input")) { \
          if (ImGui::InputScalar("##" #v "input",d,&v)) f; \
          ImGui::EndPopup(); \
        }

void USCGUI::drawChanControls() {
  for (unsigned char i = 0; i < channels; i++) {
    char strbuf[32];
    sprintf(strbuf,"Channel %d Controls",i+1);
    if (!wo.chanControlsOpen[i]) continue;
    ImGui::Begin(strbuf,&wo.chanControlsOpen[i]);
    ImGui::Checkbox("enable", &tc[i].enable);
    ImGui::SameLine();
    if (i != 0) ImGui::BeginDisabled(shareParams);
    if (ImGui::Button(tc[i].triggerEdge?"Rising":"Falling")) tc[i].triggerEdge = !tc[i].triggerEdge;
    if (i != 0) ImGui::EndDisabled();
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("trigger edge");
    bool trigShare = shareTrigger>0;
    ImGui::BeginDisabled(!trigShare);
    ImGui::SameLine();
    sprintf(strbuf, "##chan%dtrig", i+1);
    if (ImGui::RadioButton(strbuf,shareTrigger==i+1)) shareTrigger=i+1;
    if (ImGui::IsItemHovered()) {
      ImGui::SetTooltip("trigger to this channel");
    }
    ImGui::EndDisabled();

    sprintf(strbuf, "##chan%dctrls", i+1);
    if (ImGui::BeginTable(strbuf, 3)) {
      ImGui::TableSetupColumn("c1",ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("c2",ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableSetupColumn("c3",ImGuiTableColumnFlags_WidthFixed);
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
        // timebase knob
        if (i != 0) ImGui::BeginDisabled(shareParams);
        tc[i].timebase = tc[i].traceSize * 1000.0f / sampleRate;
        if (ImGuiKnobs::Knob("timebase", &tc[i].timebase, 0, (float)oscDataSize/(float)sampleRate*1000.0f, 0.0f, "%g ms", ImGuiKnobVariant_Stepped, KNOBS_SIZE, 0, 15)) {
          UPDATE_TIMEBASE;
        }
      ImGui::TableNextColumn();
        // y scale knob
        if (i != 0) ImGui::EndDisabled();
        if (ImGuiKnobs::Knob("y scale", &tc[i].yScale, 0.25f, 10.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, 0, 15)) {
          if (tc[i].yScale < 0.0f) tc[i].yScale = 0.0f;
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) tc[i].yScale = 1.0f;

      ImGui::TableNextColumn();
        // trigger knob
        bool hideTriggerKnob = false;
        float* whichTrig = &tc[i].trigger;
        if (shareParams) {
          hideTriggerKnob = i!=0;
          whichTrig = &tc[0].trigger;
        }
        if (shareTrigger > 0) {
          hideTriggerKnob = (shareTrigger - 1)!=i;
          whichTrig = &tc[shareTrigger - 1].trigger;
        }
        ImGui::BeginDisabled(hideTriggerKnob);
        ImGuiKnobs::Knob("trigger", whichTrig, -1.0f, 1.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
        if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
          if (shareParams) {
            for (unsigned char j = 1; j < channels; j++) tc[j].trigger = 0.0f;
          }
          tc[i].trigger = 0.0f;
        }
        showTrigger = ImGui::IsItemActive();
        showTrigger |= ai->didTrigger(i);
        ImGui::EndDisabled();
        if (i != 0) ImGui::BeginDisabled(shareParams);

      ImGui::TableNextRow();
      ImGui::TableNextColumn();
        // x offset knob
        if (ImGuiKnobs::Knob("x offset", &tc[i].trigOffset, -1.0f, 1.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15)) {
          if (tc[i].trigOffset < -1.0f) tc[i].trigOffset = -1.0f;
          if (tc[i].trigOffset >  1.0f) tc[i].trigOffset =  1.0f;
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) {
          tc[i].trigOffset = 0.0f;
          tc[i].traceOffset = ((tc[i].trigOffset + 1.0f)/2) * tc[i].traceSize;
          if (tc[i].traceSize != 0) {
            tc[i].trigOffset = 2*((float)tc[i].traceOffset/(float)tc[i].traceSize)-1.0f;
          } else {
            tc[i].trigOffset = 0;
          }
        }
        if (ImGui::IsItemActive()) {
          tc[i].traceOffset = ((tc[i].trigOffset + 1.0f)/2) * tc[i].traceSize;
          if (tc[i].traceSize != 0) {
            tc[i].trigOffset = 2*((float)tc[i].traceOffset/(float)tc[i].traceSize)-1.0f;
          } else {
            tc[i].trigOffset = 0;
          }
        }
        if (tc[i].traceOffset + tc[i].traceSize > oscDataSize) tc[i].traceOffset = oscDataSize - tc[i].traceSize;
        if (i != 0) ImGui::EndDisabled();
        if (i != 0 && shareParams) {
          tc[i].timebase    = tc[0].timebase;
          tc[i].traceSize   = tc[0].traceSize;
          tc[i].traceOffset = tc[0].traceOffset;
          tc[i].trigOffset  = tc[0].trigOffset;
          tc[i].trigger     = tc[shareTrigger>0?(shareTrigger-1):0].trigger;
          tc[i].trigHoldoff = tc[0].trigHoldoff;
          tc[i].triggerEdge = tc[0].triggerEdge;
        }

      ImGui::TableNextColumn();
        // y offset knob
        if (ImGuiKnobs::Knob("y offset", &tc[i].yOffset, -1.0f, 1.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15)) {
          if (tc[i].yOffset < -1.0f) tc[i].yOffset = -1.0f;
          if (tc[i].yOffset >  1.0f) tc[i].yOffset =  1.0f;
        }
        if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) tc[i].yOffset = 0.0f;

      ImGui::EndTable();
    }
    
    // color picker
    ImGui::ColorEdit4("##coledit",(float*)&tc[i].color);
    
    ImGui::End();
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
      RIGHTCLICK_EXACT_INPUT(xyp.persistence, ImGuiDataType_Float, {if (xyp.persistence<0.0f) {xyp.persistence=0.0f;} if (xyp.persistence>1000.0f) {xyp.persistence=1000.0f;} xyp.sampleLen = sampleRate * xyp.persistence / 1000;})
      ImGui::TableNextColumn();

      ImGuiKnobs::Knob("x scale", &xyp.xScale, 0.5f, 4.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput|ImGuiKnobFlags_ValueTooltip, 15);
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) xyp.xScale = 1.0f;
      RIGHTCLICK_EXACT_INPUT(xyp.yScale, ImGuiDataType_Float, {if (xyp.yScale<0.25f) {xyp.yScale=0.25f;} if (xyp.yScale>4.0f) {xyp.yScale=4.0f;}})
      ImGui::TableNextColumn();
      ImGuiKnobs::Knob("y scale", &xyp.yScale, 0.5f, 4.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput|ImGuiKnobFlags_ValueTooltip, 15);
      if (ImGui::IsItemClicked(ImGuiMouseButton_Middle)) xyp.yScale = 1.0f;
      RIGHTCLICK_EXACT_INPUT(xyp.xScale, ImGuiDataType_Float, {if (xyp.xScale<0.25f) {xyp.xScale=0.25f;} if (xyp.xScale>4.0f) {xyp.xScale=4.0f;}})
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

    if (ImGui::InputInt("X channel",&xyp.xChan)) {
      if (xyp.xChan < 1) xyp.xChan = 1;
      if (xyp.xChan > channels) xyp.xChan = channels;
    }
    if (ImGui::InputInt("Y channel",&xyp.yChan)) {
      if (xyp.yChan < 1) xyp.yChan = 1;
      if (xyp.yChan > channels) xyp.yChan = channels;
    }

    ImGui::ColorEdit3("##coledit",(float*)&xyp.color);

    ImGui::End();
  }
}