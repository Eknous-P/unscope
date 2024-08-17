#ifndef USC_GUI_H
#define USC_GUI_H

#include <imgui.h>
#include <implot.h>
#include "imgui_stdlib.h"

#include "../shared.h"
#include "../audio/audio.h"

#define INIFILE "unscope.ini"

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
      float color[4];
      float trigger;
      int   traceSize;   float timebase;
      int   traceOffset; float trigOffset;
      int   trigHoldoff;
      bool  triggerEdge;
    };

    struct xyParams {
      float xScale,  yScale;
      float xOffset, yOffset;
      float color[4];
      int   sampleLen;
      float persistence;
      int   xChan,   yChan;
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

    bool fullscreen;
    ImGuiIO io;
    ImGuiStyle style;
    ImVec4 bgColor, trigColor;
    bool isGood, running, updateOsc, restartAudio, audioLoopback;
    unsigned char channels;
    USCRenderers renderer;
  
    float **oscData, **oscAuxData, **oscAlign;
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
    int shareTrigger; // abs part - which channel, sign - do/don't

  public:
    void setupRenderer(USCRenderers r);

    void attachAudioInput(USCAudioInput* i);

    void writeOscData(unsigned char chan, float* datax, float* datay);
    void writeOscAuxData(unsigned char chan, float* data);
  
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

    void drawSettings();
  
    void audioSet();
    bool doRestartAudio();

    int  getAudioDeviceSetting();
    void setAudioDeviceSetting(int d);
  
    USCGUI(unsigned long int sampleRateDef, unsigned long int dataSize, unsigned char chanCount, float timebaseDef, float yScaleDef, float triggerDef, int rendererDef);
    ~USCGUI();
};

extern const char *windowLayout;

#endif