#include "../gui.h"
#include <SDL.h>
#include <SDL_opengl.h>
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl3.h"

#ifndef USCRENDERER_OGL_SDL
#define USCRENDERER_OGL_SDL

class USCRendOGL_SDL: public USCRender {
  private:
    SDL_WindowFlags window_flags;
    SDL_Window* window;
    SDL_GLContext gl_context;
    SDL_Event event;
  public:
    int setup();
    int init();
    bool beginFrame();
    void endFrame(ImGuiIO io, ImVec4 col);
    void deinit();
    void doFullscreen(bool f);
};

#endif