#ifndef USC_GUI_H
#define USC_GUI_H

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


class USCGUI {
  private:
    struct scopeParams {
      int plotFlags;
      int scopeFlags;
    };

    struct traceParams {
      bool  enable;
      float yOffset;
      float yScale;
      ImVec4 color;
      float trigger;
      int   traceSize;   float timebase;
      int   traceOffset; float trigOffset;
      int   trigHoldoff;
      bool  triggerEdge;
    };

    struct xyParams {
      float xScale,  yScale;
      float xOffset, yOffset;
      ImVec4 color;
      int   sampleLen;
      float persistence;
      int   xChan,   yChan;
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
    ImVec4 bgColor, trigColor;
    bool isGood, running, updateAudio, restartAudio, audioLoopback;
    unsigned char channels;
    USCRenderers renderer;
  
    float **oscData, **oscAlign;
    unsigned long int oscDataSize, sampleRate;
    USCRender *rd;
    USCAudioInput *ai;

    std::vector<DeviceEntry> devs;
    int device, deviceNum;

    scopeParams sc;
    traceParams *tc;
    xyParams    xyp;
    windowsOpen wo;
    settings    st;

    bool showTrigger, shareParams;
    signed char shareTrigger; // abs part - which channel, sign - do/don't
    TriggerModes triggerMode;

  public:
    void setupRenderer(USCRenderers r);

    void attachAudioInput(USCAudioInput* i);

    void writeOscData(unsigned char chan, float* datax, float* datay);
  
    bool isRunning();
    int  init();
    void getDevices(std::vector<DeviceEntry> d);
    void doFrame();
    void drawGUI();
  
    void drawMainScope();
    void drawXYScope();

    void drawGlobalControls();
    void drawChanControls();
    void drawXYScopeControls();

    void drawAbout();
    void drawSettings();

    void drawTriggerLamp(unsigned char chan);

    bool doRestartAudio();

    int  getAudioDeviceSetting();
    void setAudioDeviceSetting(int d);
  
    USCGUI(unscopeParams params);
    ~USCGUI();
};

extern const char *windowLayout;
extern const char *triggerModeNames[4];

#endif