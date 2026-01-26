/*
unscope - an audio oscilloscope
Copyright (C) 2025-2026 Eknous

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

#include "../gui.h"

int USCRender::initRender() {
  return 0;
}

int USCRender::setupRender(int _winFlags, const char* winName, int winX, int winY, int winW, int winH) {
  win=NULL;
  winFlags=_winFlags;
  (void)winName;
  (void)winX;
  (void)winY;
  (void)winW;
  (void)winH;
  return 0;
}

int USCRender::renderPreLoop() {
  return 0;
}

int USCRender::renderPostLoop() {
  return 0;
}

void USCRender::destroyRender() {
}

SDL_Window* USCRender::getWindow() {
  return win;
}

USCRender::~USCRender() {
}
