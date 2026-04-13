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

#include "consts.h"

#include <SDL.h>
#include "imgui.h"
#include "imgui_stdlib.h"
#include "imgui_internal.h"
#include "imgui_stdlib.h"

#include "shared.h"
#include "data.h"
#include "config.h"
#include "scope.h"

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
template <typename T> T invLerp(T a, T b, float x) {return (x-a)/(b-a);}

class USCGUI {
  private:
    USCConfig* config;

    struct settingsData {
      bool dummy;
      // gui-related stuff
      bool enableMultiViewports;
      bool scopeContainChannelControls;
      bool showDisabledBuffers;
      //
      settingsData():
        dummy(false),
        enableMultiViewports(false),
        scopeContainChannelControls(false),
        showDisabledBuffers(false)
        {}
    } settings;

    struct colorsData {
      ImU32 widgetActiveColor;
      ImU32 defaultChannelColor;
      ImU32 pianoTopColor, pianoBottomColor;
    } colors;

    struct SettingsCategory {
      const char* name;
      vector<Parameter> settings;
      vector<SettingsCategory> children;

      void drawParams();
      void drawTree();

      void readConfig(USCConfig* conf);
      void writeConfig(USCConfig* conf);

      SettingsCategory();
      SettingsCategory(const char* n, std::initializer_list<Parameter> p, std::initializer_list<SettingsCategory> c);
    };

    SettingsCategory settingsParams;

    string layoutFile;

    vector<ScopeWindow> scopeWindows;

    struct windowsOpen {
      bool aboutOpen;
      bool cursorsOpen;
      bool dataIOConfigOpen;
      bool settingsOpen;
      bool spectrumOpen;
      bool spectrumControlsOpen;
#ifdef PROGRAM_DEBUG
      bool metricsOpen;
      bool paramDebugOpen;
      bool triggerDebugOpen;
      bool fftDebugOpen;
#endif
    } wo;

    int windowWidth, windowHeight;
    bool fullscreen, running;
    ImVec4 bgColor;
    USCRenderers renderer;
  
    USCRender* rd;
    USCData* data;

    bool doFallback, singleShot;

    ImU32* pianoColors[12];

    DataBuffer* newScopeBuffer;
    DataDrivers newDriver;
    int newScopeWin;

#ifdef PROGRAM_DEBUG
    int debuggedWindow, debuggedChannel;
    double fftPeak;
#endif
    string errorText;

    void doFullscreen();

    void setupRenderer(USCRenderers r);

  // windows:
    void drawChanControls();

    void drawAbout(bool* open);
    void drawSettings(bool* open);
    void drawDataConfig(bool* open);

    void errorPopup(const char* errorTxt, ...);

    // void drawScopeDebugImFrustrated();

#ifdef PROGRAM_DEBUG
    void drawTriggerDebug(bool* open);
    void drawParamDebug(bool* open);
    // void drawFFTDebug(bool* open);
#endif

  public:
    int  init(USCRenderers rend);
    bool isRunning();
    void doFrame();
    void drawGUI();

    void readConfig();
    void writeConfig();
  
    USCGUI(USCData* _data, USCConfig* conf, string layoutFilePath);
    ~USCGUI();
};

#endif