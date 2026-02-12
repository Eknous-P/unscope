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

float clamp(float a) {
  if (a > 1.0f) return 1.0f;
  if (a < -1.0f) return -1.0f;
  return a;
}

const char* getErrorMsg(int e) {
  return errMsgs[e];
}

int main(int argc, char** argv) {
  int e;

  // parse arguments
  // if (argc > 1) {
  //   unsigned char flagStartIndex = 1;
  //   int value = 0;
  //   for (int i = 1; i < argc; i += 2) {
  //     if (argv[i][0] == '-') {
  //       flagStartIndex = 1;
  //       if (argv[i][1] == '-') {
  //         flagStartIndex = 2;
  //       }
  //       if (argv[i][flagStartIndex + 1] == 0) {
  //         switch (argv[i][flagStartIndex]) {
  //           case UPARAM_ABOUT:
  //             printf("%s%s", verMsg, aboutMsg);
  //             return 0;
  //           case UPARAM_HELP:
  //             printf("%s%s", verMsg, helpMsg);
  //             return 0;
  //           case UPARAM_LICENSE:
  //             printf("%s%s", verMsg, licenseMsg);
  //             return 0;
  //           case UPARAM_VERSION:
  //             printf("%s", verMsg);
  //             return 0;
  //           default: break;
  //         }
  //         if (i + 1 == argc) {
  //           printf(ERROR_MSG "no value for argument %s given" MSG_END, argv[i]);
  //           continue;
  //         }
  //         try {
  //           value = std::stoi(argv[i + 1]);
  //         } catch (...) {
  //           printf(ERROR_MSG "invalid argument for %s given: %s" MSG_END, argv[i], argv[i + 1]);
  //           continue;
  //         }
  //         switch (argv[i][flagStartIndex]) {
  //           case UPARAM_BUFFERSIZE:
  //             params.audioBufferSize = value;
  //             break;
  //           case UPARAM_FRAMESIZE:
  //             audConf.frameSize = value;
  //             break;
  //           case UPARAM_CHANNELCOUNT: {
  //             if (value > 3) {
  //               printf(ERROR_MSG "invalid value for %s given: %d" MSG_END, argv[i], value);
  //               continue;
  //             }
  //             audConf.inputChannels  = value;
  //             audConf.outputChannels = value;
  //             break;
  //           }
  //           case UPARAM_SAMPLERATE: {
  //             if ((value%8000)!=0 && (value%11025)!=0) {
  //               printf(ERROR_MSG "invalid value for %s given: %d" MSG_END, argv[i], value);
  //               continue;
  //             }
  //             audConf.sampleRate = value;
  //             break;
  //           }
  //           default: break;
  //         }
  //       } else {
  //         if (strcmp(&argv[i][flagStartIndex], "help") == 0) {
  //           printf("%s%s", verMsg, helpMsg);
  //           return 0;
  //         } else if (strcmp(&argv[i][flagStartIndex], "about") == 0) {
  //           printf("%s%s", verMsg, aboutMsg);
  //           return 0;
  //         } else if (strcmp(&argv[i][flagStartIndex], "license") == 0) {
  //           printf("%s%s", verMsg, licenseMsg);
  //           return 0;
  //         } else if (strcmp(&argv[i][flagStartIndex], "version") == 0) {
  //           printf("%s", verMsg);
  //           return 0;
  //         }
  //         if (i + 1 == argc) {
  //           printf(ERROR_MSG "no value for argument %s given" MSG_END, argv[i]);
  //           continue;
  //         }
  //         try {
  //           value = std::stoi(argv[i + 1]);
  //         } catch (...) {
  //           printf(ERROR_MSG "invalid argument for %s given: %s" MSG_END, argv[i], argv[i + 1]);
  //           continue;
  //         }
  //         if (value < 0) {
  //           printf(ERROR_MSG "invalid value for %s given: %d" MSG_END, argv[i], value);
  //           continue;
  //         }
  //         if (strcmp(argv[i] + flagStartIndex, "bufsize") == 0) {
  //           params.audioBufferSize = value;
  //           continue;
  //         } else if (strcmp(argv[i] + flagStartIndex, "framesize") == 0) {
  //           audConf.frameSize = value;
  //           continue;
  //         } else if (strcmp(argv[i] + flagStartIndex, "channels") == 0) {
  //           if (value > 3) {
  //             printf(ERROR_MSG "invalid value for %s given: %d" MSG_END, argv[i], value);
  //             continue;
  //           }
  //           audConf.inputChannels = value;
  //           audConf.outputChannels = value;
  //           continue;
  //         } else if (strcmp(argv[i] + flagStartIndex, "inputChannels") == 0) {
  //           if (value > 3) {
  //             printf(ERROR_MSG "invalid value for %s given: %d" MSG_END, argv[i], value);
  //             continue;
  //           }
  //           audConf.inputChannels = value;
  //           continue;
  //         } else if (strcmp(argv[i] + flagStartIndex, "outputChannels") == 0) {
  //           if (value > 3) {
  //             printf(ERROR_MSG "invalid value for %s given: %d" MSG_END, argv[i], value);
  //             continue;
  //           }
  //           audConf.outputChannels = value;
  //           continue;
  //         } else if (strcmp(argv[i] + flagStartIndex, "samplerate") == 0) {
  //           if ((value%8000)!=0 && (value%11025)!=0) { // i bet some odd sample rate will not be caught by this
  //             printf(ERROR_MSG "invalid value for %s given: %d" MSG_END, argv[i], value);
  //             continue;
  //           }
  //           audConf.sampleRate = value;
  //           continue;
  //         }
  //       }
  //     } else {
  //       printf(ERROR_MSG "cannot parse argument %s" MSG_END, argv[i]);
  //     }
  //   }
  // };

  USCGUI gui;

  e = gui.init(USC_RENDER_OPENGL2);
  if (e) {
    printf(ERROR_MSG "error in initializing GUI! %d:%s" MSG_END, e, getErrorMsg(e));
    return 1;
  }

  USCData data;

  e = data.addDriver(DATA_PORTAUDIO);
  if (e) {
    printf(ERROR_MSG "oh no!! %d" MSG_END, e);
    return 1;
  }

  gui.attachData(&data);

  while (gui.isRunning()) {
    gui.doFrame();
  }

  for (int i=0; i<data.getDriverCount(); i++) {
    data.dispatchDriverCommand(i, DRIVER_STOP_CALLBACK);
    data.removeDriver(i);
  }


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

const char* thirdPartyMsg = // TODO: change this to be able to use TextLinkOpenURL
"unscope is made using these libraries:\n"
"\n"
"PortAudio (https://github.com/PortAudio/portaudio)\n"
#ifdef NON_SYS_SDL
"SDL (https://github.com/libsdl-org/SDL)\n"
#endif
"Dear ImGui (https://github.com/ocornut/imgui)\n"
"ImGui Knobs (https://github.com/altschuler/imgui-knobs)\n"
"imgui_toggle (https://github.com/cmdwtf/imgui_toggle)\n"
"PFFFT (https://github.com/marton78/pffft)\n"
"ShowCQT (modified) (https://github.com/mfcc64/showcqt-js)\n";

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

const char* errMsgs[] = {
  "",
  "init fail",
  "no devices found\n",
  "cant open device\n",
  "cant start device\n",
  "cant init audioinput\n",
  "cant setup gui renderer\n",
  "cant init gui renderer\n"
};

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
