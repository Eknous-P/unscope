#ifndef USC_AUDIO_H
#define USC_AUDIO_H

#include <portaudio.h>
#include "shared.h"

class USCAudioInput { // get audio into a buffer
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
    std::vector<DeviceEntry> enumerateDevs();
    int init(PaDeviceIndex dev, bool loopback);
    int stop();

    unsigned char getChannelCount();
    float **getData();
    const PaDeviceInfo* getDeviceInfo();
    void setUpdateState(bool u);

    USCAudioInput(unscopeParams *params);
    ~USCAudioInput();
};

#endif