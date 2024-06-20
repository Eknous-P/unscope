#include <imgui.h>
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <implot.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui_stdlib.h"

#include "../shared.h"
#include "../audio/audio.h"

#ifndef USC_GUI_H
#define USC_GUI_H

#define INIFILE "unscope.ini"

class USCGUI {
  private:
    struct scopeParams {
      int plotFlags;
      int scopeFlags;
    };

    struct traceParams {
      bool enable;
      float yOffset;
      float yScale;
      float color[4];
      float trigger;
      int traceSize; float timebase;
      int traceOffset; float trigOffset;
      int trigHoldoff;
      bool triggerEdge;
    };

    struct xyParams {
      float xScale, yScale;
      float xOffset, yOffset;
      float color[4];
      int sampleLen;
      float persistence;
    };

    struct windowsOpen {
      bool mainScopeOpen;
      bool auxScopeOpen;
      bool chanControlsOpen[3];
      bool xyScopeOpen;
      bool xyScopeControlsOpen;
      bool globalControlsOpen;
    };

    struct settings {
      bool doSetting;
    };

    int err;

    SDL_WindowFlags window_flags;
    SDL_Window* window;
    SDL_GLContext gl_context;
    bool fullscreen;
    ImGuiIO io;
    ImGuiStyle style;
    ImVec4 clear_color, trigColor;
    bool isGood, running, updateOsc, restartAudio, audioLoopback;
    unsigned char channels;
    float **oscData, **oscAuxData, **oscAlign;
    unsigned long int oscDataSize, sampleRate;
    USCAudioInput *ai;
    USCAudioProcess *ap;

    std::vector<DeviceEntry> devs;
    int device, deviceNum;

    scopeParams sc;
    traceParams *tc;
    xyParams xyp;
    windowsOpen wo;
    settings st;

    bool showTrigger;

  public:
    void attachAudioInput(USCAudioInput* i);
    void attachAudioProcess(USCAudioProcess* p);

    void writeOscData(unsigned char chan, float* datax, float* datay);
    void writeOscAuxData(unsigned char chan, float* data);
  
    bool isRunning();
    int init();
    void getDevices(std::vector<DeviceEntry> d);
    void doFrame();
    void drawGUI();
  
    void drawMainScope();
    void drawAuxScope();
    void drawXYScope();

    void drawGlobalControls();
    void drawChanControls();
    void drawXYScopeControls();

    void drawSettings();
  
    void audioSet();
    bool doRestartAudio();

    int getAudioDeviceSetting();
    void setAudioDeviceSetting(int d);
  
    USCGUI(unsigned long int sampleRateDef, unsigned long int dataSize, unsigned char chanCount, float timebaseDef, float yScaleDef, float triggerDef);
    ~USCGUI();
};

extern const char *windowLayout;

#endif