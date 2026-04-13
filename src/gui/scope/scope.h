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

#ifndef _SCOPE_H
#define _SCOPE_H

#include "data.h"

#include "showcqt.h"
#include "pffft.h"

#include "trigger.h"
#include "analog.h"
#include "fallback.h"
#include "smooth.h"

#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"
#include <cstddef>

typedef float(*spectrumWindowFunction)(float,float);
typedef float(*plotScaleFunction)(float);

class ScopeChannel;

class spectrumGetter {
  protected:
    vector<Parameter> params;
    ScopeChannel* chan;
    float* output;

    float minFreq, maxFreq;
  public:
    virtual void setup(ScopeChannel* _chan);
    virtual void process();
    virtual void drawParams();
    virtual float* getOutput();
};

class spectrumFFT : public spectrumGetter {
  private:
    float* work;
    PFFFT_Setup* setup;
    bool updateSetup;
};

struct spectrumData {
  float *in, *out, *work;
  PFFFT_Setup* setup;

  ShowCQT* cqt;
  
  bool updateSetup, running;

  unsigned int fftBins, cqtBins;
  // 0 - linear fft, 1 - constant-q
  int mode;
  // 0 - line, 1 - bar
  int plotType;
  float fftFreqMin, fftFreqMax, fftValMin, fftValMax;
  int cqtOctaveMin, cqtOctaveMax;
  /** 2 nibbles per axis
   * v | x axis | y axis
   * 0 | linear | linear
   * 1 | log    | dB
   */
  unsigned char scale;
  bool colorModulate;
  spectrumData():
    in(NULL),
    out(NULL),
    work(NULL),
    setup(NULL),
    cqt(NULL),
    updateSetup(false),
    running(false),
    fftBins(2048),
    cqtBins(1200),
    mode(0),
    plotType(0),
    fftFreqMin(0.0f), fftFreqMax(20000.0f),
    fftValMin(0.0f), fftValMax(1.0f),
    cqtOctaveMin(0), cqtOctaveMax(10),
    scale(0),
    colorModulate(false) {}
};

struct WhichBuffer {
  int driver;
  int driverBuffer;
};

class ScopeWindow;

class ScopeChannel {
  private:
    ScopeWindow* parent;
    DataBuffer* buffer;
    Trigger* trigger;
    vector<Parameter> params;
    nint samples, needle;
    WhichBuffer bufferNum;
    float lineSize;
    Triggers triggerNum;
    int chanOfTrigger;
  public:
    struct SpectrumData{
      spectrumWindowFunction windowFunction;
      PFFFT_Setup* setup;
      float *in, *out, *work;
      size_t bins;
      float minFreq, maxFreq;
      float minV, maxV;
      bool updateSetup, isOK;

      void initSetup();
      void destroySetup();

      void process(DataBuffer* buf, nint needle);
    } fft;
    float xOffset, yOffset;
    float timeScale, yScale;
    ImVec4 color;
    bool showWaveform, showControls;
    float timeScaleLimits[3];

    ScopeChannel();
    ScopeChannel(ScopeWindow* p, DataBuffer* buf, WhichBuffer num);

    DataBuffer* getBuffer();
    Trigger* getTrigger();
    Triggers getTriggerNum();
    int getTriggerChan();
#ifdef PROGRAM_DEBUG
    nint getNeedle();
#endif
    nint getSamples();

    int setBuffer(DataBuffer* buf, WhichBuffer num);
    int setTriger(Triggers which);
    int setTrigger(int chan);

    void drawWaveform(ImRect rect, ImDrawList* dl);
    void drawSpectrum(ImRect rect, ImDrawList* dl);
    void drawParams();
    vector<Parameter>* getParams();

    void deleteTrigger();

    void loadFromNode(YAML::Node node);
    YAML::Node saveToNode();

    ~ScopeChannel();
};

struct plotCursor {
  const char* label;
  float pos;
  plotCursor():
    label(NULL),
    pos(0.0f) {}
  plotCursor(const char* l, float p) {
    label = l;
    pos = p;
  }
};

enum ScopeMode : unsigned char {
  SCOPE_SCOPE,
  SCOPE_XY,
  SCOPE_SPECTRUM
};

struct ScopeWindow {
  USCData* data;
  DataBuffer* newScopeBuffer;
  WhichBuffer newBufferNum;
  int index, primaryChan;

  int secondaryChan; // for xy mode
  float xyPersistence, xyMaxLength;
  nint xySamples, xyMaxSamples;

  vector<ScopeChannel*> channels;
  bool windowOpen;

  ScopeMode scopeMode;
  bool drawGrid, drawScaleY;
  bool showHCursors, showVCursors;
  bool showChannelControls, showWindowControls;

  plotCursor HCursors[2], VCursors[2];

  void drawTextCentered(ImRect rect, ImDrawList* dl, const char* text);
  void drawScopeGrid(ImRect rect, ImDrawList* dl, unsigned int xDiv, unsigned int yDiv, ImU32 color=0x44ffffff, plotScaleFunction xScale=NULL, plotScaleFunction yScale=NULL);

  // TODO: what the fuck is this, use ImRect dumbass     -----v
  bool plotDragX(float* v, const char* label, ImDrawList* dl, ImVec4 rect, ImU32 col, float v_min=-1.f, float v_max=1.f);
  bool plotDragY(float* v, const char* label, ImDrawList* dl, ImVec4 rect, ImU32 col, float v_min=-1.f, float v_max=1.f);

  void drawScopeWaveform(ImRect rect, ImDrawList* dl);
  void drawXY(ImRect rect, ImDrawList* dl);

  // void (*drawFunc)(ImRect rect, ImDrawList* dl);

  void drawScopeWindow(bool scopeContainChannelControls, bool showDisabledBuffers);

  void loadFromNode(YAML::Node node);
  YAML::Node saveToNode();

  ScopeWindow():
    data(NULL),
    newScopeBuffer(NULL),
    index(0),
    primaryChan(0),
    secondaryChan(1),
    xyPersistence(20.0f), xyMaxLength(1000.0f),
    xySamples(500), xyMaxSamples(1000),
    channels({}),
    windowOpen(true),
    scopeMode(SCOPE_SCOPE),
    drawGrid(true),
    drawScaleY(true),
    showHCursors(false), showVCursors(false),
    showChannelControls(true), showWindowControls(true) {}
  ScopeWindow(USCData* d, int i):
    data(d),
    newScopeBuffer(NULL),
    index(i),
    primaryChan(0),
    secondaryChan(1),
    xyPersistence(20.0f), xyMaxLength(1000.0f),
    xySamples(500), xyMaxSamples(1000),
    channels({}),
    windowOpen(true),
    scopeMode(SCOPE_SCOPE),
    drawGrid(true),
    drawScaleY(true),
    showHCursors(false), showVCursors(false),
    showChannelControls(true), showWindowControls(true) {
      HCursors[0]=plotCursor("X1",-.5f);
      HCursors[1]=plotCursor("X2",.5f);
      VCursors[0]=plotCursor("Y1",-.5f);
      VCursors[1]=plotCursor("Y2",.5f);
    }
};

class XYScope {
  private:
    DataBuffer *xChan, *yChan;
    float xScale,  yScale;
    float xOffset, yOffset;
    ImVec4 color;
    nint  sampleLen;
    float persistence, lineSize;
    bool showControls, buffersOK;
  public:
    XYScope();
    XYScope(DataBuffer* x, DataBuffer* y);

    void setBuffers(DataBuffer* x, DataBuffer* y);

    void drawWaveform(ImRect rect, ImDrawList* dl);
    void drawControls(USCData* data);
};

extern const float yScaleKnobLimits[3];
extern const float lineSizeKnobLimits[3];

#endif
