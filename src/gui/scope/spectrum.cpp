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
#include "imgui_internal.h"

#define UPDATE_X_SCALE sc.scale=(sc.scale&0xf0)|(xs&0xf); generateFFTFrequencies();
#define UPDATE_Y_SCALE sc.scale=(sc.scale&0xf)|(ys<<4);

typedef float (*scaleFunc)(float);

float scaleFuncLinear(float x) {
  return x;
}

float scaleFuncLog(float x) {
  const float base=100;
  return log((base-1)*x+1.0f)/log(base);
}

float scaleFuncDb(float y) {
  return log10(y)*20.0f/70.0f+1;
}

scaleFunc const xScaleFunctions[]={
  scaleFuncLinear,
  scaleFuncLog
};

scaleFunc const yScaleFunctions[]={
  scaleFuncLinear,
  scaleFuncDb
};

void USCGUI::generateFFTFrequencies() {
  const double sampleRate=48000;
  fftFrequencies.clear();
  switch (sc.scale&0xf) {
    case 0: {
      for (int i=0; i<sampleRate/2; i+=1000) {
        fftFrequencies.push_back(i);
      }
      break;
    }
    case 1: {
      int freq=0;
      for (int j=10; j<sampleRate/2; j*=10) {
        for (int i=1; i<10; i++) {
          freq = i*j;
          if (freq>sampleRate/2) break;
          fftFrequencies.push_back(freq);
        }
        if (freq>sampleRate/2) break;
      }
      break;
    }
    default: break;
  }
}

void USCGUI::drawSpectrumControls(bool* open) {
  if (!oscData) return;
  if (!*open) return;
  if (ImGui::Begin("Spectrum Controls", open)) {
    if (ImGui::Combo("Mode", &sc.mode, spectrumModes, 2)) {
      sd.updateSetup=true;
    }
    ImGui::Text("Plot:");
    ImGui::Indent();
    if (ImGui::RadioButton("Line", sc.plotType==0)) {
      sc.plotType=0;
    }
    if (ImGui::RadioButton("Bar", sc.plotType==1)) {
      sc.plotType=1;
    }
    ImGui::Unindent();
    if (sc.plotType==1)
      ImGui::Checkbox("Value affects brightness", &sc.colorModulate);
    ImGui::Separator();
    switch (sc.mode) {
      case 0: {
        if (wo.spectrumOpen && !sd.running) {
          ImGui::Text("invalid fft bin count! try %d", pffft_nearest_transform_size(sc.fftBins, PFFFT_REAL, 1));
          ImGui::PushStyleColor(ImGuiCol_Border, 0xff0000ff);
          ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
        }
        if (ImGui::InputScalar("Bins", ImGuiDataType_U32, &sc.fftBins)) {
          if (sc.fftBins > oscDataSize) sc.fftBins = oscDataSize;
          sd.updateSetup=true;
        }
        if (wo.spectrumOpen && !sd.running) {
          ImGui::PopStyleVar();
          ImGui::PopStyleColor();
        }
        ImGui::Text("X scale:");
        ImGui::Indent();
        unsigned char xs = sc.scale&0xf;
        if (ImGui::RadioButton("Linear##XSC", xs==0)) {
          xs=0;
          UPDATE_X_SCALE
        }
        if (ImGui::RadioButton("Logarithmic##XSC", xs==1)) {
          xs=1;
          UPDATE_X_SCALE
        }
        ImGui::Unindent();
        if (ImGui::SliderFloat("X Zoom", &sc.fftXZoom, 1.0f, 10.0f)) {
          if (sc.fftXZoom<1.0f) sc.fftXZoom=1.0f;
          if (sc.fftXZoom>10.0f) sc.fftXZoom=10.0f;
        }
        if (ImGui::SliderFloat("X Offset", &sc.fftXOffset, 0.0f, 1.0f)) {
          if (sc.fftXOffset<0.0f) sc.fftXOffset=0.0f;
          if (sc.fftXOffset>1.0f) sc.fftXOffset=1.0f;
        }
        break;
      }
      case 1: {
        if (ImGui::InputScalar("Width", ImGuiDataType_U32, &sc.cqtBins)) {
          if (sc.cqtBins > CQT_MAX_WIDTH) sc.cqtBins = CQT_MAX_WIDTH;
          sd.updateSetup=true;
        }
        if (ImGui::SliderInt("Octaves", &sc.cqtOctaves, 1, 10)) {
          if (sc.cqtOctaves<1) sc.cqtOctaves=1;
          if (sc.cqtOctaves>10) sc.cqtOctaves=10;
        }
        if (ImGui::SliderInt("Offset", &sc.cqtBaseNote, 0, 120)) {
          if (sc.cqtBaseNote<0) sc.cqtBaseNote=0;
          if (sc.cqtBaseNote>120) sc.cqtBaseNote=120;
        }
        break;
      }
      default:
        break;
    }
    ImGui::Text("Y scale:");
    ImGui::Indent();
    unsigned char ys = sc.scale>>4;
    if (ImGui::RadioButton("Linear##YSC", ys==0)) {
      ys=0;
      UPDATE_Y_SCALE
    }
    if (ImGui::RadioButton("dB##YSC", ys==1)) {
      ys=1;
      UPDATE_Y_SCALE
    }
    ImGui::Unindent();
  }
  ImGui::End();
}

void USCGUI::drawSpectrum(bool* open) {
  if (!oscData) return;
  if (!*open) return;
  ImGui::Begin("Spectrum",open);
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f,0.0f));
  ImDrawList* dl = ImGui::GetWindowDrawList();
  ImVec2 origin = ImGui::GetWindowPos(), size = ImGui::GetWindowSize();
  float titleBar = ImGui::GetCurrentWindow()->TitleBarHeight;
  float bottomTextSize=0.0f;
  origin.y += titleBar;
  size.y   -= titleBar;
  // x scale labels (precalc)
  {
    bottomTextSize=ImGui::CalcTextSize("1234567890k").y;
    size.y -= bottomTextSize;
  }
  // v scale labels
  {
    float padding = ImGui::GetStyle().FramePadding.x;
    ImVec2 p1, textSize;
    char buf[16];
    switch (sc.scale>>4) {
      case 0: { // linear
        textSize = ImGui::CalcTextSize("1.00");
        textSize.x += 5.f;
        FOR_RANGE(9) {
          p1.y = origin.y + size.y * ((z+1)/10.f) - textSize.y/2;
          p1.x = origin.x + padding;
          snprintf(buf, 16, "%1.2f", (9-z)/10.0f);
          dl->AddText(p1, ImGui::GetColorU32(ImGuiCol_Text), buf);
        }
        break;
      }
      case 1: { // db
        textSize = ImGui::CalcTextSize("-70");
        textSize.x += 5.f;
        FOR_RANGE(7) {
          p1.y = origin.y + size.y * ((z+1)/8.f) - textSize.y/2;
          p1.x = origin.x + padding;
          snprintf(buf, 16, "-%2d", (z+1)*10);
          dl->AddText(p1, ImGui::GetColorU32(ImGuiCol_Text), buf);
        }
        break;
      }
    }
    origin.x += textSize.x+padding;
    size.x   -= textSize.x+padding;
  }
  // y grid
  switch (sc.scale>>4) {
    case 0: { // linear
      unsigned char lines=((size.y>450)?20:10);
      for (unsigned char z=1; z<lines; z++) {
        dl->AddLine(
          origin+ImVec2(0,size.y*z/lines),
          origin+ImVec2(size.x,size.y*z/lines),
          0x44ffffff);
      }
      break;
    }
    case 1: { // db
      unsigned char lines=((size.y>450)?16:8);
      for (unsigned char z=1; z<lines; z++) {
        dl->AddLine(
          origin+ImVec2(0,size.y*z/lines),
          origin+ImVec2(size.x,size.y*z/lines),
          0x44ffffff);
      }
      break;
    }
  }
  // x grid
  switch (sc.mode) {
    case 0: {
      switch (sc.scale&0xf) {
        case 0: {
          float pos=0;
          for (size_t j=0; j<fftFrequencies.size(); j++) {
            int i=fftFrequencies[j];
            char buf[16];
            pos=sc.fftXZoom*size.x*(i/(sampleRate/2.0f)-sc.fftXOffset);
            if (pos>size.x) break;
            dl->AddLine(
              origin+ImVec2(pos, 0),
              origin+ImVec2(pos, size.y),
              0x44ffffff);
            {
              snprintf(buf, 16, "%dk", i/1000);
              float x=ImGui::CalcTextSize(buf).x;
              dl->AddText(
                origin+ImVec2(pos-x/2, size.y),
                ImGui::GetColorU32(ImGuiCol_Text),
                buf
              );
            }
          }
          break;
        }
        case 1: {
          float pos=0, prevPos=0, i;
          char buf[16];
          for (size_t j=0; j<fftFrequencies.size(); j++) {
            i=fftFrequencies[j];
            ImU32 color=0x22ffffff;
            pos=sc.fftXZoom*size.x*((xScaleFunctions[sc.scale&0xf])(i/(sampleRate/2.0f))-sc.fftXOffset);
            if (pos>size.x) break;
            if (j%9==0) {
              color=0x44ffffff;
            }
            { // if xlabel
              if (i>=1000) {
                snprintf(buf, 16, "%dk", (int)i/1000);
              } else {
                snprintf(buf, 16, "%d", (int)i);
              }
              float x=ImGui::CalcTextSize(buf).x;
              if (pos-prevPos > x || j%9==0) dl->AddText(
                origin+ImVec2(pos-x/2, size.y),
                ImGui::GetColorU32(ImGuiCol_Text),
                buf
              );
            }
            dl->AddLine(
              origin+ImVec2(pos, 0),
              origin+ImVec2(pos, size.y),
              color);
            prevPos=pos;
          }
          break;
        }
        default: break;
      }
      break;
    }
    case 1: {
      float x=0;
      const float max=12*sc.cqtOctaves, step=size.x/max;
      char buf[16];
      #define NOTE_OFFSET 4
      for (int z = sc.cqtBaseNote; z < sc.cqtOctaves*12+sc.cqtBaseNote; z++, x+=step) {
        ImU32 color=((z+NOTE_OFFSET)%12==0)?0x44ffffff:0x22ffffff;
        dl->AddLine(
          origin+ImVec2(x, 0.f),
          origin+ImVec2(x, size.y),
          color);
        dl->AddRectFilled(
          origin+ImVec2(x, 0.f),
          origin+ImVec2(x+step, size.y),
          ImGui::ColorConvertFloat4ToU32(*(pianoColors[(z+NOTE_OFFSET)%12]))
        );
        if ((z+NOTE_OFFSET)%12==0) { // if xlabel
          snprintf(buf, 16, "C%d", (z+NOTE_OFFSET)/12);
          float _x=ImGui::CalcTextSize(buf).x;
          dl->AddText(
            origin+ImVec2(x-_x/2, size.y),
            ImGui::GetColorU32(ImGuiCol_Text),
            buf
          );
        }
        if (x>origin.x+size.x) break;
      }
      #undef NOTE_OFFSET
      break;
    }
    default: break;
  }
  for (int z = channels-1; z>=0; z--) {
    if (sd.updateSetup) {
      printf(INFO_MSG "updating spectrum..." MSG_END);
      sd.updateSetup=false;
      sd.running=true;
      if (sd.in) {
        pffft_aligned_free(sd.in);
        sd.in=NULL;
      }
      if (sd.out) {
        pffft_aligned_free(sd.out);
        sd.out=NULL;
      }
      if (sd.work) {
        pffft_aligned_free(sd.work);
        sd.work=NULL;
      }
      if (sd.setup) {
        pffft_destroy_setup(sd.setup);
        sd.setup=NULL;
      }
      if (sd.cqt) {
        delete sd.cqt;
        sd.cqt=NULL;
      }
      sd.in = (float*)pffft_aligned_malloc(sizeof(float)*sc.fftBins*2);
      if (!sd.in) sd.running=false;
      switch (sc.mode) {
        case 0: {
          sd.setup = pffft_new_setup(sc.fftBins*2, PFFFT_REAL);
          if (!sd.setup) {
            printf(ERROR_MSG "failed to create pffft setup!" MSG_END);
            sd.running=false;
          }
          sd.out  = (float*)pffft_aligned_malloc(sizeof(float)*sc.fftBins*2);
          sd.work = (float*)pffft_aligned_malloc(sizeof(float)*sc.fftBins*2);
          if (!(sd.out && sd.work)) {
            printf(ERROR_MSG "failed to allocate fft buffers!" MSG_END);
            sd.running=false;
          }
          break;
        }
        case 1: {         
          sd.cqt = new ShowCQT;
          if (!sd.cqt) {
            printf(ERROR_MSG "failed to create cqt!" MSG_END);
            sd.running=false;
          }
          CQT_init(sd.cqt, sampleRate, sc.cqtBins, size.y, 0, 100, 0);
          break;
        }
        default:
          printf(ERROR_MSG "invalid mode!" MSG_END);
          sd.running=false;
          break;
      }
      if (sd.running) printf(SUCCESS_MSG "spectrum ready!" MSG_END);
    }
    
    if (sd.running) {
      switch (sc.mode) {
        case 0: {
          memset(sd.work, 0, sizeof(float)*sc.fftBins*2);
          nint cur=oscDataSize-sc.fftBins*2;
          for (unsigned int i = 0; i < sc.fftBins*2; i++) {
            float f = oscData[z][cur+i];
            sd.in[i] = f * 0.5 * (1 - cos(M_PI*i/sc.fftBins));
          }
          pffft_transform_ordered(sd.setup, sd.in, sd.out, sd.work,PFFFT_FORWARD);
          break;
        }
        case 1: {
          nint cur=oscDataSize-sd.cqt->fft_size;
          for (unsigned int i = 0; i < sd.cqt->fft_size; i++) {
            float f = oscData[z][cur+i];
            sd.cqt->input[0][i] = f * 0.5 * (1 - cos(M_PI*i/sd.cqt->fft_size));
            sd.cqt->input[1][i] = 0;
          }
          CQT_calc(sd.cqt);
          break;
        }
        default:
          break;
      }
      ImVec2* scaledPlot = NULL;
      unsigned int count=0;
#ifdef PROGRAM_DEBUG
      fftPeak=0.0;
#endif
      float mag=0.0f, y=0.0f;
      bool plotLine=sc.plotType==0, plotBar=sc.plotType==1;
      switch (sc.mode) {
        case 0:
          count=sc.fftBins;
          break;
        case 1:
          count=(sc.cqtBins/10.0f)*sc.cqtOctaves;
          break;
        default: break;
      }
      if (plotLine) {
        scaledPlot=new ImVec2[count];
        if (!scaledPlot) {
          printf(ERROR_MSG "failed to allocate line plot vectors (how?\?)!" MSG_END);
          plotLine=false;
        }
        memset(scaledPlot, 0, count*sizeof(ImVec2));
      }
      switch (sc.mode) {
        case 0: {
          for (unsigned int i=0; i<count; i++) {
            float curr = sc.fftXZoom*size.x*((xScaleFunctions[sc.scale&0xf])((float)i/count)-sc.fftXOffset),
                  next = sc.fftXZoom*size.x*((xScaleFunctions[sc.scale&0xf])((float)(i+1)/count)-sc.fftXOffset);
            mag = 2.0*sqrt(sd.out[i<<1] * sd.out[i<<1] + sd.out[(i<<1)+1] * sd.out[(i<<1)+1])/(float)count;
            y = 1.0-yScaleFunctions[sc.scale>>4](mag);
            if (plotLine) {
              scaledPlot[i].x = origin.x + curr;
              scaledPlot[i].y = origin.y + size.y*y;
            }
            if (plotBar){
              if (y>1.0f) continue;
              ImVec4 color=tc[z].color;
              if (sc.colorModulate)
                color.w*=1.0f-y;
              dl->AddRectFilled(
                origin+ImVec2(curr, size.y),
                origin+ImVec2(next, size.y*y),
                ImGui::ColorConvertFloat4ToU32(color)
              );
            }
          }
          break;
        }
        case 1: {
          const float step=size.x/count;
          const int offset=(sc.cqtBins/120.0f)*sc.cqtBaseNote;
          for (unsigned int i=0; i<count; i++) {
            if (i+offset>sd.cqt->t_size) {
              count=i;
              break;
            }
            y=1.0f-yScaleFunctions[sc.scale>>4](2.0f*sd.cqt->magOutput[i+offset]);
            if (plotLine) {
              scaledPlot[i].x = origin.x + step*i;
              scaledPlot[i].y = origin.y + size.y*y;
            }
            if (plotBar) {
              if (y>1.0f) continue;
              ImVec4 color=tc[z].color;
              if (sc.colorModulate)
                color.w*=1.0f-y;
              dl->AddRectFilled(
                origin+ImVec2(step*i, size.y),
                origin+ImVec2(step*(i+1), size.y*y),
                ImGui::ColorConvertFloat4ToU32(color)
              );
            }
          }
          break;
        }
        default:
          break;
      }
      if (plotLine) {
        ImGui::PushClipRect(origin, origin+size, true);
        dl->AddPolyline(scaledPlot, count, ImGui::ColorConvertFloat4ToU32(tc[z].color), 0, 1.0f);
        ImGui::PopClipRect();
        delete[] scaledPlot;
      }
    }
  }
  ImGui::PopStyleVar();
  // hover thing
  // {
  //   if (ImGui::IsWindowHovered()) {
  //     float mx = (ImGui::GetMousePos().x - origin.x)/size.x;
  //     int sample = samples*mx;
  //     float freq = sampleRate*scaleFunctions[sc.scale&0xf](mx);
  //     char buf[512];
  //     snprintf(buf,512,"sample: %d, freq: %f",sample, freq);
  //     dl->AddText(origin+ImVec2(10.f,10.f), ImGui::ColorConvertFloat4ToU32(ImGui::GetStyleColorVec4(ImGuiCol_Text)), buf);
  //   }
  // }
  ImGui::End();
}

#ifdef PROGRAM_DEBUG
void USCGUI::drawFFTDebug(bool* open) {
  if (!oscData) return;
  if (!*open) return;
  if (ImGui::Begin("FFT Debug", open)) {
    if (sd.setup) if (ImGui::BeginTabBar("##fftDebugTab")) {
      if (ImGui::BeginTabItem("raw data")) {
        ImGui::Text("fft peak: %f", fftPeak);
        if (ImGui::BeginTable("data", 4)) {
            ImGui::TableNextRow(ImGuiTableRowFlags_Headers);
            ImGui::TableNextColumn();
            ImGui::Text("i");
            ImGui::TableNextColumn();
            ImGui::Text("input");
            ImGui::TableNextColumn();
            ImGui::Text("real");
            ImGui::TableNextColumn();
            ImGui::Text("complex");
          for (int i=0; i<sc.fftBins; i++) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%d",i);
            ImGui::TableNextColumn();
            ImGui::Text("%f",sd.in[i]);
            ImGui::TableNextColumn();
            ImGui::Text("%f",sd.out[i<<1]);
            ImGui::TableNextColumn();
            ImGui::Text("%f",sd.out[(i<<1)+1]);
          }
          ImGui::EndTable();
        }
        ImGui::EndTabItem();
      }
      if (ImGui::BeginTabItem("fft input")) {
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0f,0.0f));
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 origin = ImGui::GetCurrentWindow()->DC.CursorPos, size = ImGui::GetContentRegionAvail();
        float titleBar = ImGui::GetCurrentWindow()->TitleBarHeight;
        origin.y += titleBar;
        size.y -= titleBar;
        int samples=sc.fftBins*2;
        ImVec2* scaledPlot = new ImVec2[samples];
        for (nint i=0; i<samples; i++) {
          scaledPlot[i].x = origin.x + size.x*(i/(float)samples);
          scaledPlot[i].y = origin.y + size.y/2.0f - sd.in[i]*size.y/2.0f;
        }
        dl->AddPolyline(scaledPlot, samples, ImGui::ColorConvertFloat4ToU32(tc[0].color), 0, 1.0f);
        delete[] scaledPlot;
        ImGui::PopStyleVar();
        ImGui::EndTabItem();
      }
      ImGui::EndTabBar();
    }
  }
  ImGui::End();
}
#endif
