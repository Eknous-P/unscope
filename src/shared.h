#ifndef USC_SHARED_H
#define USC_SHARED_H

#include <vector>
#include <string>

#define PROGRAM_NAME "unscope"
#define PROGRAM_VER "0.3rc2"

#define PROGRAM_WIDTH 1280
#define PROGRAM_HEIGHT 720

#define ERROR_MSG "\033[31;1m"
#define INFO_MSG "\033[35;1m"
#define SUCCESS_MSG "\033[32;1m"
#define MISC_MSG "\033[34;1m"
#define MSG_END "\033[0m"

enum unscopeErrors {
  UAUDIOERR_NOERR = 0,
  UAUDIOERR_NOINIT,
  UAUDIOERR_NODEVS,
  UAUDIOERR_NODEV,
  UAUDIOERR_NOSTART,
  UAUDIOERR_NOGOOD,

  UGUIERROR_SETUPFAIL,
  UGUIERROR_INITFAIL
};

enum unscopeArgs {
  UPARAM_BUFFERSIZE   = 'b',
  UPARAM_FRAMESIZE    = 'f',
  UPARAM_CHANNELCOUNT = 'c',
  UPARAM_SAMPLERATE   = 's',
  UPARAM_ABOUT        = 'a',
  UPARAM_HELP         = 'h',
  UPARAM_VERSION      = 'v'
};

enum USCRenderers {
  USC_REND_OPENGL_SDL = 0,
  USC_REND_DIRECTX11_SDL
};

struct unscopeParams {
  unsigned int audioBufferSize;
  unsigned int audioFrameSize;
  unsigned char channels;
  unsigned int sampleRate;
  int audioDevice;

  // gui ...
  float timebase;
  float xyPersist;
  float scale;
  float trigger;

  int renderer;
};

struct DeviceEntry {
  int dev;
  bool shouldPassThru;
  std::string devName;
  DeviceEntry(int d, bool p, std::string dn) {
    dev = d;
    shouldPassThru = p;
    devName = dn;
  }
};

struct AlignParams {
  float trigger;
  unsigned long int waveLen;
  long int offset;
  bool edge; // true -> falling, false -> rising
  unsigned long int holdoff;
  bool fallback; // use fixed sweep if not triggering
  AlignParams():
    trigger(0.0f),
    waveLen(0),
    offset(0),
    edge(false),
    holdoff(0),
    fallback(false)
  {}
  
  AlignParams(float t,
              unsigned long int wl,
              long int of,
              bool e,
              unsigned long int ho,
              bool fb) {
    trigger  = t;
    waveLen  = wl;
    offset   = of;
    edge     = e;
    holdoff  = ho;
    fallback = fb;
  }
};

float clamp(float a);
extern const char *helpMsg, *verMsg, *aboutMsg, *errMsgs[], *renderers[];
const char* getErrorMsg(int e);

#endif