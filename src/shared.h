#ifndef USC_SHARED_H
#define USC_SHARED_H

#include <vector>
#include <string>

#define PROGRAM_NAME "unscope"
#define PROGRAM_VER "0.2"

#define PROGRAM_WIDTH 1280
#define PROGRAM_HEIGHT 720

#define parseParams(p, argc, argv) \
  if (argc > 1) { \
    unsigned char flagStartIndex = 1; \
    int value = 0; \
    for (int i = 1; i < argc; i+=2) { \
      if (argv[i][0] == '-') { \
        if (argv[i][1] == '-') { \
          flagStartIndex = 2; \
        } \
        flagStartIndex = 1; \
        switch (argv[i][flagStartIndex]) { \
          case UPARAM_HELP: \
            printf("%s%s",verMsg,helpMsg); \
            return 0; \
          case UPARAM_VERSION: \
            printf("%s",verMsg); \
            return 0; \
          default: break; \
        } \
        if (i + 1 == argc) { \
          printf("no value for argument %s given\n", argv[i]); \
          continue; \
        } \
        try { \
          value = std::stoi(argv[i+1]); \
        } catch (...) { \
          printf("invalid argument for %s given: %s\n", argv[i], argv[i+1]); \
          continue; \
        } \
        switch (argv[i][flagStartIndex]) { \
          case UPARAM_BUFFERSIZE: \
            p.audioBufferSize = value; \
            break; \
          case UPARAM_FRAMESIZE: \
            p.audioFrameSize = value; \
            break; \
          case UPARAM_CHANNELCOUNT: \
            p.channels = value; \
            break; \
          case UPARAM_SAMPLERATE: \
            p.sampleRate = value; \
            break; \
          default: break; \
        } \
      } else { \
        printf("cannot parse argument %s\n",argv[i]); \
      } \
    } \
  }

#define ERROR_MSG "\033[31;1m"
#define INFO_MSG "\033[35;1m"
#define SUCCESS_MSG "\033[32;1m"
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
  UPARAM_HELP         = 'h',
  UPARAM_BUFFERSIZE   = 'b',
  UPARAM_FRAMESIZE    = 'f',
  UPARAM_CHANNELCOUNT = 'c',
  UPARAM_SAMPLERATE   = 's',
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
extern const char *helpMsg, *verMsg, *errMsgs[], *renderers[];
const char* getErrorMsg(int e);

#endif