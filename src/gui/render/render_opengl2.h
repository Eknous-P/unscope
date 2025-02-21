#ifndef USCRENDERER_OGL_SDL
#define USCRENDERER_OGL_SDL

#include "../gui.h"
#include <SDL_opengl.h>
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl2.h"

class USCRenderOpenGL2: public USCRender {
  SDL_Window* win;
  SDL_WindowFlags winFlags;
  SDL_Event event;

  SDL_GLContext glContext;
  public:
    int initRender();
    int setupRender(SDL_WindowFlags _winFlags, const char* winName, ImVec2 winPos, ImVec2 winSize);
    int renderPreLoop();
    int renderPostLoop();
    void destroyRender();

    SDL_Window* getWindow();
    ~USCRenderOpenGL2();
};

#endif