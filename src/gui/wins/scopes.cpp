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
#include <cstdio>
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_toggle.h>
#include <trigger.h>

inline nint msToSamples(float ms, double sampleRate) {
  return ms*sampleRate/1000.0f;
}

USCGUI::ScopeChannel::ScopeChannel() {
  buffer = NULL;
  trigger = NULL;
  spectrum = NULL;
  xOffset   = yOffset = 0.0f;
  timeScale = yScale  = 0.0f;
  samples = 0;
  needle = 0;
  color = ImVec4();
  lineSize = 1.0f;
  whichWindow = 0;
  triggerNum = TRIG_INVALID;
  showWaveform = false;
  showControls = false;
}

USCGUI::ScopeChannel::ScopeChannel(DataBuffer* buf) {
  buffer = buf;
  trigger = NULL;
  spectrum = NULL;
  xOffset = yOffset = 0.0f;
  yScale = 1.0f;
  timeScale = 60.0f;
  samples = msToSamples(timeScale, buffer->getSampleRate());
  needle = 0;
  color = ImVec4(0.1f, 0.92f, 0.2f, 1.0f);
  lineSize = 1.0f;
  whichWindow = 0;
  triggerNum = TRIG_INVALID;
  showWaveform = true;
  showControls = true;
  setTriger(TRIG_FALLBACK);
}

DataBuffer* USCGUI::ScopeChannel::getBuffer() {
  return buffer;
}

Trigger* USCGUI::ScopeChannel::getTrigger() {
  return trigger;
}

Triggers USCGUI::ScopeChannel::getTriggerNum() {
  return triggerNum;
}

#ifdef PROGRAM_DEBUG
nint USCGUI::ScopeChannel::getNeedle() {
  return needle;
}
#endif

int USCGUI::ScopeChannel::setBuffer(DataBuffer* buf) {
  if (buf==NULL) return 1;
  buffer = buf;
  samples = msToSamples(timeScale, buffer->getSampleRate());
  return 0;
}

int USCGUI::ScopeChannel::setTriger(Triggers which) {
  // destroy current trigger
  if (triggerNum>=0 && trigger) {
    delete trigger;
  }
  // set new trigger
  triggerNum = TRIG_INVALID;
  Trigger* tp;
  switch (which) {
    case TRIG_ANALOG:
      tp = new TriggerAnalog;
      break;
    case TRIG_SMOOTH:
      tp = new TriggerSmooth;
      break;
    default:
      tp = new TriggerFallback;
      break;
  }
  if (!tp) return 1;
  tp->setupTrigger(buffer);
  trigger = tp;
  triggerNum = which;
  return 0;
}

void USCGUI::ScopeChannel::drawWaveform(ImRect rect, ImDrawList* dl) {
  if (color.w==0.0f) return;
  ImVec2 *scaledWave = new ImVec2[samples];
  nint i=0;
  if (trigger) {
    if (trigger->trigger(samples)) {
      needle = trigger->getTriggerIndex();
    } else {
      needle = buffer->getSize() - samples;
    }
  } else {
    needle = buffer->getSize() - samples;
  }
  // x offsetting has to be done on the bufer needle
  needle -= msToSamples(timeScale*xOffset/2.0f, buffer->getSampleRate());

  for (; i < samples; i++, needle++) {
    if (needle > buffer->getSize()) break;
    scaledWave[i] = {
      ImLerp(rect.Min.x, rect.Max.x, (float)i/(float)samples),
      ImLerp(rect.GetCenter().y, rect.Min.y, buffer->getValueScaled(needle) * yScale - yOffset)
    };
  }
  dl->AddPolyline(scaledWave, i, ImGui::ColorConvertFloat4ToU32(color), 0, lineSize);
  delete[] scaledWave;
}

USCGUI::ScopeChannel::~ScopeChannel() {
  if (triggerNum>=0) {
    if (trigger) {
      delete trigger;
      trigger = NULL;
    }
  }
  if (spectrum) {
    if (spectrum->in) pffft_aligned_free(spectrum->in);
    if (spectrum->out) pffft_aligned_free(spectrum->out);
    if (spectrum->work) pffft_aligned_free(spectrum->work);
    if (spectrum->setup) pffft_destroy_setup(spectrum->setup);
    if (spectrum->cqt) delete spectrum->cqt;
    delete spectrum;
    spectrum = NULL;
  }
}

bool USCGUI::ScopeWindow::plotDragX(float* v, const char* label, ImDrawList* dl, ImVec4 rect, ImU32 col, float v_min, float v_max) {
  ImVec2 p1, p2;
  p1.x = rect.x + (*v + 1.f) * rect.z/2.f;
  p2.x = p1.x;
  p1.y = rect.y;
  p2.y = p1.y + rect.w;
  dl->AddLine(p1, p2, col);
  ImGui::SetCursorPosX(p1.x-2.5f);
  ImGui::SetCursorPosY(0.0f);
  ImGui::PushID(label);
  ImGui::InvisibleButton(label, ImVec2(5.f,rect.w));
  float min=v_min, max=v_max;
  bool ret = ImGui::DragBehavior(ImGui::GetID(label), ImGuiDataType_Float, v, 2.f/rect.z, &min, &max, "", ImGuiSliderFlags_None);
  if (ImGui::IsItemHovered() || ret) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_Stationary|ImGuiHoveredFlags_DelayNormal)) {
    if (ImGui::BeginTooltip()) {
      ImGui::Text("%s", label);
      ImGui::EndTooltip();
    }
  }
  ImGui::PopID();
  return ret;
}

bool USCGUI::ScopeWindow::plotDragY(float* v, const char* label, ImDrawList* dl, ImVec4 rect, ImU32 col, float v_min, float v_max) {
  ImGuiWindow* window = ImGui::GetCurrentWindow();
  ImVec2 p1, p2;
  p1.x = rect.x;
  p2.x = p1.x + rect.z;
  p1.y = rect.y + (-*v + 1.f) * rect.w/2.f;
  p2.y = p1.y;
  dl->AddLine(p1, p2, col);
  window->DC.CursorPos=ImVec2(p1.x,p1.y-2.5f)-window->Scroll;
  ImGui::PushID(label);
  ImGui::InvisibleButton(label, ImVec2(rect.z, 5.f));
  float min=v_min, max=v_max;
  bool ret = ImGui::DragBehavior(ImGui::GetID(label), ImGuiDataType_Float, v, 2.f/rect.w, &min, &max, "", ImGuiSliderFlags_Vertical);
  if (ImGui::IsItemHovered() || ret) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeNS);
    if (ImGui::IsItemHovered(ImGuiHoveredFlags_Stationary|ImGuiHoveredFlags_DelayNormal)) {
    if (ImGui::BeginTooltip()) {
      ImGui::Text("%s", label);
      ImGui::EndTooltip();
    }
  }
  ImGui::PopID();
  return ret;
}

void USCGUI::ScopeWindow::drawScopeWaveform(ImRect rect, ImDrawList* dl) {
  dl->AddRect(rect.Min, rect.Max, 0xff00ff00);
  const int chans = channels.size();
  // v scale labels
  if (drawScaleY && !channels.empty()) {
    ImVec2 p1, textSize = ImGui::CalcTextSize("-1.000");
    textSize.x += 5.f;
    char buf[16];
    snprintf(buf, 16, " CH.%.2d", primaryChan+1);
    p1.y = rect.Min.y + 5.f;
    p1.x = rect.Min.x;
    dl->AddText(p1, ImGui::GetColorU32(ImGuiCol_Text), buf);
    FOR_RANGE(9) {
      p1.y = ImLerp(rect.Min.y, rect.Max.y, (z+1)/10.f) - textSize.y/2;
      p1.x = rect.Min.x;
      float v = -(((signed char)z-4)/5.f + channels[primaryChan]->yOffset) / channels[primaryChan]->yScale;
      snprintf(buf, 16, v>0?" %1.3f":"%1.3f", v);
      dl->AddText(p1, ImGui::GetColorU32(ImGuiCol_Text), buf);
    }
    rect.Min.x += textSize.x;
  }
  // grid
  if (drawGrid) {
    ImVec2 p1, p2;
    FOR_RANGE(9) {
      // veritcal lines
      p1.x = p2.x = ImLerp(rect.Min.x, rect.Max.x, (z+1)/10.f);
      p1.y = rect.Min.y;
      p2.y = rect.Max.y;
      dl->AddLine(p1, p2, 0x44ffffff);
      // horizontal lines
      p1.y = p2.y = ImLerp(rect.Min.y, rect.Max.y, (z+1)/10.f);
      p1.x = rect.Min.x;
      p2.x = rect.Max.x;
      dl->AddLine(p1, p2, 0x44ffffff);
    }
  }

  // waveforms
  for (int i=0; i<channels.size(); i++) {
    ScopeChannel* chan = channels[i];
    chan->drawWaveform(rect, dl);
    // drags
    // {
    //   char strbuf[256];
    //   Trigger* trig = chan->getTrigger();
    //   ImVec4 trigColor = trig->getTriggered()?ImVec4(0,1,0,.5f):ImVec4(1,0,0,.5f);
    //   snprintf(strbuf, 256, "x offset (ch %d)", i+1);
    //   plotDragX(&chan->xOffset, strbuf, dl, ImVec4(rect.Min.x, rect.Min.y, rect.GetWidth(), rect.GetHeight()), 0xff00ff33);
    //   // snprintf(strbuf, 256, "y offset##CH%d", z);
    //   // plotDragY(&tc[z].yOffset, strbuf, dl, ImVec4(origin.x, origin.y, size.x, size.y), 0xff00ff33);
    //   for (TriggerParam p : trig->getParams()) {
    //     // if (shareTrigger > 0 && i != shareTrigger - 1) continue;
    //     if (p.bindToDragX) {
    //       snprintf(strbuf, 256, "%s (ch %d)", p.getLabel(), i+1);
    //       plotDragX((float*)p.getValuePtr(), strbuf, dl, ImVec4(rect.Min.x, rect.Min.y, rect.GetWidth(), rect.GetHeight()), 0xff0077ff);
    //     }
    //     if (p.bindToDragY) {
    //       snprintf(strbuf, 256, "%s (ch %d)", p.getLabel(), i+1);
    //       plotDragY((float*)p.getValuePtr(), strbuf, dl, ImVec4(rect.Min.x, rect.Min.y-chan->yOffset*rect.GetHeight()/2.0f, rect.GetWidth(), rect.GetHeight()), 0xff0077ff);
    //     }
    //   }
    // }
  }
  // cursors (also drags)
  if (showHCursors) {
    ImVec4 cursorColor = ImVec4(.24f, .13f, .98f, 1.f);
    char buf[256];
    plotDragX(&HCursors[0].pos, "x cursor 1", dl, ImVec4(rect.Min.x, rect.Min.y, rect.GetWidth(), rect.GetHeight()), ImGui::ColorConvertFloat4ToU32(cursorColor));
    plotDragX(&HCursors[1].pos, "x cursor 2", dl, ImVec4(rect.Min.x, rect.Min.y, rect.GetWidth(), rect.GetHeight()), ImGui::ColorConvertFloat4ToU32(cursorColor));

    FOR_RANGE(chans) {
      const float tDiff = fabsf(
        (channels[primaryChan]->timeScale) * ((HCursors[1].pos - HCursors[0].pos) / 2.0f)
      );
      snprintf(buf, 256, "[CH %d] X1: %.3f X2: %.3f | %2.4fms (%4.4fHz)",
        z + 1,
        HCursors[0].pos,
        HCursors[1].pos,
        tDiff,
        1000.0f / tDiff
      );
      dl->AddText(rect.Min+ImVec2(5.0f, 5.0f+10.0f*z),ImGui::GetColorU32(ImGuiCol_Text), buf);
    }
  }
  if (showVCursors) {
    ImVec4 cursorColor = ImVec4(.94f, .73f, .18f, 1.f);
    char buf[256];
    plotDragY(&VCursors[0].pos, "y cursor 1", dl, ImVec4(rect.Min.x, rect.Min.y, rect.GetWidth(), rect.GetHeight()), ImGui::ColorConvertFloat4ToU32(cursorColor));
    plotDragY(&VCursors[1].pos, "y cursor 2", dl, ImVec4(rect.Min.x, rect.Min.y, rect.GetWidth(), rect.GetHeight()), ImGui::ColorConvertFloat4ToU32(cursorColor));

    FOR_RANGE(chans) {
      const float vDiff = fabsf(VCursors[1].pos - VCursors[0].pos); // TODO: v cal
      snprintf(buf, 256, "[CH %d] Y1: %.3f Y2: %.3f | %2.4fV p-p",
        z + 1,
        VCursors[0].pos,
        VCursors[1].pos,
        vDiff
      );
      dl->AddText(rect.Min+ImVec2(5.0f, rect.GetHeight()-15.0f-10.0f*z),ImGui::GetColorU32(ImGuiCol_Text), buf);
    }
  }
  // trig hints
  // {
  //   if (trigNum==TRIG_SMOOTH) {
  //     FOR_RANGE(channels) {
  //       unsigned char trigChan = (shareTrigger>0)?(shareTrigger-1):(shareParams?0:z);
  //       if (!tc[z].enable) continue;
  //       if (shareTrigger>0) {
  //         if (z!=shareTrigger-1) continue;
  //       }
  //       if (trigger[trigChan]->getParams()[0].isHovered()) {
  //         nint len = tc[z].traceSize;
  //         long int offset = (tc[z].traceSize / 2.f) * tc[z].xOffset;
  //         scaledWave = new ImVec2[len];
  //         nint i=0;
  //         unsigned char trigChan = (shareTrigger>0)?(shareTrigger-1):(shareParams?0:z);
  //         float* smoothBuf=((TriggerSmooth*)(trigger[z]))->getSmoothBuffer();
  //         for (; i < len; i++) {
  //           scaledWave[i].x = origin.x + size.x*((float)i/(float)len);
  //
  //           nint cur = i;
  //           if (triggered[z]) {
  //             cur += trigger[trigChan]->getTriggerIndex();
  //           }
  //           else if (doFallback) cur += oscDataSize - tc[z].traceSize;
  //           else continue;
  //
  //           cur-=offset;
  //
  //           if (cur > oscDataSize) break;
  //           scaledWave[i].y = origin.y - (smoothBuf[cur] * tc[z].yScale * (1.0f+trigger[z]->getParams()[0].getValue<float>()) + tc[z].yOffset - 1.f) * size.y/2.f;
  //         }
  //         dl->AddPolyline(scaledWave, i, 0xffff66ff, 0, .4f);
  //         delete[] scaledWave;
  //       }
  //       if (trigger[z]->getParams()[1].isHovered()) {
  //         float level=((TriggerSmooth*)(trigger[z]))->getTriggerLevel();
  //         level=origin.y+(level+1.0f)*size.y/2.0f;
  //         dl->AddLine(
  //           ImVec2(origin.x,level), 
  //           ImVec2(origin.x+size.x, level),
  //           0xff0077ff
  //         );
  //       }
  //     }
  //   }
  // }

}

void USCGUI::ScopeWindow::drawScope(bool scopeContainChannelControls) {
  if (!windowOpen) return;
  char strbuf[256];
  snprintf(strbuf, 256, "Scope %d", index+1);
  if (ImGui::Begin(strbuf, &windowOpen)) {
    if (scopeContainChannelControls && showChannelControls) {
      if (ImGui::BeginTable("scopeWin", 2, ImGuiTableFlags_Resizable)) {
        ImGui::TableSetupColumn("c1", ImGuiTableColumnFlags_WidthStretch, 0.75f);
        ImGui::TableSetupColumn("c2", ImGuiTableColumnFlags_WidthStretch, 0.25f);
        ImGui::TableNextRow();
        ImGui::TableNextColumn();
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f,0.0f));
        if (ImGui::BeginChild("waveform", ImGui::GetContentRegionAvail())) {
          ImDrawList* dl = ImGui::GetWindowDrawList();
          ImVec2 origin = ImGui::GetCursorScreenPos(), size = ImGui::GetContentRegionAvail();
          ImRect rect(origin, origin+size);
          drawScopeWaveform(rect, dl);
        }
        ImGui::EndChild();  
        ImGui::PopStyleVar();
        ImGui::TableNextColumn();
        char strbuf[128];
        for (int i=0; i<channels.size(); i++) {
          snprintf(strbuf, 128, "channel %d", i+1);
          if (ImGui::TreeNode(strbuf)) {
            channels[i]->drawControls(false);
            ImGui::TreePop();
          }
        }
        ImGui::EndTable();
      }
    } else {
      ImDrawList* dl = ImGui::GetWindowDrawList();
      ImVec2 origin = ImGui::GetCursorScreenPos(), size = ImGui::GetContentRegionAvail();
      ImRect rect(origin, origin+size);
      drawScopeWaveform(rect, dl);
    }
    if (showWindowControls) {
      ImGui::Toggle("show channel controls", &showChannelControls);
      ImGui::SameLine();
      if (ImGui::InputInt("primary channel", &primaryChan)) {
        if (primaryChan < 0) primaryChan = 0;
        if (primaryChan > channels.size()-1) primaryChan = channels.size()-1;
      }
    }
  }
  ImGui::End();
}

USCGUI::XYScope::XYScope():
  xChan(NULL),
  yChan(NULL),
  buffersOK(false),
  xScale(1.0f),
  yScale(1.0f),
  xOffset(0.0f),
  yOffset(0.0f),
  color(ImVec4()),
  persistence(0.0f),
  sampleLen(0),
  lineSize(1.0f),
  showControls(false) {}

USCGUI::XYScope::XYScope(DataBuffer* x, DataBuffer* y) {
  xChan = x;
  yChan = y;
  if (xChan->getSize()==yChan->getSize() && xChan->getSampleRate()==yChan->getSampleRate()) {
    buffersOK = true;
  } else {
    buffersOK = false;
  }
  xScale = yScale = 1.0f;
  xOffset = yOffset = 0.0f;
  color = ImVec4(0.2f, 0.9f, 0.1f, 1.0f);
  persistence = 16.6f;
  sampleLen = msToSamples(persistence, xChan->getSampleRate());
  lineSize=1.0f;
  showControls=true;
}

void USCGUI::XYScope::setBuffers(DataBuffer* x, DataBuffer* y) {
  xChan = x;
  yChan = y;
  if (xChan->getSize()==yChan->getSize() && xChan->getSampleRate()==yChan->getSampleRate()) {
    buffersOK = true;
  } else {
    buffersOK = false;
  }
  sampleLen = msToSamples(persistence, xChan->getSampleRate());
}

void USCGUI::XYScope::drawWaveform(ImRect rect, ImDrawList* dl) {
  if (!buffersOK) return;
  ImVec2* scaledPlot = new ImVec2[sampleLen];
  nint cur = xChan->getSize() - sampleLen;
  for (nint i=0; i<sampleLen; i++, cur++) {
    scaledPlot[i] = {
      ImLerp(rect.Min.x, rect.Max.x, xScale*xChan->getValueScaled(cur)+xOffset/2.0f),
      ImLerp(rect.Min.y, rect.Max.y, xScale*yChan->getValueScaled(cur)+xOffset/2.0f),
    };
  }
  dl->AddPolyline(scaledPlot, sampleLen, ImGui::ColorConvertFloat4ToU32(color), 0, lineSize);
  delete[] scaledPlot;
}

void USCGUI::drawXYScope(bool* open) {
  if (!*open) return;
  ImGui::Begin("Scope (XY)",open);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f,0.0f));
  ImDrawList* dl = ImGui::GetWindowDrawList();
  ImVec2 origin = ImGui::GetWindowPos(), size = ImGui::GetWindowSize();
  float titleBar = ImGui::GetCurrentWindow()->TitleBarHeight;
  origin.y += titleBar;
  size.y -= titleBar;
  ImVec2 sizeHalf = size/2;
  float sizeMin = (size.x<size.y?size.x:size.y)/2.0f;
  // grid
  ImRect rect = {
    origin.x + sizeHalf.x - sizeMin,
    origin.y + sizeHalf.y - sizeMin,
    origin.x + sizeHalf.x + sizeMin,
    origin.y + sizeHalf.y + sizeMin
  };
  {
    dl->AddRect(rect.Min, rect.Max, 0x44ffffff);
    ImVec2 p1, p2;
    FOR_RANGE(9) {
      // veritcal lines
      p1.x = origin.x + sizeHalf.x - sizeMin * ((z-4)/5.f);
      p2.x = p1.x;
      p1.y = origin.y + sizeHalf.y - sizeMin;
      p2.y = origin.y + sizeHalf.y + sizeMin;
      dl->AddLine(p1, p2, 0x44ffffff);
      // horizontal lines
      p1.y = origin.y + sizeHalf.y - sizeMin * ((z-4)/5.f);
      p2.y = p1.y;
      p1.x = origin.x + sizeHalf.x - sizeMin;
      p2.x = origin.x + sizeHalf.x + sizeMin;
      dl->AddLine(p1, p2, 0x44ffffff);
    }
  }
  xyScope.drawWaveform(rect, dl);
  ImGui::PopStyleVar();
  ImGui::End();
}
