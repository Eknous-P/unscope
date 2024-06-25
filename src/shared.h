#ifndef USC_SHARED_H
#define USC_SHARED_H

#include <vector>
#include <string>

#define PROGRAM_NAME "unscope"
#define PROGRAM_VER "0.2"

#define parseParams(p, argc, argv) { \
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
  } \
}

enum unscopeErrors {
  UAUDIOERR_NOERR=0,
  UAUDIOERR_NOINIT,
  UAUDIOERR_NODEVS,
  UAUDIOERR_NODEV,
  UAUDIOERR_NOSTART,
  UAUDIOERR_NOGOOD,
  UGUIERROR_SETUPFAIL,
  UGUIERROR_INITFAIL
};

enum unscopeArgs {
  UPARAM_HELP = 'h',
  UPARAM_BUFFERSIZE = 'b',
  UPARAM_FRAMESIZE = 'f',
  UPARAM_CHANNELCOUNT = 'c',
  UPARAM_SAMPLERATE = 's',
  UPARAM_VERSION = 'v'
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
  AlignParams(float t,
              unsigned long int wl,
              long int of,
              bool e,
              unsigned long int ho) {
    trigger = t;
    waveLen = wl;
    offset = of;
    edge = e;
    holdoff = ho;
  }
};

float clamp(float a);
extern const char *helpMsg, *verMsg, *errMsgs[];
std::string getErrorMsg(int e);

#endif