#include "portaudio.h"
#include <cstddef>
#include <memory.h>
#include <malloc.h>

enum AudioErrors {
  NOERR=0,
  NOINIT,
  NODEVS,
  NODEV,
  NOSTART
};

class AudioInput {
  private:

    struct AudioConfig{
      int sampleRate;
      int frameSize;
      unsigned char channels;
    };

    struct AudioBuffer {
      float* data;
      unsigned long index;
      unsigned long size;
    };

    AudioConfig conf;
    AudioBuffer buffer;

    PaStream *stream;
    PaError err;
    PaStreamParameters streamParams;

    int bufferGetCallback(
      const void *inputBuffer, void *outputBuffer,
      unsigned long framesPerBuffer,
      const PaStreamCallbackTimeInfo* timeInfo,
      PaStreamCallbackFlags statusFlags);

    static int _PaCallback(
      const void *inputBuffer, void *outputBuffer,
      unsigned long framesPerBuffer,
      const PaStreamCallbackTimeInfo* timeInfo,
      PaStreamCallbackFlags statusFlags,
      void *userData );
  
  public:
    AudioInput();
    int init();
    int stop();
    void *getData();
    unsigned long getDataSize();
    ~AudioInput();
};