#include <portaudio.h>
#include <cstddef>
#include <memory.h>
#include <malloc.h>

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
      unsigned char channelsDef;
    };

    struct AudioBuffer {
      float **data;
      float **alignRamp;
      unsigned long int size;
      unsigned long int index;
    };

    AudioConfig conf;
    AudioBuffer buffer;
    AlignParams alignParams[3] = {AlignParams(0,0,0,0,0),AlignParams(0,0,0,0,0),AlignParams(0,0,0,0,0)}; // i hate that i have to init the values in the header but otherwise it doesnt work!! help!!

    std::vector<DeviceEntry> devs;
  
    PaStream *stream;
    PaError err;
    PaStreamParameters streamParams;

    bool isGood, running, triggered, doUpdate;
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
    float *getData(unsigned char chan);
    const PaDeviceInfo* getDeviceInfo();
    bool didTrigger();
    void setUpdateState(bool u);
    void setAlignParams(unsigned char chan, AlignParams ap);
    float *getAlignRamp(unsigned char c);

    USCAudioInput(unsigned int frameSize, unsigned int bufferSize, unsigned char channelsDef, unsigned int sampleRateDef);
    ~USCAudioInput();
};

#endif