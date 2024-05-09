#include "portaudio.h"
#include <cstddef>
#include <malloc.h>

class AudioInput {
  private:

    struct AudioConfig{
      int sampleRate;
      int frameSize;
      unsigned char channels;
    };

    struct AudioBuffer {
      float *data;
      unsigned long index;
      unsigned long size;
    };

    AudioConfig conf;
    AudioBuffer buffer;

    PaStream *stream;
    PaError err;
    PaStreamParameters streamParams;

  public:

    int bufferGetCallback(
      const void *input, void *output,
      unsigned long frameCount,
      const PaStreamCallbackTimeInfo* timeInfo,
      PaStreamCallbackFlags statusFlags);

    static int _PaCallback(
      const void *input, void *output,
      unsigned long frameCount,
      const PaStreamCallbackTimeInfo* timeInfo,
      PaStreamCallbackFlags statusFlags,
      void *userData);

    AudioInput();
    int init();
    int stop();
    void *getData();
    unsigned long getDataSize();
    ~AudioInput();
};