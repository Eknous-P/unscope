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
  ImGui::BeginDisabled(!trigShare);
  int trigChan = abs(shareTrigger);
  if (ImGui::InputInt("channel",&trigChan)) {
    if (trigChan < 1) trigChan = 1;
    if (trigChan > channels) trigChan = channels;
  }
  shareTrigger = trigChan;
  if (!trigShare) shareTrigger = -shareTrigger;
  ImGui::EndDisabled();
  
  ImGui::Checkbox("update",&updateOsc);
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
    printf("opening device %d: %s ...\n",device,Pa_GetDeviceInfo(device)->name);
    err = ai->init(device,/*audioLoopback*/0);
    channels = ai->getChannelCount();
    if (err != paNoError) {
      printf("%d:%s", err, getErrorMsg(err).c_str());
      // try again
      if (err != paInvalidDevice) throw err;
      printf("trying default device...\n");
      device = Pa_GetDefaultInputDevice();
      err = ai->init(device,0);
      channels = ai->getChannelCount();
      if (err != paNoError) {
        printf("%d:%s", err, getErrorMsg(err).c_str());
        throw err;
      }
      setAudioDeviceSetting(device);
    }
  }
  ImGui::End();
}

void USCGUI::drawChanControls() {
  for (unsigned char i = 0; i < channels; i++) {
    char strbuf[32];
    sprintf(strbuf,"Channel %d Controls",i+1);
    if (!wo.chanControlsOpen[i]) continue;
    ImGui::Begin(strbuf,&wo.chanControlsOpen[i]);
    ImGui::Checkbox("enable", &tc[i].enable);
    ImGui::SameLine();
    if (ImGui::Button(tc[i].triggerEdge?"Rising":"Falling")) tc[i].triggerEdge = !tc[i].triggerEdge;
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("trigger edge");
    if (i != 0) ImGui::BeginDisabled(shareParams);
    tc[i].timebase = tc[i].traceSize * 1000 / sampleRate;
    if (ImGuiKnobs::Knob("timebase", &tc[i].timebase, 0, (float)oscDataSize/(float)sampleRate*1000, 0.0f, "%g ms", ImGuiKnobVariant_Stepped, KNOBS_SIZE, 0, 15)) {
      tc[i].traceSize = sampleRate * tc[i].timebase / 1000;
      tc[i].traceOffset = ((tc[i].trigOffset + 1.0f)/2) * tc[i].traceSize;
      if (tc[i].traceOffset + tc[i].traceSize > oscDataSize) tc[i].traceOffset = oscDataSize - tc[i].traceSize;
      if (tc[i].traceSize != 0) {
        tc[i].trigOffset = 2*((float)tc[i].traceOffset/(float)tc[i].traceSize)-1.0f;
      } else {
        tc[i].trigOffset = 0;
      }
    }
    ImGui::SameLine();
    ImGuiKnobs::Knob("trigger", &tc[shareParams?shareTrigger-1:i].trigger, -1.0f, 1.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
      if (shareParams) {
        for (unsigned char j = 1; j < channels; j++) tc[j].trigger = 0.0f;
      }
      tc[i].trigger = 0.0f;
    }
    showTrigger = ImGui::IsItemActive();
    showTrigger |= ai->didTrigger(i);
    ImGui::SameLine();
    ImGuiKnobs::Knob("x offset", &tc[i].trigOffset, -1.0f, 1.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) tc[i].trigOffset = 0.0f;
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
    ImGuiKnobs::Knob("y scale", &tc[i].yScale, 0.25f, 10.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, 0, 15);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) tc[i].yScale = 1.0f;
    ImGui::SameLine();
    ImGuiKnobs::Knob("y offset", &tc[i].yOffset, -1.0f, 1.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
    if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) tc[i].yOffset = 0.0f;
    
    ImGui::ColorEdit4("##color",tc[i].color);
    
    ImGui::End();
  }
}

void USCGUI::drawXYScopeControls() {
  if (!wo.xyScopeControlsOpen) return;
  if (channels > 1) {
    ImGui::Begin("XY Scope Controls",&wo.xyScopeControlsOpen);
    if (ImGui::BeginTable("##xycontrols",2)) {
      ImGui::TableNextColumn();

      xyp.persistence = xyp.sampleLen * 1000 / sampleRate;
      if (ImGuiKnobs::Knob("persistence", &xyp.persistence, 0.0f, (float)oscDataSize/(float)sampleRate*1000, 0.0f,"%g ms", ImGuiKnobVariant_Stepped, KNOBS_SIZE, 0, 15)) {
        xyp.sampleLen = sampleRate * xyp.persistence / 1000;
      }

      ImGui::TableNextColumn();

      ImGuiKnobs::Knob("x scale", &xyp.xScale, 0.5f, 4.0f, 0.25f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput|ImGuiKnobFlags_ValueTooltip, 15);
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) xyp.xScale = 1.0f;
      ImGui::SameLine();
      ImGuiKnobs::Knob("x offset", &xyp.xOffset, -1.0f, 1.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) xyp.xOffset = 0.0f;
      
      ImGuiKnobs::Knob("y scale", &xyp.yScale, 0.5f, 4.0f, 0.25f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput|ImGuiKnobFlags_ValueTooltip, 15);
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) xyp.yScale = 1.0f;
      ImGui::SameLine();
      ImGuiKnobs::Knob("y offset", &xyp.yOffset, -1.0f, 1.0f, 0.0f,"%g", ImGuiKnobVariant_Stepped, KNOBS_SIZE, ImGuiKnobFlags_NoInput, 15);
      if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) xyp.yOffset = 0.0f;

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
    ImGui::ColorEdit4("##color",xyp.color);
    ImGui::End();
  }
}