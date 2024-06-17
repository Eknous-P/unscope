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

class GUI {
  private:
    struct scopeParams {
      int plotFlags;
      int scopeFlags;
      float trigger;
      int traceSize; float timebase;
      int traceOffset; float trigOffset;
      int trigHoldoff;
      float yScale;
      float color[3][4];
      bool triggerEdge;
    };

    struct windowConfig {
      unsigned short int w,h;
      const char *layout;
    };

    int err;

    SDL_WindowFlags window_flags;
    SDL_Window* window;
    SDL_GLContext gl_context;
    bool fullscreen;
    ImGuiIO io;
    ImGuiStyle style;
    ImVec4 clear_color;
    bool running, updateOsc, restartAudio, audioLoopback;
    unsigned char triggerChan;
    unsigned char channels;
    float **oscData, **oscAuxData, *oscAlign;
    unsigned long int oscDataSize, sampleRate;
    AudioInput *ai;
    AudioProcess *ap;

    std::vector<DeviceEntry> devs;
    int device, deviceNum;

    scopeParams sc;
    windowConfig winC;

    bool showTrigger;

  public:
    void attachAudioInput(AudioInput* i);
    void attachAudioProcess(AudioProcess* p);

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
  
    unsigned long int getTraceSize();
    float getTrigger();
  
    void audioSet();
    bool doRestartAudio();

    int getAudioDeviceSetting();
    void setAudioDeviceSetting(int d);
  
    GUI(unsigned long int sampleRateDef, unsigned long int dataSize, unsigned char chanCount, float timebaseDef, float yScaleDef, float triggerDef);
    ~GUI();
};

extern const char* numberStrs[16];

#endif