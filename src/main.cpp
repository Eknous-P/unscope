#include "audio/audio.h"
#include "gui/gui.h"
#include "../shared.h"

float clamp(float a) {
  if (a > 1.0f) return 1.0f;
  if (a < -1.0f) return -1.0f;
  return a;
}

const char* getErrorMsg(int e) {
  if (e>=0) {
    return errMsgs[e];
  } else {
    return Pa_GetErrorText(e);
  }
}

int main(int argc, char* argv[]) {
  int e;
  unscopeParams params;

  params.audioBufferSize = 65536; // N samples
  params.audioFrameSize  = 512;   // N samples
  params.channels        = 2;     // N (1-3)
  params.sampleRate      = 48000; // Hz (only common values?)
  params.audioDevice     = 0;     // internal ID, gets overwritten anyway
  params.timebase        = 60;    // ms
  params.xyPersist       = 40;    // ms
  params.scale           = 1.0f;  // (no unit)
  params.trigger         = 0.0f;  // (no unit)
#ifdef _WIN32
  params.renderer = USC_RENDER_DIRECTX11;
#else
  params.renderer = USC_RENDER_OPENGL2;
#endif

  // parse arguments
  if (argc > 1) {
    unsigned char flagStartIndex = 1;
    int value = 0;
    for (int i = 1; i < argc; i += 2) {
      if (argv[i][0] == '-') {
        flagStartIndex = 1;
        if (argv[i][1] == '-') {
          flagStartIndex = 2;
        }
        if (argv[i][flagStartIndex + 1] == 0) {
          switch (argv[i][flagStartIndex]) {
            case UPARAM_ABOUT:
              printf("%s%s", verMsg, aboutMsg);
              return 0;
            case UPARAM_HELP:
              printf("%s%s", verMsg, helpMsg);
              return 0;
            case UPARAM_VERSION:
              printf("%s", verMsg);
              return 0;
            default: break;
          }
          if (i + 1 == argc) {
            printf(ERROR_MSG "no value for argument %s given" MSG_END, argv[i]);
            continue;
          }
          try {
            value = std ::stoi(argv[i + 1]);
          } catch (...) {
            printf(ERROR_MSG "invalid argument for %s given: %s" MSG_END, argv[i], argv[i + 1]);
            continue;
          }
          switch (argv[i][flagStartIndex]) {
            case UPARAM_BUFFERSIZE:
              params.audioBufferSize = value;
              break;
            case UPARAM_FRAMESIZE:
              params.audioFrameSize = value;
              break;
            case UPARAM_CHANNELCOUNT: {
              if (value > 3) {
                printf(ERROR_MSG "invalid value for %s given: %d" MSG_END, argv[i], value);
                continue;
              }
              params.channels = value;
              break;
            }
            case UPARAM_SAMPLERATE: {
              if ((value%8000)!=0 && (value%11025)!=0) {
                printf(ERROR_MSG "invalid value for %s given: %d" MSG_END, argv[i], value);
                continue;
              }
              params.sampleRate = value;
              break;
            }
            default: break;
          }
        } else {
          if (strcmp(argv[i] + flagStartIndex, "help") == 0) {
            printf("%s%s", verMsg, helpMsg);
            return 0;
          } else if (strcmp(argv[i] + flagStartIndex, "about") == 0) {
            printf("%s%s", verMsg, aboutMsg);
            return 0;
          } else if (strcmp(argv[i] + flagStartIndex, "version") == 0) {
            printf("%s", verMsg);
            return 0;
          }
          if (i + 1 == argc) {
            printf(ERROR_MSG "no value for argument %s given" MSG_END, argv[i]);
            continue;
          }
          try {
            value = std::stoi(argv[i + 1]);
          } catch (...) {
            printf(ERROR_MSG "invalid argument for %s given: %s" MSG_END, argv[i], argv[i + 1]);
            continue;
          }
          if (value < 0) {
            printf(ERROR_MSG "invalid value for %s given: %d" MSG_END, argv[i], value);
            continue;
          }
          if (strcmp(argv[i] + flagStartIndex, "bufsize") == 0) {
            params.audioBufferSize = value;
            continue;
          } else if (strcmp(argv[i] + flagStartIndex, "framesize") == 0) {
            params.audioFrameSize = value;
            continue;
          } else if (strcmp(argv[i] + flagStartIndex, "channels") == 0) {
            if (value > 3) {
              printf(ERROR_MSG "invalid value for %s given: %d" MSG_END, argv[i], value);
              continue;
            }
            params.channels = value;
            continue;
          } else if (strcmp(argv[i] + flagStartIndex, "samplerate") == 0) {
            if ((value%8000)!=0 && (value%11025)!=0) {
              printf(ERROR_MSG "invalid value for %s given: %d" MSG_END, argv[i], value);
              continue;
            }
            params.sampleRate = value;
            continue;
          }
        }
      } else {
        printf(ERROR_MSG "cannot parse argument %s" MSG_END, argv[i]);
      }
    }
  };

  USCGUI g(&params);

  e = g.init();
  if (e != 0) {
    printf(ERROR_MSG "error in initializing GUI! %s" MSG_END, getErrorMsg(e));
    return -1;
  }

  USCAudioInput i(&params);

  g.attachAudioInput(&i);

  params.audioDevice = Pa_GetDefaultInputDevice();

  g.getDevices(i.enumerateDevs());
  g.setAudioDeviceSetting(params.audioDevice);

  printf(INFO_MSG "opening device %d: %s..." MSG_END,params.audioDevice,Pa_GetDeviceInfo(params.audioDevice)->name);
  e = i.init(params.audioDevice,0);
  if (e != paNoError) {
    printf(ERROR_MSG "%d: cant init audio! %s" MSG_END, e, getErrorMsg(e));
    // try again
    printf(INFO_MSG "trying again..." MSG_END);
    if (e != paInvalidDevice) {
      printf(ERROR_MSG "%d: cant init audio! %s" MSG_END, e, getErrorMsg(e));
      return e;
    }
    printf(INFO_MSG "trying default device..." MSG_END);
    params.audioDevice = Pa_GetDefaultInputDevice();
    e = i.init(params.audioDevice,0);
    if (e != paNoError) {
      printf(ERROR_MSG "%d: cant init audio! %s" MSG_END, e, getErrorMsg(e));
      return e;
    }
    g.setAudioDeviceSetting(params.audioDevice);
  }

  while (g.isRunning()) {
    g.doFrame();
  }

  e = i.stop();
  printf( "%s%s" MSG_END, (e==paNoError)?SUCCESS_MSG:ERROR_MSG, Pa_GetErrorText(e));
  return e!=paNoError;
}

#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow) {
  return main(__argc, __argv);
}
#endif

const char* verMsg = MISC_MSG PROGRAM_NAME " (version " PROGRAM_VER ")" MSG_END;

const char* helpMsg =
"Program arguments\n"
"  -b, -bufsize:    set the audio buffer size (default: 65536)\n"
"  -f, -framesize:  set the audio frame size  (default: 512)\n"
"  -c, -channels:   set the channel amount    (default: 2)\n"
"  -s, -samplerate: set the sample rate       (default: 48000)\n"
"  -a, -about:      print about message\n"
"  -h, -help:       print this message\n"
"  -v, -version:    print the program version\n";

const char* aboutMsg = 
"Audio oscilloscope for Linux"
#ifdef _WIN32
" and Windows"
#endif
"\n\n"
PROGRAM_NAME " is made using these libraries:\n\n"
"PortAudio (https://github.com/PortAudio/portaudio)\n"
#ifdef NON_SYS_SDL
"SDL (https://github.com/libsdl-org/SDL)\n"
#endif
"Dear ImGui (https://github.com/ocornut/imgui)\n"
"ImPlot (https://github.com/epezent/implot)\n"
"ImGui Knobs (https://github.com/altschuler/imgui-knobs)\n"
"imgui_toggle (https://github.com/cmdwtf/imgui_toggle)\n\n"
PROGRAM_NAME " is licensed under the MIT licence.\n"
"see LICENSE for details.\n"
"Copyright © 2024-2025 Eknous\n"
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
#ifdef USE_OPENGL
  "OpenGL",
#endif
#ifdef USE_DIRECTX
  "DirectX 11",
#endif
};