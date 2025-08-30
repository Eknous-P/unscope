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
#include "imgui_internal.h"

#define UPDATE_X_SCALE sc.scale=(sc.scale&0xf0)|(xs&0xf);
#define UPDATE_Y_SCALE sc.scale=(sc.scale&0xf)|(ys<<4);
typedef float (*xScaleFunc)(float,float);
typedef float (*yScaleFunc)(float);

float xScaleFuncLinear(float x, float freq) {
  return x;
}

float xScaleFuncLog(float x, float freq) {
  const float base=100;
  return log((base-1)*x+1.0f)/log(base);
}

float yScaleFuncLinear(float y) {
  return y;
}

float yScaleFuncDb(float y) {
  return log10(y)*20.0f/70.0f+1;
}

xScaleFunc xScaleFunctions[]={
  xScaleFuncLinear,
  xScaleFuncLog
};

yScaleFunc yScaleFunctions[]={
  yScaleFuncLinear,
  yScaleFuncDb
};

void USCGUI::drawSpectrumControls(bool* open) {
  if (!oscData) return;
  if (!*open) return;
  if (ImGui::Begin("Spectrum Controls", open)) {
    if (ImGui::InputScalar("Bins", ImGuiDataType_U32, &sc.samples)) {
      if (sc.samples > oscDataSize) sc.samples = oscDataSize;
      FOR_RANGE(channels) sd[z].updateSetup=true;
    }
    if (ImGui::Combo("Spectrum Mode", &sc.mode, spectrumModes, 2)) {
      FOR_RANGE(channels) sd[z].updateSetup=true;
    }
    switch (sc.mode) {
      case 0: {
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
        break;
      }
      case 1: {
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
    ImGui::Text("Spectrum Colors:");
    ImGui::Indent();
    char buf[256];
    FOR_RANGE(channels) {
      ImGui::PushID(z);
      snprintf(buf, 256, "Channel %d", z+1);
      ImGui::ColorButton(buf, sd[z].color);
      if (ImGui::BeginPopupContextItem("##specCol",ImGuiPopupFlags_MouseButtonLeft)) {
        ImGui::ColorPicker4("##specColEdit",(float*)&sd[z].color);
        ImGui::EndPopup();
      }
      ImGui::SameLine();
      ImGui::Text("%s",buf);
      ImGui::PopID();
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
  origin.y += titleBar;
  size.y -= titleBar;
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
          snprintf(buf, 16, "%1.2f", (z+1)/10.0f);
          dl->AddText(p1, ImGui::GetColorU32(ImGuiCol_Text), buf);
        }
        break;
      }
      case 1: { // db
        textSize = ImGui::CalcTextSize("70dB");
        textSize.x += 5.f;
        FOR_RANGE(7) {
          p1.y = origin.y + size.y * ((z+1)/8.f) - textSize.y/2;
          p1.x = origin.x + padding;
          snprintf(buf, 16, "%2ddB", (z+1)*10);
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
      for (unsigned char z=1; z<((size.y>450)?20:10); z++) {
        dl->AddLine(
          origin+ImVec2(0,size.y*z/10),
          origin+ImVec2(size.x,size.y*z/10),
          0x44ffffff);
      }
      break;
    }
    case 1: { // db
      for (unsigned char z=1; z<((size.y>450)?16:8); z++) {
        dl->AddLine(
          origin+ImVec2(0,size.y*z/8),
          origin+ImVec2(size.x,size.y*z/8),
          0x44ffffff);
      }
      break;
    }
  }
  unsigned int samples = sc.samples;
  for (int z = channels-1; z>=0; z--) {
    if (sd[z].updateSetup) {
      sd[z].updateSetup=false;
      sd[z].running=true;
      if (sd[z].in) {
        pffft_aligned_free(sd[z].in);
        sd[z].in=NULL;
      }
      if (sd[z].out) {
        pffft_aligned_free(sd[z].out);
        sd[z].out=NULL;
      }
      if (sd[z].work) {
        pffft_aligned_free(sd[z].work);
        sd[z].work=NULL;
      }
      if (sd[z].setup) {
        pffft_destroy_setup(sd[z].setup);
        sd[z].setup=NULL;
      }
      sd[z].in = (float*)pffft_aligned_malloc(sizeof(float)*samples*2);
      if (!sd[z].in) sd[z].running=false;
      switch (sc.mode) {
        case 0: {
          sd[z].setup = pffft_new_setup(samples*2, PFFFT_REAL);
          if (!sd[z].setup) {
            sd[z].running=false;
          }
          sd[z].out  = (float*)pffft_aligned_malloc(sizeof(float)*samples*2);
          sd[z].work = (float*)pffft_aligned_malloc(sizeof(float)*samples*2);
          if (!(sd[z].out && sd[z].work)) sd[z].running=false;
          break;
        }
        case 1: {
          sd[z].running=false;
          break;
        }
        default:
          sd[z].running=false;
          break;
      }
    }
    
    if (sd[z].running) {
      nint cur=oscDataSize-samples*2;
      switch (sc.mode) {
        case 0:
          for (unsigned int i = 0; i < samples*2; i++) {
            float f = oscData[z][cur+i];
            sd[z].in[i] = f * 0.5 * (1 - cos(M_PI*i/samples));
          }
          pffft_transform_ordered(sd[z].setup, sd[z].in, sd[z].out, sd[z].work,PFFFT_FORWARD);
          break;
        case 1: {
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
      switch (sc.mode) {
        case 0: scaledPlot=new ImVec2[samples]; break;
        default:  break;
      }
        float mag=0.0f;
        switch (sc.mode) {
          case 0: {
            count=samples;
            for (unsigned int i=0; i<count; i++) {
              scaledPlot[i].x = origin.x + size.x*(xScaleFunctions[sc.scale&0xf])((float)i/samples, sampleRate/2.0f);
              mag = 2.0*sqrt(sd[z].out[i<<1] * sd[z].out[i<<1] + sd[z].out[(i<<1)+1] * sd[z].out[(i<<1)+1])/(float)samples;
              scaledPlot[i].y = origin.y + size.y - size.y*yScaleFunctions[sc.scale>>4](mag);
            }
            break;
          }
          case 1: {
            break;
          }
          default:
            break;
        }
#ifdef PROGRAM_DEBUG
        if (mag>fftPeak) fftPeak=mag;
#endif
      // dl->PushClipRectFullScreen();
      dl->AddPolyline(scaledPlot, count, ImGui::ColorConvertFloat4ToU32(sd[z].color), 0, 1.0f);
      // dl->PopClipRect();
      delete[] scaledPlot;
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
    if (ImGui::BeginTabBar("##fftDebugTab")) {
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
          for (int i=0; i<sc.samples; i++) {
            ImGui::TableNextRow();
            ImGui::TableNextColumn();
            ImGui::Text("%d",i);
            ImGui::TableNextColumn();
            ImGui::Text("%f",sd[0].in[i]);
            ImGui::TableNextColumn();
            ImGui::Text("%f",sd[0].out[i<<1]);
            ImGui::TableNextColumn();
            ImGui::Text("%f",sd[0].out[(i<<1)+1]);
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
        int samples=sc.samples*2;
        ImVec2* scaledPlot = new ImVec2[samples];
        for (nint i=0; i<samples; i++) {
          scaledPlot[i].x = origin.x + size.x*(i/(float)samples);
          scaledPlot[i].y = origin.y + size.y/2.0f - sd[0].in[i]*size.y/2.0f;
        }
        dl->AddPolyline(scaledPlot, samples, ImGui::ColorConvertFloat4ToU32(sd[0].color), 0, 1.0f);
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
