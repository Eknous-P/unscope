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

#include "scope.h"
#include "gui.h"
#include "imgui-knobs.h"
#include "imgui_toggle.h"
#include "consts.h"
#include "pffft.h"
#include <IconsFontaudio.h>
#include <cstdio>
#include <imgui.h>
#include <iterator> // std::advance

inline nint msToSamples(float ms, double sampleRate) {
  return ms*sampleRate/1000.0f;
}

inline float windowHann(float v, float ext) {
  return (0.5*(1.0-cos(2.0*M_PI*v)));
}

inline float scaleLinear(float x) {return x;}

inline float scaleFuncLog(float x) {
  constexpr float base=100;
  return log((base-1)*x+1.0f)/log(base);
}

inline float scaleFuncDb(float y) {
  return log10(y)*20.0f/70.0f+1;
}

const float yScaleKnobLimits[3] = {0.25f, 10.0f, 1.0f};
const float lineSizeKnobLimits[3] = {1.0f, 16.0f, 1.0f};

// ScopeChannel

ScopeChannel::ScopeChannel() {
  parent = NULL;
  buffer = NULL;
  trigger = NULL;
  fft = {NULL, NULL, NULL, NULL, NULL, 0, 0, 0, 0, 0, false, false};
  params = {};
  bufferNum = {0,0};
  xOffset   = yOffset = 0.0f;
  timeScale = yScale  = 0.0f;
  samples = 0;
  needle = 0;
  color = ImVec4();
  lineSize = 1.0f;
  timeScaleLimits[0]=0;
  timeScaleLimits[1]=0;
  timeScaleLimits[2]=60;
  triggerNum = TRIG_INVALID;
  chanOfTrigger = 0;
  showWaveform = false;
  showControls = false;
}

ScopeChannel::ScopeChannel(ScopeWindow* p, DataBuffer* buf, WhichBuffer num) {
  parent = p;
  buffer = buf;
  trigger = NULL;
  fft = {&windowHann, NULL, NULL, NULL, NULL, 8192, 0, 24000, 0, 1, true, true};
  params = {
    Parameter(PARAM_KNOBFLOAT, true, "timeScale", "time scale", NULL, (void*)timeScaleLimits, &timeScale, NULL),
    Parameter(PARAM_KNOBFLOAT, true, "yScale", "y scale", NULL, (void*)yScaleKnobLimits, &yScale, NULL),
    Parameter(PARAM_KNOBNORM, true, "xOffset", "x offset", NULL, NULL, &xOffset),
    Parameter(PARAM_KNOBNORM, true, "yOffset", "y offset", NULL, NULL, &yOffset),
    Parameter(PARAM_KNOBFLOAT, true, "lineSize", "line size", NULL, (void*)lineSizeKnobLimits, &lineSize),
  };
  bufferNum = num;
  xOffset = yOffset = 0.0f;
  yScale = 1.0f;
  timeScale = 60.0f;
  if (buffer) {
    samples = msToSamples(timeScale, buffer->getSampleRate());
    timeScaleLimits[1] = 1000.0f*buffer->getSize()/buffer->getSampleRate();
  } else {
    samples=0;
    timeScaleLimits[1] = 1;
  }
  needle = 0;
  color = ImVec4(0.1f, 0.92f, 0.2f, 1.0f);
  lineSize = 1.0f;
  timeScaleLimits[0]=0;
  timeScaleLimits[2]=60;
  triggerNum = TRIG_INVALID;
  chanOfTrigger = 0;
  showWaveform = true;
  showControls = true;
  setTriger(TRIG_FALLBACK);
}

DataBuffer* ScopeChannel::getBuffer() {
  return buffer;
}

Trigger* ScopeChannel::getTrigger() {
  return trigger;
}

int ScopeChannel::getTriggerChan() {
  if (triggerNum != TRIG_STOLEN) return -1;
  return chanOfTrigger;
}

Triggers ScopeChannel::getTriggerNum() {
  if (triggerNum==TRIG_INVALID) return TRIG_FALLBACK; 
  return triggerNum;
}

#ifdef PROGRAM_DEBUG
nint ScopeChannel::getNeedle() {
  return needle;
}
#endif

nint ScopeChannel::getSamples() {
  return samples;
}

int ScopeChannel::setBuffer(DataBuffer* buf, WhichBuffer num) {
  buffer = buf;
  bufferNum = num;
  if (buffer == NULL) {
    setTriger(TRIG_INVALID);
    return 1;
  }
  samples = msToSamples(timeScale, buffer->getSampleRate());
  timeScaleLimits[1] = 1000.0f*buffer->getSize()/buffer->getSampleRate();
  setTriger(triggerNum);
  return 0;
}

void ScopeChannel::deleteTrigger() {
  if (triggerNum==TRIG_STOLEN) {
    triggerNum = TRIG_INVALID;
    return;
  }
  if (triggerNum>=0 && trigger) {
    delete trigger;
    trigger = NULL;
  }

} 

int ScopeChannel::setTriger(Triggers which) {
  // destroy current trigger
  deleteTrigger();
  // set new trigger
  triggerNum = TRIG_INVALID;
  Trigger* tp;
  switch (which) {
    case TRIG_INVALID:
      return 1;
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

int ScopeChannel::setTrigger(int chan) {
  deleteTrigger();
  trigger = NULL;
  triggerNum = TRIG_STOLEN;
  chanOfTrigger = chan;
  return 0;
}

void ScopeChannel::drawParams() {
  if (!buffer->isInited()) {
    ImGui::TextUnformatted("selected buffer is not initialized");
  }
  ImGui::BeginDisabled(!buffer->isInited());
  const float minX = ImGui::GetCursorPosX();
  const float maxX = minX+ImGui::GetContentRegionAvail().x;
  float currentX = minX;
  // the 1st one is the timescale, which needs special attention
   ImGui::BeginDisabled(triggerNum==TRIG_STOLEN);
  if (params[0].draw()) {
    samples = msToSamples(timeScale, buffer->getSampleRate());
  }
  if (triggerNum==TRIG_STOLEN && ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled)) {
    ImGui::SetTooltip("time scale is locked to the channel the trigger is stolen from");
  }
  ImGui::EndDisabled();
  currentX+=params[0].getEstimatedWidth();
  for (int j=1; j<params.size(); j++) {
    currentX+=params[j].getEstimatedWidth();
    if (currentX<maxX) ImGui::SameLine();
    else currentX = minX+params[j].getEstimatedWidth();
    params[j].draw();
  }
  if (triggerNum!=TRIG_STOLEN && trigger!=NULL && !trigger->getParams()->empty()) {
    ImGui::Separator();
    currentX = minX+trigger->getParams()->at(0).getEstimatedWidth();
    trigger->getParams()->at(0).draw();
    for (int j=1; j<trigger->getParams()->size(); j++) { // yea sorry its a mess again >~<
      currentX+=trigger->getParams()->at(j).getEstimatedWidth();
      if (currentX<maxX) ImGui::SameLine();
      else currentX = minX+trigger->getParams()->at(j).getEstimatedWidth();
      trigger->getParams()->at(j).draw();
    }
  }
  ImGui::EndDisabled();
}

void ScopeChannel::drawWaveform(ImRect rect, ImDrawList* dl) {
  if (!showWaveform) return;
  if (color.w==0.0f) return;
  if (buffer == NULL) return;
  if (!buffer->isInited()) return;
  nint i=0;
  if (triggerNum == TRIG_STOLEN) {
    Trigger* stolenTrigger = parent->channels[chanOfTrigger]->getTrigger();
    if (stolenTrigger == NULL) {
      deleteTrigger();
      needle = buffer->getSize() - samples;
    } else {
      timeScale=parent->channels[chanOfTrigger]->timeScale;
      samples=parent->channels[chanOfTrigger]->samples;
      if (stolenTrigger->getTriggered()) needle = stolenTrigger->getTriggerIndex();
      else needle = buffer->getSize() - samples;
    }
  } else {
    if (trigger) {
      if (trigger->trigger(samples)) {
        needle = trigger->getTriggerIndex();
      } else {
        needle = buffer->getSize() - samples;
      }
    } else {
      needle = buffer->getSize() - samples;
    }
  }
  // x offsetting has to be done on the bufer needle
  needle -= msToSamples(timeScale*xOffset/2.0f, buffer->getSampleRate());

  ImVec2 *scaledWave = new ImVec2[samples];
  for (; i < samples; i++, needle++) {
    if (needle > buffer->getSize()) break;
    scaledWave[i] = {
      ImLerp(rect.Min.x, rect.Max.x, (float)i/(float)samples),
      ImLerp(rect.GetCenter().y, rect.Min.y, buffer->getValueScaled(needle) * yScale + yOffset)
    };
  }
  dl->AddPolyline(scaledWave, i, ImGui::ColorConvertFloat4ToU32(color), 0, lineSize);
  delete[] scaledWave;
}

void ScopeChannel::SpectrumData::initSetup() {
  destroySetup();
  in = (float*)pffft_aligned_malloc(bins*sizeof(float));
  if (!in) return;
  out = (float*)pffft_aligned_malloc(bins*sizeof(float));
  if (!out) return;
  work = (float*)pffft_aligned_malloc(bins*sizeof(float));
  if (!work) return;
  setup = pffft_new_setup(bins, PFFFT_REAL);
  if (!setup) return;
  updateSetup=false;
}

void ScopeChannel::SpectrumData::destroySetup() {
  if (in) pffft_aligned_free(in);
  if (out) pffft_aligned_free(out);
  if (work) pffft_aligned_free(work);
  if (setup) pffft_destroy_setup(setup);
}
// pffft_transform(fft.setup, fft.in, fft.out, fft.work, PFFFT_FORWARD);

void ScopeChannel::SpectrumData::process(DataBuffer* buf, nint needle) {
  for (nint i=0, j=buf->getSize()-bins; i<bins; i++,j++) {
    in[i] = buf->getValueScaled(j)*windowFunction((float)i/(float)(bins-1),0.0f);
  }
  pffft_transform(setup,in,out,work,PFFFT_FORWARD);
}

void ScopeChannel::drawSpectrum(ImRect rect, ImDrawList* dl) {
  // if (!fft.isOK) return;
  if (fft.updateSetup) {
    fft.initSetup();
    if (fft.updateSetup) {
      fft.isOK = false;
      printf(ERROR_MSG "failed to init fft!" MSG_END);
    }
    return;
  }
  fft.process(buffer,0);

  ImVec2* plot = new ImVec2[fft.bins/2];
  float v=0.0f, f=0.0f;
  size_t n=0;
  for (size_t i=0; i<fft.bins/2; i++,n++) {
    v = 2.0f*sqrtf(fft.out[i<<1]*fft.out[i<<1]+fft.out[(i<<1)+1]*fft.out[(i<<1)+1])/fft.bins;
    f = invLerp(fft.minFreq/buffer->getSampleRate()*2.0f, fft.maxFreq/buffer->getSampleRate()*2.0f, (float)i/(fft.bins/2.0f));
    plot[i] = {
      ImLerp(rect.Min.x, rect.Max.x, f),
      ImLerp(rect.Max.y, rect.Min.y, invLerp(fft.minV, fft.maxV, v))
    };
    if (plot[i].x>rect.Max.x) break;
  }
  dl->AddPolyline(plot, n, ImGui::ColorConvertFloat4ToU32(color), 0, 1.0f);
}

void ScopeChannel::loadFromNode(YAML::Node node) {
  for (int i=0; i<params.size(); i++) {
    params[i].readFromConfig(node);
  }

  if (buffer)
    samples = msToSamples(timeScale, buffer->getSampleRate());

  color = ImGui::ColorConvertU32ToFloat4(node["color"].as<unsigned int>());
  showWaveform = node["showWaveform"].as<bool>();
  showControls = node["showControls"].as<bool>();

  if (node["buffer"].IsDefined()) {
    WhichBuffer newBuf;
    newBuf.driver = node["buffer"][0].as<int>();
    newBuf.driverBuffer = node["buffer"][1].as<int>();

    setBuffer(parent->data->getDriver(newBuf.driver)->getBuffer(newBuf.driverBuffer), newBuf);
  }

  if (node["trigger"].IsDefined() && buffer) {
    YAML::Node triggerNode = node["trigger"];
    if (triggerNode["stealFrom"].IsDefined()) {
      setTrigger(triggerNode["stealFrom"].as<int>());
    } else {
      setTriger((Triggers)triggerNode["num"].as<int>());
      for (int i=0; i<trigger->getParams()->size(); i++) {
        trigger->getParams()->at(i).readFromConfig(triggerNode);
      }
    }
  }

}

YAML::Node ScopeChannel::saveToNode() {
  YAML::Node node;

  for (int i=0; i<params.size(); i++) {
    params[i].writeToConfig(node);
  }

  node["color"] = ImGui::ColorConvertFloat4ToU32(color);
  node["showWaveform"] = showWaveform;
  node["showControls"] = showControls;

  if (buffer) {
    node["buffer"].push_back(bufferNum.driver);
    node["buffer"].push_back(bufferNum.driverBuffer);
  }

  YAML::Node triggerNode;
  if (trigger) {
    triggerNode["num"] = (int)triggerNum;
    for (int i=0; i<trigger->getParams()->size(); i++) {
      trigger->getParams()->at(i).writeToConfig(triggerNode);
    }
    node["trigger"] = triggerNode;
  } else if (triggerNum==TRIG_STOLEN) {
    triggerNode["stealFrom"] = chanOfTrigger;
    node["trigger"] = triggerNode;  
  }

  return node;
}

ScopeChannel::~ScopeChannel() {
  deleteTrigger();
}

vector<Parameter>* ScopeChannel::getParams() {
  return &params;
}

// ScopeWindow

void ScopeWindow::drawTextCentered(ImRect rect, ImDrawList* dl, const char* text) {
  ImVec2 textSize = ImGui::CalcTextSize(text)/2.0f;
  dl->AddText(rect.GetCenter() - textSize, ImGui::GetColorU32(ImGuiCol_Text), text);
}

void ScopeWindow::drawScopeGrid(ImRect rect, ImDrawList* dl, unsigned int xDiv, unsigned int yDiv, ImU32 color, plotScaleFunction xScale, plotScaleFunction yScale) {
  ImVec2 p1, p2;
  if (xScale==NULL) xScale=&scaleLinear;
  if (yScale==NULL) yScale=&scaleLinear;
  FOR_RANGE(xDiv-1) {
    // veritcal lines
    p1.x = p2.x = xScale(ImLerp(rect.Min.x, rect.Max.x, (z+1)/(float)xDiv));
    p1.y = rect.Min.y;
    p2.y = rect.Max.y;
    dl->AddLine(p1, p2, color);
  }
  FOR_RANGE(yDiv-1) {
    // horizontal lines
    p1.y = p2.y = yScale(ImLerp(rect.Min.y, rect.Max.y, (z+1)/(float)yDiv));
    p1.x = rect.Min.x;
    p2.x = rect.Max.x;
    dl->AddLine(p1, p2, color);
  }
}

bool ScopeWindow::plotDragX(float* v, const char* label, ImDrawList* dl, ImVec4 rect, ImU32 col, float v_min, float v_max) {
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

bool ScopeWindow::plotDragY(float* v, const char* label, ImDrawList* dl, ImVec4 rect, ImU32 col, float v_min, float v_max) {
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

void ScopeWindow::drawScopeWaveform(ImRect rect, ImDrawList* dl) {
  if (channels.empty()) {
    drawTextCentered(rect, dl, "no channels!");
    return;
  }
  // dl->AddRect(rect.Min, rect.Max, 0xff00ff00);
  const int chans = channels.size();
  if (primaryChan > chans-1) primaryChan = chans-1;

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
      float v = -(((signed char)z-4)/5.f + channels[primaryChan]->yOffset) / channels[primaryChan]->yScale;
      snprintf(buf, 16, "%+1.3f", v);
      dl->AddText(p1, ImGui::GetColorU32(ImGuiCol_Text), buf);
    }
    rect.Min.x += textSize.x;
    rect.Max.x -= textSize.x; // precalc for sec. chan
  }

  // grid
  if (drawGrid) {
    drawScopeGrid(rect, dl, 10, 10);
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
  for (int c=0; c<channels.size(); c++) {
    ScopeChannel* chan=channels[c];
    if (chan->getTriggerNum()==TRIG_STOLEN) continue;
    if (chan->getTriggerNum()==TRIG_SMOOTH) {
      if (chan->getTrigger()->getParams()->at(0).isHovered() || chan->getTrigger()->getParams()->at(0).isActive()) {
        nint len = chan->getSamples();
        ImVec2* scaledWave = new ImVec2[len];
        float* smoothBuf=((TriggerSmooth*)chan->getTrigger())->getSmoothBuffer();
        nint n=0, cur = chan->getBuffer()->getSize() - len;
        if (chan->getTrigger()->getTriggered())
          cur = chan->getTrigger()->getTriggerIndex();
        for (; n < len; cur++, n++) {
          if (cur > chan->getBuffer()->getSize()) break;
          scaledWave[n] = {
            ImLerp(rect.Min.x, rect.Max.x, (float)n/(float)len),
            ImLerp(rect.GetCenter().y, rect.Min.y, smoothBuf[cur]*chan->yScale+chan->yOffset)
          };
        }
        dl->AddPolyline(scaledWave, n, 0xffff66ff, 0, 1.0f);
        delete[] scaledWave;
      }
      // if (chan->getTrigger()->getParams()->at(1).isHovered()) {
      //   float level=((TriggerSmooth*)(chan->getTrigger()))->getTriggerLevel();
      //   level=origin.y+(level+1.0f)*size.y/2.0f;
      //   dl->AddLine(
      //     ImVec2(origin.x,level), 
      //     ImVec2(origin.x+size.x, level),
      //     0xff0077ff
      //   );
      // }
    }
  }
  // v  scale labels (sec. chan)
  if (drawScaleY && !channels.empty()) {
    ImVec2 p1, textSize = ImGui::CalcTextSize("-1.000");
    // textSize.x += 5.f;
    char buf[16];
    snprintf(buf, 16, " CH.%.2d", secondaryChan+1);
    p1.y = rect.Min.y + 5.f;
    p1.x = rect.Max.x + 5.f;
    dl->AddText(p1, ImGui::GetColorU32(ImGuiCol_Text), buf);
    FOR_RANGE(9) {
      p1.y = ImLerp(rect.Min.y, rect.Max.y, (z+1)/10.f) - textSize.y/2;
      float v = -(((signed char)z-4)/5.f + channels[secondaryChan]->yOffset) / channels[secondaryChan]->yScale;
      snprintf(buf, 16, "%+1.3f", v);
      dl->AddText(p1, ImGui::GetColorU32(ImGuiCol_Text), buf);
    }
  }

}

void ScopeWindow::drawXY(ImRect rect, ImDrawList* dl) {
  if (channels.size()<2) {
    drawTextCentered(rect, dl, "not enough channels!");
    return;
  }
  if (primaryChan   < 0 || primaryChan   >= channels.size()
   || secondaryChan < 0 || secondaryChan >= channels.size()) return;
  DataBuffer* xBuf = channels[primaryChan]->getBuffer(),
            * yBuf = channels[secondaryChan]->getBuffer();
  if (xBuf == NULL || yBuf == NULL) return;
  ImVec2* points = new ImVec2[xySamples];
  nint needle = xBuf->getSize() - xySamples;
  for (nint i=0; i<xySamples; i++, needle++) {
    points[i] = {
      ImLerp(rect.GetCenter().x, rect.Max.x, xBuf->getValueScaled(needle)),
      ImLerp(rect.GetCenter().y, rect.Min.y, yBuf->getValueScaled(needle)),
    };
  }
  dl->AddPolyline(points, xySamples,
    ImGui::GetColorU32(channels[primaryChan]->color), 0, 1.0f);

  delete[] points;
}

void ScopeWindow::drawScopeWindow(bool scopeContainChannelControls, bool showDisabledBuffers) {
  if (!windowOpen) return;
  char strbuf[256];
  snprintf(strbuf, 256, "Scope %d", index+1);
  if (ImGui::Begin(strbuf, &windowOpen)) {
    int columns=2; float scopeWidth=1.0f;
    if (scopeContainChannelControls && showChannelControls) {
      columns++; scopeWidth-=.25f;
    }
    if (showWindowControls) {
      columns++; scopeWidth-=.25f;
    }
    if (ImGui::BeginTable("scopeWin", columns, ImGuiTableFlags_Resizable)) {
      if (showWindowControls) {
        ImGui::TableSetupColumn("wc", ImGuiTableColumnFlags_WidthStretch, 0.25f);
      }
        ImGui::TableSetupColumn("sc", ImGuiTableColumnFlags_WidthStretch, scopeWidth);
        ImGui::TableSetupColumn("cc", ImGuiTableColumnFlags_WidthFixed);
      if (scopeContainChannelControls && showChannelControls) {
        ImGui::TableSetupColumn("cc", ImGuiTableColumnFlags_WidthStretch, 0.25f);
      }
      ImGui::TableNextRow();
      ImGui::TableNextColumn();
      if (showWindowControls) {
        // ImGui::InputInt("primary channel", &primaryChan);
        // ImGui::InputInt("secondary channel", &secondaryChan);
        char strbuf[512];
        snprintf(strbuf, 512, "channel %d", primaryChan+1);
        if (ImGui::BeginCombo("primary channel", strbuf)) {
          for (int i=0; i<channels.size(); i++) {
            snprintf(strbuf, 512, "channel %d (%s)", i+1, channels[i]->getBuffer()->getName().c_str());
            if (ImGui::Selectable(strbuf, primaryChan==i)) {
              primaryChan = i;
            }
          }
          ImGui::EndCombo();
        }
        snprintf(strbuf, 512, "channel %d", secondaryChan+1);
        if (ImGui::BeginCombo("secondary channel", strbuf)) {
          for (int i=0; i<channels.size(); i++) {
            snprintf(strbuf, 512, "channel %d (%s)", i+1, channels[i]->getBuffer()->getName().c_str());
            if (ImGui::Selectable(strbuf, secondaryChan==i)) {
              secondaryChan = i;
            }
          }
          ImGui::EndCombo();
        }
        if (ImGui::BeginCombo("mode", scopeModes[scopeMode])) {
          for (int i=0; i<3; i++) {
            if (ImGui::Selectable(scopeModes[i], scopeMode == i)) {
              scopeMode = (ScopeMode)i;
            }
          }
          ImGui::EndCombo();
        }
        if (ImGui::BeginTable("Scopes", 6)) {
          ImGui::TableSetupColumn("c1", ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("c2", ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("c3", ImGuiTableColumnFlags_WidthStretch);
          ImGui::TableSetupColumn("c4", ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("c5", ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableSetupColumn("c6", ImGuiTableColumnFlags_WidthFixed);
          ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
          ImGui::TableNextColumn();
          ImGui::TableNextColumn();
          ImGui::Text("buffer");
          ImGui::TableNextColumn();
          ImGui::Text("trigger");
          ImGui::TableNextColumn();
          for (int i=0; i<channels.size(); i++) {
            ImGui::PushID(i);
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            // remove button
            if (ImGui::Button("×##scopeRemove")) {
              auto whichWinChan = channels.begin();
              std::advance(whichWinChan, i);
              channels.erase(whichWinChan);
              ImGui::PopID();
              continue;
            }
            ImGui::TableNextColumn();
            // buffer list
            DataBuffer* curBuf = channels.at(i)->getBuffer();
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::BeginCombo("##scopeBufferSelect", curBuf?curBuf->getName().c_str():"<none>")) {
              ImGui::BeginDisabled();
              ImGui::Selectable("select a buffer...");
              ImGui::EndDisabled();
              ImGui::Separator();
              char comboStrBuf[1024];
              int m=0;
              for (int j=0; j<data->getDriverCount(); j++) {
                for (int k=0; k<data->getDriver(j)->getBufferCount(); k++) {
                  DataBuffer* buf = data->getDriver(j)->getBuffer(k);
                  snprintf(comboStrBuf, 1024, "%d: %s - %s", 1+m++, data->getDriver(j)->getDriverInfo().name, buf->getName().c_str());
                  if (!buf->isInited() && !showDisabledBuffers) continue;
                  ImGui::BeginDisabled(!buf->isInited());
                  if (ImGui::Selectable(comboStrBuf, channels.at(i)->getBuffer() == buf)) {
                    if (channels.at(i)->setBuffer(buf, {j,k}))
                      printf(ERROR_MSG "GUI: failed to set buffer for channel %d!" MSG_END, i);
                  }
                  ImGui::EndDisabled();
                }
              }
              ImGui::EndCombo();
            }
            ImGui::TableNextColumn();
            // trigger list
            ImGui::BeginDisabled(curBuf==NULL);
            const char* comboPreview = triggerNames[channels.at(i)->getTriggerNum()];
            char strbuf[512];
            if (channels[i]->getTriggerChan()!=-1) {
              snprintf(strbuf, 512, "CH.%.2d", channels[i]->getTriggerChan()+1);
              comboPreview = strbuf;
            }
            ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
            if (ImGui::BeginCombo("##scopeTriggerSelect", comboPreview)) {
              for (int j=0; j<TRIG_MAX; j++) {
                snprintf(strbuf, 512, "New %s", triggerNames[j]);
                if (ImGui::Selectable(strbuf, j==channels.at(i)->getTriggerNum())) {
                  channels.at(i)->setTriger((Triggers)j);
                }
              }
              ImGui::BeginDisabled();
              ImGui::Selectable("trigger from channel:");
              ImGui::EndDisabled();
              for (int j=0; j<channels.size(); j++) {
                if (j==i) continue;
                if (channels[j]->getTriggerNum()==TRIG_STOLEN) continue;
                DataBuffer* own = channels[i]->getBuffer(),
                          * target = channels[j]->getBuffer();
                if (own->getSampleRate() != target->getSampleRate()) continue;
                if (own->getSize() != target->getSize()) continue;

                snprintf(strbuf, 512, "channel %d (%s)", j+1, target->getName().c_str());
                if (ImGui::Selectable(strbuf, channels[i]->getTriggerChan()==j)) {
                  channels[i]->setTrigger((int)j);
                }
              }
              ImGui::EndCombo();
            }
            ImGui::EndDisabled();
            ImGui::TableNextColumn();
            // show waveform
            ImGui::Checkbox("##scopeShowWaveform", &(channels.at(i)->showWaveform));
            if (ImGui::IsItemHovered())
              ImGui::SetTooltip("show waveform");
            ImGui::TableNextColumn();
            // show controls
            ImGui::Checkbox("##scopeShowControls", &(channels.at(i)->showControls));
            if (ImGui::IsItemHovered())
              ImGui::SetTooltip("show controls");
            ImGui::TableNextColumn();
            // color picker
            ImGui::ColorButton("color", channels.at(i)->color);
            if (ImGui::BeginPopupContextItem("##xycol",ImGuiPopupFlags_MouseButtonLeft)) {
              ImGui::ColorPicker4("##xycoledit",(float*)&channels.at(i)->color);
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
                  snprintf(comboStrBuf, 256, "%d: %s - %s", m++, data->getDriver(j)->getDriverInfo().name, buf->getName().c_str());
                  if (ImGui::Selectable(comboStrBuf, newScopeBuffer == buf)) {
                    newScopeBuffer = buf;
                    newBufferNum = {j,k};
                  }
                }
              }
              ImGui::EndCombo();
            }
            if (ImGui::Button("Add Scope")) {
              ScopeChannel* scope = new ScopeChannel(this,newScopeBuffer, newBufferNum);
              channels.push_back(scope);
              newScopeBuffer = NULL;
              newBufferNum = {0,0};
              ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
          }
          ImGui::EndTable();
        }
        ImGui::TableNextColumn();
      }
      ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f,0.0f));
      if (ImGui::BeginChild("waveform", ImGui::GetContentRegionAvail())) {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 origin = ImGui::GetCursorScreenPos(), size = ImGui::GetContentRegionAvail();
        ImRect rect(origin, origin+size);

        switch (scopeMode) {
          case SCOPE_XY:
            if (true) { // squareView
              float sizeMin = (size.x<size.y?size.x:size.y)/2.0f;
              rect = {
                rect.GetCenter().x - sizeMin,
                rect.GetCenter().y - sizeMin,
                rect.GetCenter().x + sizeMin,
                rect.GetCenter().y + sizeMin
              };
            }
            drawXY(rect, dl);
            if (drawGrid) {
              drawScopeGrid(rect, dl, 10, 10);
            }
            break;
          case SCOPE_SPECTRUM:
            for (int i=0; i<channels.size(); i++)
              channels[i]->drawSpectrum(rect, dl);
            drawScopeGrid(rect, dl, 10, 10, 0x44ffffff);
            break;
          case SCOPE_SCOPE:
          default:
            drawScopeWaveform(rect, dl);
            break;
        }
        // dl->AddRect(rect.Min, rect.Max, -1);
        // no function pointers... yet
        // drawFunc(rect, dl);

        const ImVec2 buttonSize = ImGui::GetStyle().FramePadding*2.0f + ImGui::CalcTextSize(ICON_FAD_CARET_LEFT);
        ImGui::SetCursorPosY(rect.GetHeight()-buttonSize.y);
        if (ImGui::Button(showWindowControls?ICON_FAD_CARET_LEFT "##winCtrl":ICON_FAD_CARET_RIGHT "##winCtrl"))
          showWindowControls=!showWindowControls;
        if (ImGui::IsItemHovered())
          ImGui::SetTooltip("show window controls");
        if (scopeContainChannelControls) {
          ImGui::SameLine();
          ImGui::SetCursorPosX(rect.GetWidth()-buttonSize.x);
          if (ImGui::Button(showChannelControls?ICON_FAD_CARET_RIGHT "##chanCtrl":ICON_FAD_CARET_LEFT "##chanCtrl"))
            showChannelControls=!showChannelControls;
          if (ImGui::IsItemHovered())
            ImGui::SetTooltip("show channel controls");
        }
      }
      ImGui::EndChild();  
      ImGui::PopStyleVar();
      ImGui::TableNextColumn();
      {
        if (ImGui::Button(ICON_FAD_REDO "##chanArrReset")) {
          const int chans = channels.size();
          for (int i=0; i<chans; i++) {
            channels[i]->yScale = 1.0f;
            channels[i]->yOffset = 0.0f;
          }
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("reset channel arrangement");
        }
        if (ImGui::Button(ICON_FAD_H_EXPAND "##chanArrStack")) {
          const int chans = channels.size();
          float scale = 1.0f/chans;
          for (int i=0; i<chans; i++) {
            channels[i]->yScale = scale;
            channels[i]->yOffset = 1.0f - (i*2.0f+1)*scale;
          }
        }
        if (ImGui::IsItemHovered()) {
          ImGui::SetTooltip("stack channels");
        }
      }
      if (scopeContainChannelControls && showChannelControls) {
        ImGui::TableNextColumn();
        char strbuf[128];
        for (int i=0; i<channels.size(); i++) {
          snprintf(strbuf, 128, "channel %d", i+1);
          ImGui::PushStyleColor(ImGuiCol_Text, channels[i]->color);
          bool colorPopped=false;
          if (ImGui::TreeNode(strbuf)) {
            ImGui::PopStyleColor(); colorPopped=true;
            channels[i]->drawParams();
            ImGui::TreePop();
          }
          if (!colorPopped) ImGui::PopStyleColor();
        }
      }
      ImGui::EndTable();
    }
  }
  ImGui::End();
}

void ScopeWindow::loadFromNode(YAML::Node node) {
  primaryChan = node["primaryChan"].as<int>();
  secondaryChan = node["secondaryChan"].as<int>();

  scopeMode = (ScopeMode)node["scopeMode"].as<int>();
  xyPersistence = node["xyPersistence"].as<float>();

  drawGrid = node["drawGrid"].as<bool>();
  drawScaleY = node["drawScaleY"].as<bool>();
  showHCursors = node["showHCursors"].as<bool>();
  showVCursors = node["showVCursors"].as<bool>();
  showChannelControls = node["showChannelControls"].as<bool>();
  showWindowControls = node["showWindowControls"].as<bool>();

  char strbuf[256];
  snprintf(strbuf, 256, "chan0");
  for (int i=1; node[strbuf].IsDefined(); i++) {
    ScopeChannel* chan = new ScopeChannel(this, NULL, {0,0});
    chan->loadFromNode(node[strbuf]);
    channels.push_back(chan);
    snprintf(strbuf, 256, "chan%d", i);
  }
}

YAML::Node ScopeWindow::saveToNode() {
  YAML::Node node;
  node["primaryChan"] = primaryChan;
  node["secondaryChan"] = secondaryChan;

  node["scopeMode"] = (int)scopeMode;
  node["xyPersistence"] = xyPersistence;

  node["drawGrid"] = drawGrid;
  node["drawScaleY"] = drawScaleY;
  node["showHCursors"] = showHCursors;
  node["showVCursors"] = showVCursors;
  node["showChannelControls"] = showChannelControls;
  node["showWindowControls"] = showWindowControls;

  char strbuf[256];
  for (int i=0; i<channels.size(); i++) {
    snprintf(strbuf, 256, "chan%d", i);
    node[strbuf] = channels[i]->saveToNode();
  }

  return node;
}
