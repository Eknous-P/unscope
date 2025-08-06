/*
unscope - an audio oscilloscope
Copyright (C) 2025 Eknous

unscope is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 2 of the License, or (at your option) any later
version.

unscope is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License along with
unscope. If not, see <https://www.gnu.org/licenses/>. 
*/

#ifndef USC_RENDER_SDLRENDERER2_H
#define USC_RENDER_SDLRENDERER2_H

#include "../gui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_sdlrenderer2.h"

class USCRenderSDLRenderer: public USCRender {
  SDL_Window* win;
  int winFlags;
  SDL_Event event;

  SDL_Renderer* renderer;
  public:
    int initRender();
    int setupRender(int _winFlags, const char* winName, int winX, int winY, int winW, int winH);
    int renderPreLoop();
    int renderPostLoop();
    void destroyRender();

    SDL_Window* getWindow();
    ~USCRenderSDLRenderer();
};

#endif
