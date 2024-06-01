#include <portaudio.h>
#include <cstddef>
#include <memory.h>
#include <malloc.h>

#include "../shared.h"

#ifndef USC_AUDIO_H
#define USC_AUDIO_H

class AudioInput { // get audio into a buffer and generate "alignment ramp"
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
      float *alignRamp;
      unsigned long int size;
      unsigned long int index;
    };

    AudioConfig conf;
    AudioBuffer buffer;

    std::vector<DeviceEntry> devs;
  
    PaStream *stream;
    PaError err;
    PaStreamParameters streamParams;

    bool isGood, running, triggered;

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
    AudioInput(unsigned int frameSize, unsigned int bufferSize,unsigned char channelsDef, unsigned int sampleRateDef);
    std::vector<DeviceEntry> enumerateDevs();
    int init(PaDeviceIndex dev);
    int stop();
    unsigned char getChannelCount();
    float *getData(unsigned char chan);
    const PaDeviceInfo* getDeviceInfo();
    ~AudioInput();
    bool didTrigger();
    float *getAlignRamp(unsigned char chan, float trigger,unsigned long int waveLen, long int offset, bool edge); // true -> falling, false ->rising
};

class AudioProcess { // process audio data with various fx
  private:
    float **dataIn, **dataOut;
    unsigned long int dataSize, i;
    unsigned char channels;
  public:
    void writeDataIn(float* d, unsigned char chan);
    float *getDataOut(unsigned char chan);
    void derive();
    void integrate();

    AudioProcess(unsigned int bufferSizeDef, unsigned char chans);
    ~AudioProcess();
};

#endif