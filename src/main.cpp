#include "audio/audio.h"
#include "gui/gui.h"
#include "../shared.h"

float clamp(float a) {
  if (a > 1.0f) return 1.0f;
  if (a < -1.0f) return -1.0f;
  return a;
}

const char* getErrorMsg(int e) {
  switch (e) {
    case UAUDIOERR_NOERR:
      return "";
    case UAUDIOERR_NODEVS:
    case UAUDIOERR_NODEV:
    case UAUDIOERR_NOSTART:
    case UAUDIOERR_NOGOOD:
    case UGUIERROR_SETUPFAIL:
    case UGUIERROR_INITFAIL:
      return errMsgs[e-2];
    default: 
      return Pa_GetErrorText(e);
  }
}

int main(int argc, char** argv) {
  int e;
  unscopeParams params;

  params.audioBufferSize = 65536; // N samples
  params.audioFrameSize = 512;    // N samples
  params.channels = 2;            // N (1-3)
  params.sampleRate = 48000;      // Hz (only common values?)
  params.audioDevice = 0;         // internal ID, gets overwritten anyway
  params.timebase = 60;           // ms
  params.xyPersist = 40;          // ms
  params.scale = 1.0f;            // (no unit)
  params.trigger = 0.0f;          // (no unit)
#ifdef _WIN32
  params.renderer = USC_REND_DIRECTX11_SDL;
#else
  params.renderer = USC_REND_OPENGL_SDL;
#endif

  parseParams(params, argc, argv);

  USCGUI g(params);

  e = g.init();
  if (e != 0) {
    printf(ERROR_MSG "error in initializing GUI! %s\n" MSG_END,getErrorMsg(e));
    return -1;
  }

  USCAudioInput i(params.audioFrameSize,
               params.audioBufferSize,
               params.channels,
               params.sampleRate);

  g.attachAudioInput(&i);

  params.audioDevice = Pa_GetDefaultInputDevice();

  g.getDevices(i.enumerateDevs());
  g.setAudioDeviceSetting(params.audioDevice);

  printf(INFO_MSG "opening device %d: %s ...\n" MSG_END,params.audioDevice,Pa_GetDeviceInfo(params.audioDevice)->name);
  e = i.init(params.audioDevice,0);
  if (e != paNoError) {
    printf(ERROR_MSG "%d:cant init audio!\n%s" MSG_END, e, getErrorMsg(e));
    // try again
    if (e != paInvalidDevice) {
      printf(ERROR_MSG "%d:cant init audio!\n%s" MSG_END, e, getErrorMsg(e));
      return e;
    }
    printf(INFO_MSG "trying default device...\n" MSG_END);
    params.audioDevice = Pa_GetDefaultInputDevice();
    e = i.init(params.audioDevice,0);
    if (e != paNoError) {
      printf(ERROR_MSG "%d:cant init audio!\n%s" MSG_END, e, getErrorMsg(e));
      return e;
    }
    g.setAudioDeviceSetting(params.audioDevice);
  }
  while (g.isRunning()) {
    g.doFrame();
  }
  e = i.stop();
  printf( "%s%s\n" MSG_END,(e==paNoError)?SUCCESS_MSG:ERROR_MSG, Pa_GetErrorText(e));
  return e!=paNoError;
}

#ifdef _WIN32
int APIENTRY WinMain(HINSTANCE hInstance,
    HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow) {
    return main(__argc, __argv);
}
#endif

const char* verMsg = PROGRAM_NAME " (version " PROGRAM_VER ")\n";
const char* helpMsg =
"Program arguments\n"
"  -h: print this message\n"
"  -b: set the audio buffer size (default: 65536)\n"
"  -f: set the audio frame size (default: 512)\n"
"  -c: set the channel amount (default: 2)\n"
"  -s: set the sample rate (default: 48000)\n"
"  -v: print the program version\n";
const char* errMsgs[] = {
  "no devices found\n",
  "cant open device\n",
  "cant start device\n",
  "cant init portaudio\n",
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
  ""
};