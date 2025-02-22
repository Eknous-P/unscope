#ifndef USC_GUI_H
#define USC_GUI_H

#include <SDL.h>
#include <imgui.h>
#include <implot.h>
#include "imgui_stdlib.h"

#include "../shared.h"
#include "../audio/audio.h"

#define INIFILE "unscope.ini"

enum TriggerModes : unsigned char {
  TRIGGER_NONE, // fixed sweep
  TRIGGER_AUTO, // normal, fixed if not triggering
  TRIGGER_NORMAL,
  TRIGGER_SINGLE
};

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


class USCGUI {
  private:
    struct scopeParams {
      int plotFlags;
      int scopeFlags;
    };

    struct traceParams {
      bool   enable;
      float  yOffset;
      float  yScale;
      ImVec4 color;
      int    traceSize;
      float  timebase;
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
    };

    struct settings {
      bool doSetting;
    };

    int err;

    bool fullscreen;
    ImGuiIO io;
    ImGuiStyle style;
    ImVec4 bgColor;
    bool isGood, running, updateAudio, restartAudio, audioLoopback, triggerSet;
    unsigned char channels;
    USCRenderers renderer;
  
    float **oscData, **oscAlign;
    unsigned long int oscDataSize, sampleRate;
    USCRender *rd;
    USCAudioInput *ai;

    std::vector<DeviceEntry> devs;
    int device, deviceNum;

    unscopeParams *up;
    scopeParams    sc;
    traceParams   *tc;
    xyParams       xyp;
    windowsOpen    wo;
    settings       st;

    bool showTrigger, shareParams;
    signed char shareTrigger; // abs part - which channel, sign - do/don't

    bool doFallback, singleShot;

    Triggers trigNum;
    Trigger **trigger, **fallbackTrigger;

  public:
    void setupRenderer(USCRenderers r);
    void setupTrigger(Triggers t);

    void attachAudioInput(USCAudioInput* i);

    void setOscData(float** d);
  
    bool isRunning();
    int  init();
    void getDevices(std::vector<DeviceEntry> d);
    void doFrame();
    void drawGUI();
  
    void drawMainScope();
    void drawXYScope();

    void drawAlignDebug();

    void drawGlobalControls();
    void drawChanControls();
    void drawXYScopeControls();

    void drawAbout();
    void drawSettings();

    bool doRestartAudio();

    int  getAudioDeviceSetting();
    void setAudioDeviceSetting(int d);
  
    USCGUI(unscopeParams *params);
    ~USCGUI();
};

extern const unsigned char step_one;
extern const char *windowLayout;
extern const char *triggerNames[];

#endif