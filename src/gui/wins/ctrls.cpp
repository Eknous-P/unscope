#include "gui.h"

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
    if (i != 0) ImGui::BeginDisabled(shareParams);
    tc[i].timebase = tc[i].traceSize * 1000 / sampleRate;
    if (ImGui::SliderFloat("timebase", &tc[i].timebase, 0, (float)oscDataSize/(float)sampleRate*1000, "%g ms")) {
      tc[i].traceSize = sampleRate * tc[i].timebase / 1000;
      tc[i].traceOffset = ((tc[i].trigOffset + 1.0f)/2) * tc[i].traceSize;
      if (tc[i].traceOffset + tc[i].traceSize > oscDataSize) tc[i].traceOffset = oscDataSize - tc[i].traceSize;
      if (tc[i].traceSize != 0) {
        tc[i].trigOffset = 2*((float)tc[i].traceOffset/(float)tc[i].traceSize)-1.0f;
      } else {
        tc[i].trigOffset = 0;
      }
    }
    ImGui::SliderFloat("trigger", &tc[i].trigger, -1.0f, 1.0f, "%g");
    showTrigger = ImGui::IsItemActive();
    showTrigger |= ai->didTrigger();
    // ImGui::SliderInt("holdoff", &tc[i].trigHoldoff, 0, 1024, "%d");
    ImGui::SameLine();
    if (ImGui::Button(tc[i].triggerEdge?"Rising":"Falling")) tc[i].triggerEdge = !tc[i].triggerEdge;
    if (ImGui::IsItemHovered()) ImGui::SetTooltip("trigger edge");
    ImGui::SliderFloat("x offset", &tc[i].trigOffset, -1.0f, 1.0f, "%g");
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
      tc[i].timebase = tc[0].timebase;
      tc[i].traceSize = tc[0].traceSize;
      tc[i].traceOffset = tc[0].traceOffset;
      tc[i].trigOffset = tc[0].trigOffset;
      tc[i].trigger = tc[0].trigger;
      tc[i].trigHoldoff = tc[0].trigHoldoff;
      tc[i].triggerEdge = tc[0].triggerEdge;
    }
    ImGui::SliderFloat("scale", &tc[i].yScale, 0.25f, 10.0f, "%g");
    ImGui::SliderFloat("y offset", &tc[i].yOffset, -1.0f, 1.0f, "%g");
    
    ImGui::ColorEdit4("##color",tc[i].color);
    
    ImGui::End();
  }
}

void USCGUI::drawXYScopeControls() {
  if (!wo.xyScopeControlsOpen) return;
  if (channels > 1) {
    ImGui::Begin("XY Scope Controls",&wo.xyScopeControlsOpen);
    ImGui::SliderFloat("X scale",&xyp.xScale,0.5f,4.0f,"%g");
    ImGui::SliderFloat("Y scale",&xyp.yScale,0.5f,4.0f,"%g");
    ImGui::SliderFloat("X offset",&xyp.xOffset,-4.0f,4.0f,"%g");
    ImGui::SliderFloat("Y offset",&xyp.yOffset,-4.0f,4.0f,"%g");
    xyp.persistence = xyp.sampleLen * 1000 / sampleRate;
    if (ImGui::SliderFloat("persistence", &xyp.persistence, 0, (float)oscDataSize/(float)sampleRate*1000, "%g ms")) {
      xyp.sampleLen = sampleRate * xyp.persistence / 1000;
    }
    ImGui::ColorEdit4("##color",xyp.color);
    ImGui::End();
  }
}