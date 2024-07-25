#include <portaudio.h>
#include <cstddef>
#include <memory.h>
#include <malloc.h>

#include "../shared.h"

#ifndef USC_AUDIO_H
#define USC_AUDIO_H

enum ProcessNodes {
  PNODE_BLANK,
  PNODE_MIXER,
  PNODE_DERIVER,
  PNODE_INTEGRATOR,
  PNODE_CLIPPER,
  PNODE_MULTIPLIER,
  PNODE_COUNT
};

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

struct ProcessNodeDefines {
  const char* name;
  unsigned char inputs;
};

struct ProcessNodeParam {
  const char* name;
  float value;
};

class ProcessNode {
    ProcessNodeDefines def;
    ProcessNodeParam params[4];
    float *input[4], *output;
  public:
    void attachTo(float* in);
    float* getOutput();
    virtual const char* getName();
    virtual void process();
};

class USCAudioProcess { // process audio data with various fx
  private:
    float **dataIn, **dataOut;
    unsigned long int dataSize, i;
    unsigned char channels;
    ProcessNode *nodes[256];
    unsigned short nodeCount;
  public:
    void writeDataIn(float* d, unsigned char chan);
    float *getDataOut(unsigned char chan);
    void doProcessing();
    // void derive();
    // void integrate();
    int addNode(ProcessNodes nodeId);

    USCAudioProcess(unsigned int bufferSizeDef, unsigned char chans);
    ~USCAudioProcess();
};

#endif