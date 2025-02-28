#ifndef USC_SHARED_H
#define USC_SHARED_H

#include <vector>
#include <string>
#include <memory.h>
#include <math.h>

// program stuff
#define PROGRAM_NAME "unscope"
#define PROGRAM_VER "0.3"

#define PROGRAM_WIDTH 1280
#define PROGRAM_HEIGHT 720

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
    delete x; \
    x = NULL; \
  }

#define DELETE_PTR_ARR(x) \
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

#define DELETE_DOUBLE_PTR_ARR(x,n) \
  if (x) { \
    FOR_RANGE(n) { \
      DELETE_PTR_ARR(x[z]) \
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
  USC_RENDER_NONE=0,
  USC_RENDER_OPENGL2,
  USC_RENDER_DIRECTX11,
};

struct unscopeParams {
  unsigned long int audioBufferSize;
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
  char* devName;
  DeviceEntry():
    dev(0),
    shouldPassThru(false),
    devName(NULL) {}
  DeviceEntry(int d, bool p, const char* dn) {
    dev = d;
    shouldPassThru = p;
    devName = new char[256];
    memcpy(devName, dn, 256);
  }
  // ~DeviceEntry() {
  //   printf("%p\n",(void*)devName);
  //   DELETE_PTR(devName);
  // }
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