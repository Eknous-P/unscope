#include <imgui.h>
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"
#include <implot.h>
#include <SDL.h>
#include <SDL_opengl.h>

#define PROGRAM_NAME "unscope"
#define PROGRAM_VER 0

class GUI {
  private:
    struct scopeParams {
      int traceSize;
      int traceOffset;
      float yScale;
      float trigger;
      float color[4];
    };
    struct windowConfig {
      unsigned short int w,h;
      const char *layout;
    };
    SDL_WindowFlags window_flags;
    SDL_Window* window;
    SDL_GLContext gl_context;
    ImGuiIO io;
    ImGuiStyle style;
    ImVec4 clear_color;
    bool running, update;
    unsigned char channels;
    float *oscData, *oscAuxData, *oscAlign;
    unsigned int oscDataSize;

    scopeParams sc;
    windowConfig winC;

  public:
    void writeOscData(float* datax, float* datay);
    void writeOscAuxData(float* data);
    bool isRunning();
    int init();
    void doFrame();
    void drawGUI();
    void drawMainScope();
    void drawAuxScope();
    unsigned long int getTraceSize();
    float getTrigger();
    GUI(unsigned long int dataSize, unsigned char chanCount, int traceSizeDef, float yScaleDef, float triggerDef);
    ~GUI();
};