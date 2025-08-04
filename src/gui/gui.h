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

#ifndef USC_GUI_H
#define USC_GUI_H

#include <SDL.h>
#include <imgui.h>
#include "imgui_stdlib.h"

#include "shared.h"
#include "audio.h"

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
#ifdef USE_OPENGL
#include "render/render_opengl2.h"
#endif
#ifdef USE_DIRECTX
#include "render/render_directx11.h"
#endif

#include "trigger.h"
#include "fallback.h"
#include "analog.h"
#include "smooth.h"

class USCGUI {
  private:
    struct settings {
      bool msDiv;
    } settings;

    struct traceParams {
      bool   enable;
      float  xOffset, yOffset;
      float  timebase, yScale;
      ImVec4 color;
      int    traceSize;
    };

    struct xyParams {
      float xScale,  yScale;
      float xOffset, yOffset;
      ImVec4 color;
      int   sampleLen;
      float persistence;
      unsigned char axisChan[2];
    };

    struct windowsOpen {
      bool mainScopeOpen;
      bool chanControlsOpen[3];
      bool xyScopeOpen;
      bool xyScopeControlsOpen;
      bool globalControlsOpen;
      bool aboutOpen;
      bool cursorsOpen;
      bool audioConfigOpen;
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

    bool fullscreen;
    ImGuiIO io;
    ImGuiStyle style;
    ImVec4 bgColor;
    bool isGood, running, updateAudio, restartAudio, audioLoopback, triggerSet;
    unsigned char channels;
    USCRenderers renderer;
  
    float **oscData;
    nint oscDataSize, sampleRate;
    USCRender *rd;
    USCAudio *ai;

    vector<AudioDevice>* devs;
    int inputDeviceS, outputDeviceS; // selectabe selection

    unscopeParams *up;
    AudioConfig   *audConf;
    traceParams   *tc;
    xyParams       xyp;
    windowsOpen    wo;

    bool showTrigger, shareParams;
    signed char shareTrigger; // abs part - which channel, sign - do/don't

    bool doFallback, singleShot;
    float loopbackVolume;

    Triggers trigNum;
    Trigger **trigger;

    plotCursor HCursors[2], VCursors[2];
    bool showHCursors, showVCursors;

    bool plotDragX(float* v, const char* label, ImDrawList* dl, ImVec4 rect, ImU32 col, float v_min=-1.f, float v_max=1.f);
    bool plotDragY(float* v, const char* label, ImDrawList* dl, ImVec4 rect, ImU32 col, float v_min=-1.f, float v_max=1.f);
    float mapToRange(ImVec2 src, ImVec2 dest, float v);

#ifdef TRIGGER_DEBUG
    nint triggerDebugBegin, triggerDebugEnd;
#endif
    char errorText[2048];

    void doFullscreen();

  public:
    void setupRenderer(USCRenderers r);
    void setupTrigger(Triggers t);

    void attachAudioInput(USCAudio* i);

    void setOscData(float** d);
  
    bool isRunning();
    int  init();
    void doFrame();
    void drawGUI();
  
    void drawMainScope();
    void drawXYScope();

#ifdef TRIGGER_DEBUG
    void drawTriggerDebug();
#endif

    void drawGlobalControls();
    void drawChanControls();
    void drawXYScopeControls();

    void drawAbout();
    void drawSettings();
    void drawCursors();
    void drawAudioConfig();

    void errorPopup(const char* errorTxt, ...);

    bool doRestartAudio();

    int  getAudioInputDevice();
    int  getAudioOutputDevice();
    void updateAudioDevices();
  
    USCGUI(unscopeParams *params, AudioConfig *aConf);
    ~USCGUI();
};

extern const unsigned char step_one;
extern const char *windowLayout;
extern const char *triggerNames[];
extern const int sampleRates[];
extern const int frameSizes[];

#endif
