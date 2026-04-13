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

#include "data.h"
#include "gui.h"
#include "shared.h"
#include "config.h"

int main(int argc, char** argv) {
  int e;
  char* prefPath = SDL_GetPrefPath(NULL, "unscope");
#ifdef _WIN32
#warning "SDL_GetPrefPath: set org!"
#endif
  string prefPathS;
  prefPathS+=prefPath;
  SDL_free(prefPath);

  printf(INFO_MSG "loading config..." MSG_END);
  string confPath=prefPathS+"config.yaml";
  USCConfig config(confPath.c_str());
  config.loadConfig();

  printf(INFO_MSG "initializing data..." MSG_END);
  USCData data(&config);
  data.loadFromConfig();

  printf(INFO_MSG "initializing GUI..." MSG_END);
  string layoutPath=prefPathS+"layout.ini";
  USCGUI gui(&data, &config, layoutPath);

  gui.readConfig();
#ifdef _WIN32
  e = gui.init(USC_RENDER_DIRECTX11);
#else
  e = gui.init(USC_RENDER_OPENGL2);
#endif
  if (e) {
    printf(ERROR_MSG "failed to init GUI! exiting..." MSG_END);
    return 1;
  }


  printf(INFO_MSG "starting GUI loop" MSG_END);
  while (gui.isRunning()) {
    gui.doFrame();
  }

  printf(INFO_MSG "GUI loop over" MSG_END);
  gui.writeConfig();

  data.saveToConfig();
  for (int i=0; i<data.getDriverCount(); i++) {
    data.dispatchDriverCommand(i, DRIVER_DESTROY);
    data.removeDriver(i);
  }

  config.saveConfig();

  printf(SUCCESS_MSG "exiting successfully." MSG_END);
  return 0;
}

#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow) {
  return main(__argc, __argv);
}
#endif

const char* verMsg = MISC_MSG PROGRAM_NAME_AND_VER MSG_END;

const char* helpMsg =
"Program arguments\n"
"  -b, -bufsize:        set the audio buffer size    (default: 65536)\n"
"  -f, -framesize:      set the audio frame size     (default: 512)\n"
"  -s, -samplerate:     set the sample rate          (default: 48000)\n"
"  -c: -channels        set the i/o channel count    (default: 2)\n"
"      -inputChannels:  set the input channel count  (default: 2)\n"
"      -outputChannels: set the output channel count (default: 2)\n"
"  -a, -about:      print about message\n"
"  -h, -help:       print this message\n"
"  -l  -license     print the licensing infomation\n"
"  -v, -version:    print the program version\n";

const char* aboutMsg = 
"An audio oscilloscope\n";

const char* licenseMsg =
"Copyright (C) 2025-2026 Eknous\n"
"\n"
"unscope is free software: you can redistribute it and/or modify it under the\n"
"terms of the GNU General Public License as published by the Free Software\n"
"Foundation, either version 2 of the License, or (at your option) any later\n"
"version.\n"
"\n"
"unscope is distributed in the hope that it will be useful, but WITHOUT ANY\n"
"WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A\n"
"PARTICULAR PURPOSE. See the GNU General Public License for more details.\n"
"\n"
"You should have received a copy of the GNU General Public License along with\n"
"unscope. If not, see <https://www.gnu.org/licenses/>. \n"
;

const char* renderers[] = {
  "SDL2 Renderer",
#ifdef USE_OPENGL
  "OpenGL",
#endif
#ifdef USE_DIRECTX19
  "DirectX 9",
#endif
#ifdef USE_DIRECTX11
  "DirectX 11",
#endif
};
