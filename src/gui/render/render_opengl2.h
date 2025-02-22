#ifndef USC_RENDER_OPENGL2_H
#define USC_RENDER_OPENGL2_H

#include "../gui.h"
#include <SDL_opengl.h>
#include "imgui_impl_sdl2.h"
#include "imgui_impl_opengl2.h"

class USCRenderOpenGL2: public USCRender {
  SDL_Window* win;
  int winFlags;
  SDL_Event event;

  SDL_GLContext glContext;
  public:
    int initRender();
    int setupRender(int _winFlags, const char* winName, int winX, int winY, int winW, int winH);
    int renderPreLoop();
    int renderPostLoop();
    void destroyRender();

    SDL_Window* getWindow();
    ~USCRenderOpenGL2();
};

#endif