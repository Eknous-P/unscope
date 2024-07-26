#include <portaudio.h>
#include <cstddef>
#include <memory.h>
#include <malloc.h>

#include "../shared.h"

#ifndef USC_AUDIO_H
#define USC_AUDIO_H

enum ProcessNodes {
  PNODE_BLANK = 0,

  PNODE_INPUTS = 1000,
  PNODE_OUTPUTS = 999,

  PNODE_MIXER = 1,
  PNODE_DERIVE,
  PNODE_INTEGRATE,
  PNODE_DISTORTION,
  PNODE_MULTIPLIER,
  PNODE_COUNT
};

struct ProcessNodeDefines {
  const char* name;
  unsigned char inputs;
  unsigned char paramCount;
  ProcessNodeDefines(const char* n, unsigned char i, unsigned char p) {
    name = n;
    inputs = i;
    paramCount = p;
  }
};

struct ProcessNodeParam {
  const char* name;
  float value;
  float vMin, vMax;
};

// WHY DO I HAVE TO DO THIS??!
ProcessNodeParam PNPInit(const char* n, float v, float mn, float mx);

class ProcessNode {
    float *inputs[4], *output;
  public:
    ProcessNodeParam params[4];
    void attachTo(float* in, unsigned char _input);
    float* getOutput();
    virtual ProcessNodeDefines getDefines();
    virtual void process();
};

class USCAudioProcess { // process audio data with various fx
  private:
    float **dataIn, **dataOut;
    unsigned long int dataSize, i;
    unsigned char channels;
    std::vector<ProcessNode *> nodes;
  public:
    void writeDataIn(float* d, unsigned char chan);
    float *getDataOut(unsigned char chan);
    void doProcessing();
    // returns a pointer to the new node
    ProcessNode *addNode(ProcessNodes nodeId);

    USCAudioProcess(unsigned int bufferSizeDef, unsigned char chans);
    ~USCAudioProcess();
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

    USCAudioProcess* ap;

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

    void attachAudioProcess(USCAudioProcess* p);
    USCAudioProcess* getAudioProcess();

    USCAudioInput(unsigned int frameSize, unsigned int bufferSize, unsigned char channelsDef, unsigned int sampleRateDef);
    ~USCAudioInput();
};

#endif