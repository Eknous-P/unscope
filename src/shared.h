#ifndef USC_SHARED_H
#define USC_SHARED_H

#include <vector>
#include <string>

#define PROGRAM_NAME "unscope"
#define PROGRAM_VER "0"

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

enum AudioErrors {
  UAUDIOERR_NOERR=0,
  UAUDIOERR_NOINIT,
  UAUDIOERR_NODEVS,
  UAUDIOERR_NODEV,
  UAUDIOERR_NOSTART,
  UAUDIOERR_NOGOOD
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
  std::string devName;
  DeviceEntry(int d, std::string dn) {
    dev = d;
    devName = dn;
  }
};

extern const char *helpMsg, *verMsg, *errMsgs[];
std::string getErrorMsg(int e);

#endif