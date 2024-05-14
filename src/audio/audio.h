#include <portaudio.h>
#include <cstddef>
#include <memory.h>
#include <malloc.h>

enum AudioErrors {
  NOERR=0,
  NOINIT,
  NODEVS,
  NODEV,
  NOSTART,
  NOGOOD
};

class AudioInput {
  private:

    struct AudioConfig{
      PaDeviceIndex device;
      int sampleRate;
      int frameSize;
      unsigned char channels;
    };

    struct AudioBuffer {
      float* data;
      unsigned long int index;
      unsigned long int size;
    };

    AudioConfig conf;
    AudioBuffer buffer;

    PaStream *stream;
    PaError err;
    PaStreamParameters streamParams;

    bool isGood, running;

    int bufferGetCallback(
      const void *inputBuffer, void *outputBuffer,
      unsigned long int framesPerBuffer,
      const PaStreamCallbackTimeInfo* timeInfo,
      PaStreamCallbackFlags statusFlags);

    static int _PaCallback(
      const void *inputBuffer, void *outputBuffer,
      unsigned long int framesPerBuffer,
      const PaStreamCallbackTimeInfo* timeInfo,
      PaStreamCallbackFlags statusFlags,
      void *userData );
  
  public:
    AudioInput();
    int init(PaDeviceIndex dev);
    int stop();
    float *getData();
    unsigned long int getDataSize();
    const PaDeviceInfo* getDeviceInfo();
    ~AudioInput();
};