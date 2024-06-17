#include "gui.h"

void GUI::drawControls() {
  ImGui::Begin("Global Controls");
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

  for (unsigned char i = 0; i < channels; i++) {
    char strbuf[32];
    sprintf(strbuf,"Channel %d Controls",i+1);
    ImGui::Begin(strbuf);
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
    ImGui::SliderFloat("scale", &tc[i].yScale, 0.25f, 10.0f, "%g");
    ImGui::SliderFloat("trigger", &tc[i].trigger, -1.0f, 1.0f, "%g");
    showTrigger = ImGui::IsItemActive();
    showTrigger |= ai->didTrigger();
    ImGui::SliderInt("holdoff", &tc[i].trigHoldoff, 0, 1024, "%d");
    ImGui::SliderFloat("offset", &tc[i].trigOffset, -1.0f, 1.0f, "%g");
    if (ImGui::IsItemActive()) {
      tc[i].traceOffset = ((tc[i].trigOffset + 1.0f)/2) * tc[i].traceSize;
      if (tc[i].traceSize != 0) {
        tc[i].trigOffset = 2*((float)tc[i].traceOffset/(float)tc[i].traceSize)-1.0f;
      } else {
        tc[i].trigOffset = 0;
      }
    }
    if (tc[i].traceOffset + tc[i].traceSize > oscDataSize) tc[i].traceOffset = oscDataSize - tc[i].traceSize;
    ImGui::AlignTextToFramePadding();
    ImGui::Text("trigger edge:");
    ImGui::SameLine();
    if (ImGui::Button(tc[i].triggerEdge?"Rising":"Falling")) tc[i].triggerEdge = !tc[i].triggerEdge;
    
    ImGui::ColorEdit4("##color",tc[i].color);
    
    ImGui::End();
  }
}