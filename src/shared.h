#ifndef USC_SHARED_H
#define USC_SHARED_H

#include <vector>
#include <string>
#include <memory.h>

// program stuff
#define PROGRAM_NAME "unscope"
#define PROGRAM_VER "0.3"

#define PROGRAM_WIDTH 1280
#define PROGRAM_HEIGHT 720

#define PROGRAM_CONF_FILE "unscope.yaml"

// printf fancies
#define ERROR_MSG "\033[31;1m"
#define INFO_MSG "\033[35;1m"
#define SUCCESS_MSG "\033[32;1m"
#define MISC_MSG "\033[34;1m"
#define MSG_END "\033[0m\n"

// frequently used stuff
#define FOR_RANGE(c) for (unsigned char z = 0; z < c; z++)
#define NEW_DOUBLE_PTR(x,type,size,count) \
  x = new type*[count]; \
  if (x) { \
    FOR_RANGE(count) { \
      x[z] = new type[size]; \
    } \
  }
#define DELETE_PTR(x) \
  if (x) { \
    delete[] x; \
    x = NULL; \
  }

#define DELETE_DOUBLE_PTR(x,n) \
  if (x) { \
    FOR_RANGE(n) { \
      DELETE_PTR(x[z]) \
    } \
    delete[] x; \
    x = NULL; \
  }

// ui fancies
#define KNOBS_SIZE 50.0f

#define RIGHTCLICK_EXACT_INPUT(v,d,f) \
        if (ImGui::BeginPopupContextItem(#v "input")) { \
          if (ImGui::InputScalar("##" #v "input",d,v)) f; \
          ImGui::EndPopup(); \
        }

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
  // audio
  long int audioBufferSize;
  int audioFrameSize;
  char channels;
  int sampleRate;
  int audioDevice;

  // gui
  float timebase;
  float xyPersist;
  float scale;

  int renderer;

  // colors
  int chanColor[3];
  int triggeredColor;
  int fallbackColor;
  int notTriggeredColor;
  int xyColor;
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
extern const int step_one;

#endif