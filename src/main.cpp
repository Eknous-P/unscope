#include "shared.h"
#include "audio.h"
#include "gui.h"
#include "config.h"

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

  params.mainScopeOpen       = true;
  params.chanControlsOpen[0] = true;
  params.chanControlsOpen[1] = params.channels > 1;
  params.chanControlsOpen[2] = params.channels > 2;
  params.xyScopeOpen         = params.channels > 1;
  params.xyScopeControlsOpen = params.channels > 1;
  params.globalControlsOpen  = true;
  params.aboutOpen           = false;
  params.settingsOpen        = true; // TODO: false
  params.fullscreen          = false;
  params.enableMultiViewport = false;

  params.chanColor[0] = -231278815;
  params.chanColor[1] = -231003155;
  params.chanColor[2] = -219179238;
  params.triggeredColor    = -14096106;
  params.notTriggeredColor = -13167105;
  params.xyColor = 1025376039;

#ifdef _WIN32
  params.renderer = USC_REND_DIRECTX11_SDL;
#else
  params.renderer = USC_REND_OPENGL_SDL;
#endif

  std::string confPath;
  // remember when the only roadblock to a windows build was just building it?
  // well you can forget all that! file io is here to fuck it all up

#ifdef _WIN32
#else
  confPath+=getenv("HOME");
  if (confPath.size()==0) {
    printf(INFO_MSG "failed to get home directory! config will be saved next to executable..." MSG_END);
    // confPath+="./";
  } else {
    confPath+="/.config/unscope/";
  }
#endif

  confPath+=PROGRAM_CONF_FILE;

  USCConfig conf(confPath.c_str(), &params);
  bool loaded = (conf.loadConfig() == 0);

#define CONF_LOAD_BOOL(x) params.x = conf.getConfig(#x).as<bool>()
  if (loaded) {
    CONF_LOAD_BOOL(mainScopeOpen);
    params.chanControlsOpen[0] = conf.getConfig("chanControlsOpen1").as<bool>();
    params.chanControlsOpen[1] = conf.getConfig("chanControlsOpen2").as<bool>();
    params.chanControlsOpen[2] = conf.getConfig("chanControlsOpen3").as<bool>();
    CONF_LOAD_BOOL(xyScopeOpen);
    CONF_LOAD_BOOL(xyScopeControlsOpen);
    CONF_LOAD_BOOL(globalControlsOpen);
    CONF_LOAD_BOOL(fullscreen);
  }

#undef CONF_LOAD_BOOL

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
            printf("no value for argument %s given\n", argv[i]);
            continue;
          }
          try {
            value = std ::stoi(argv[i + 1]);
          } catch (...) {
            printf("invalid argument for %s given: %s\n", argv[i], argv[i + 1]);
            continue;
          }
          switch (argv[i][flagStartIndex]) {
            case UPARAM_BUFFERSIZE:
              params.audioBufferSize = value;
              break;
            case UPARAM_FRAMESIZE:
              params.audioFrameSize = value;
              break;
            case UPARAM_CHANNELCOUNT:
              params.channels = value;
              break;
            case UPARAM_SAMPLERATE:
              params.sampleRate = value;
              break;
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
            printf("no value for argument %s given\n", argv[i]);
            continue;
          }
          try {
            value = std::stoi(argv[i + 1]);
          } catch (...) {
            printf("invalid argument for %s given: %s\n", argv[i], argv[i + 1]);
            continue;
          }
          if (strcmp(argv[i] + flagStartIndex, "bufsize") == 0) {
            params.audioBufferSize = value;
            break;
          } else if (strcmp(argv[i] + flagStartIndex, "framesize") == 0) {
            params.audioFrameSize = value;
            break;
          } else if (strcmp(argv[i] + flagStartIndex, "channels") == 0) {
            params.channels = value;
            break;
          } else if (strcmp(argv[i] + flagStartIndex, "samplerate") == 0) {
            params.sampleRate = value;
            break;
          }
        }
      } else {
        printf("cannot parse argument %s\n", argv[i]);
      }
    }
  };

  USCGUI g(&params);
  g.attachConfig(&conf);

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

  printf(INFO_MSG "opening device %d: %s...%s",params.audioDevice,Pa_GetDeviceInfo(params.audioDevice)->name, MSG_END);
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

  g.deinint();

#define CONF_WIN(x) conf.setConfig(#x,params.x)
  CONF_WIN(mainScopeOpen);
  conf.setConfig("chanControlsOpen1", params.chanControlsOpen[0]);
  conf.setConfig("chanControlsOpen2", params.chanControlsOpen[1]);
  conf.setConfig("chanControlsOpen3", params.chanControlsOpen[2]);
  CONF_WIN(xyScopeOpen);
  CONF_WIN(xyScopeControlsOpen);
  CONF_WIN(globalControlsOpen);
  CONF_WIN(fullscreen);
#undef CONF_WIN

  conf.saveConfig();

  e = i.stop();
  printf( "%s%s%s", (e==paNoError)?SUCCESS_MSG:ERROR_MSG, Pa_GetErrorText(e), MSG_END);
  return e!=paNoError;
}

// #ifdef _WIN32
// int APIENTRY WinMain(HINSTANCE hInstance,
//     HINSTANCE hPrevInstance,
//     LPSTR lpCmdLine, int nCmdShow) {
//   return main(__argc, __argv);
// }
// #endif

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
" and Windows" // i may someday setup crosscompiling
#endif
"\n\n"
PROGRAM_NAME " is made using these libraries:\n\n"
"PortAudio (https://github.com/PortAudio/portaudio)\n"
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
  "OpenGL + SDL",
#endif
#ifdef USE_DIRECTX
  "DirectX 11 + SDL",
#endif
};

const int step_one = 1;
