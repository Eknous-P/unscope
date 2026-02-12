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

#ifndef USC_GUI_H
#define USC_GUI_H

#include <SDL.h>
#include "showcqt.h"
#include "pffft.h"
#include "imgui.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"

#include "shared.h"
#include "data.h"

#define INIFILE "unscope.ini"

class USCRender {
  SDL_Window* win;
  int winFlags;
  SDL_Event event;
  public:
    virtual int initRender();
    virtual int setupRender(int _winFlags, const char* winName, int winX, int winY, int winW, int winH);
    virtual int renderPreLoop();
    virtual int renderPostLoop();
    virtual void destroyRender();

    virtual SDL_Window* getWindow();
    virtual ~USCRender();
};

// renderers
#include "render/render_sdl2renderer.h"
#ifdef USE_OPENGL
#include "render/render_opengl2.h"
#endif
#ifdef USE_DIRECTX9
#include "render/render_directx9.h"
#endif
#ifdef USE_DIRECTX11
#include "render/render_directx11.h"
#endif

#include "trigger.h"
#include "fallback.h"
#include "analog.h"
#include "smooth.h"

inline nint msToSamples(float ms, double sampleRate);

class USCGUI {
  private:
    struct settings {
      bool msDiv;
    } settings;

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

    class ScopeChannel {
      private:
        DataBuffer* buffer;
        Trigger* trigger;
        spectrumData* spectrum;
        nint samples, needle;
        float lineSize;
        Triggers triggerNum;
      public:
        float xOffset, yOffset;
        float timeScale, yScale;
        ImVec4 color;
        bool showWaveform, showControls;
        int whichWindow;

        ScopeChannel();
        ScopeChannel(DataBuffer* buf);

        DataBuffer* getBuffer();
        Trigger* getTrigger();
        Triggers getTriggerNum();
#ifdef PROGRAM_DEBUG
        nint getNeedle();
#endif

        int setBuffer(DataBuffer* buf);
        int setTriger(Triggers which);

        void drawWaveform(ImRect rect, ImDrawList* dl);
        void drawSpectrum(ImRect rect, ImDrawList* dl);
        void drawControls(bool trigControlsDisabled);

        ~ScopeChannel();
    };
    vector<ScopeChannel*> scopes;

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

    struct ScopeWindow {
      int index, primaryChan;
      vector<ScopeChannel*> channels;
      bool windowOpen;
      bool drawGrid, drawScaleY;
      bool showHCursors, showVCursors;
      bool showChannelControls, showWindowControls;

      plotCursor HCursors[2], VCursors[2];

      // TODO: what the fuck is this use ImRect dumbass     ------v
      bool plotDragX(float* v, const char* label, ImDrawList* dl, ImVec4 rect, ImU32 col, float v_min=-1.f, float v_max=1.f);
      bool plotDragY(float* v, const char* label, ImDrawList* dl, ImVec4 rect, ImU32 col, float v_min=-1.f, float v_max=1.f);

      void drawScopeWaveform(ImRect rect, ImDrawList* dl);
      void drawScope(bool scopeContainChannelControls);

      ScopeWindow():
        index(0),
        primaryChan(0),
        channels({}),
        windowOpen(true),
        drawGrid(true),
        drawScaleY(true),
        showHCursors(false), showVCursors(false),
        showChannelControls(true), showWindowControls(false) {}
      ScopeWindow(int i):
        index(i),
        primaryChan(0),
        channels({}),
        windowOpen(true),
        drawGrid(true),
        drawScaleY(true),
        showHCursors(false), showVCursors(false),
        showChannelControls(true), showWindowControls(false) {
          HCursors[0]=plotCursor("X1",-.5f);
          HCursors[1]=plotCursor("X2",.5f);
          VCursors[0]=plotCursor("Y1",-.5f);
          VCursors[1]=plotCursor("Y2",.5f);
        }
    };
    vector<ScopeWindow> scopeWindows;

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
    } xyScope;

    struct windowsOpen {
      bool xyScopeOpen;
      bool xyScopeControlsOpen;
      bool globalControlsOpen;
      bool aboutOpen;
      bool cursorsOpen;
      bool audioConfigOpen;
      bool spectrumOpen;
      bool spectrumControlsOpen;
#ifdef PROGRAM_DEBUG
      bool metricsOpen;
      bool paramDebugOpen;
      bool triggerDebugOpen;
      bool fftDebugOpen;
#endif
    } wo;

    DataBuffer_Float dummyBuffer;

    vector<int> fftFrequencies;

    bool fullscreen;
    bool scopeContainChannelControls;
    ImGuiStyle style;
    ImVec4 bgColor;
    bool isGood, running;
    unsigned char channels;
    USCRenderers renderer;
  
    USCRender* rd;
    USCData* data;

    bool shareParams;
    signed int shareTrigger; // abs part - which channel, sign - do/don't

    bool doFallback, singleShot;

    ImVec4 topKeyColor, bottomKeyColor;
    ImVec4* pianoColors[12];

    DataBuffer* newScopeBuffer;
    DataDrivers newDriver;
    int newScopeWin;

    // void generateFFTFrequencies();

#ifdef PROGRAM_DEBUG
    int debuggedChannel;
    double fftPeak;
#endif
    string errorText;

    void doFullscreen();

    void setupRenderer(USCRenderers r);

  // windows:
    void drawXYScope(bool* open);
    void drawSpectrum(bool* open);

    void drawChannelManager(bool* open);
    void drawChanControls();
    void drawXYScopeControls(bool* open);
    void drawSpectrumControls(bool* open);

    void drawAbout(bool* open);
    void drawSettings(bool* open);
    void drawAudioConfig(bool* open);

    void errorPopup(const char* errorTxt, ...);

    // void drawScopeDebugImFrustrated();

#ifdef PROGRAM_DEBUG
    void drawTriggerDebug(bool* open);
    // void drawParamDebug(bool* open);
    // void drawFFTDebug(bool* open);
#endif

  public:
    void attachData(USCData* i);
  
    int  init(USCRenderers rend);
    bool isRunning();
    void doFrame();
    void drawGUI();
  
    USCGUI();
    ~USCGUI();
};

extern const unsigned char step_one;
extern const char *windowLayout;
extern const char *dataDriverNames[DATA_MAX];
extern const char *triggerNames[];
extern const char *const spectrumModes[];

#endif