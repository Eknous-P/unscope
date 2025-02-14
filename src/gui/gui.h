#ifndef USC_GUI_H
#define USC_GUI_H

#include <imgui.h>
#include <implot.h>
#include "imgui_stdlib.h"

#include "shared.h"
#include "audio.h"
#include "config.h"

#define INIFILE "unscope.ini"

enum TriggerModes : unsigned char {
  TRIGGER_NONE, // fixed sweep
  TRIGGER_AUTO, // normal, fixed if not triggering
  TRIGGER_NORMAL,
  TRIGGER_SINGLE
};

class USCRender {
  public:
    virtual int  setup();
    virtual int  init();
    virtual bool beginFrame();
    virtual void endFrame(ImGuiIO io, ImVec4 col);
    virtual void deinit();
    virtual void doFullscreen(bool f);
};

// renderers
#ifdef USE_OPENGL
#include "render/render_opengl_sdl.h"
#endif
#ifdef USE_DIRECTX
#include "render/render_directx11_sdl.h"
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
      int    traceSize;
      float  timebase;
    };

    struct xyParams {
      float xScale,  yScale;
      float xOffset, yOffset;
      int   sampleLen;
      float persistence;
      unsigned char axisChan[2];
    };

    int err;

    ImGuiIO io;
    ImGuiStyle style;
    ImVec4 bgColor;
    bool isGood, running, updateAudio, restartAudio, audioLoopback, triggerSet;
    unsigned char channels;
    USCRenderers renderer;
  
    float **oscData, **oscAlign;
    unsigned long int oscDataSize, sampleRate;
    float bufferTime;
    USCRender *rd;
    USCAudioInput *ai;
    USCConfig *cf;

    std::vector<DeviceEntry> devs;
    char** devs_c; // for settings
    int device, deviceNum;

    unscopeParams *up;
    scopeParams    sc;
    traceParams   *tc;
    xyParams       xyp;

    bool showTrigger, shareParams;
    signed char shareTrigger; // abs part - which channel, sign - do/don't

    bool doFallback, singleShot;

    Triggers trigNum;
    Trigger **trigger, **fallbackTrigger;

  public:
    void setupRenderer(USCRenderers r);
    void setupTrigger(Triggers t);

    void attachAudioInput(USCAudioInput* i);
    void attachConfig(USCConfig* c);

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

    void deinint();
  
    USCGUI(unscopeParams *params);
    ~USCGUI();
};

extern ImVec2 operator+(ImVec2 lhs, ImVec2 rhs);
extern ImVec2 operator+(ImVec2 lhs, float rhs);

extern const char *windowLayout;
extern const char *triggerNames[];

#endif