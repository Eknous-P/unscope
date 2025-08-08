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

#ifndef USC_RENDER_DIRECTX_H
#define USC_RENDER_DIRECTX_H

#include "../gui.h"
#include "imgui_impl_sdl2.h"
#include "imgui_impl_dx9.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <SDL_syswm.h>

class USCRenderDirectX9 : public USCRender {
  SDL_Window* win;
  int winFlags;
  SDL_Event event;
  
  IDirect3DDevice9* dev;

  float clearColor[4];

  public:
    int initRender();
    int setupRender(int _winFlags, const char* winName, int winX, int winY, int winW, int winH);
    int renderPreLoop();
    int renderPostLoop();
    void destroyRender();
};

#endif
