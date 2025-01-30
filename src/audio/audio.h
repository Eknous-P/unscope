#include <portaudio.h>

#include "../shared.h"

#ifndef USC_AUDIO_H
#define USC_AUDIO_H

class USCAudioInput { // get audio into a buffer and generate "alignment ramp"
  private:
    struct AudioConfig{
      PaDeviceIndex device;
      int sampleRate;
      int frameSize;
      unsigned char channels;
    };

    struct AudioBuffer {
      float **data, **dataCopy;
      unsigned long int size;
      unsigned long int index;
    };

    AudioConfig conf;
    AudioBuffer buffer;

    std::vector<DeviceEntry> devs;
  
    PaStream *stream;
    PaError err;
    PaStreamParameters streamParams;

    bool isGood, running, updateAudio;
    unsigned long int holdoffTimer;

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

      void align(unsigned char chan);

  public:
    std::vector<DeviceEntry> enumerateDevs();
    int init(PaDeviceIndex dev, bool loopback);
    int stop();

    unsigned char getChannelCount();
    float **getData();
    const PaDeviceInfo* getDeviceInfo();
    bool didTrigger(unsigned char chan);
    void setUpdateState(bool u);

    USCAudioInput(unscopeParams *params);
    ~USCAudioInput();
};

#endif