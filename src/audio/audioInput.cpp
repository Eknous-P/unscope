#include "audio.h"

#define CHECK_TRIGGERED triggered[j]=triggerLow[j]&&triggerHigh[j]

USCAudioInput::USCAudioInput(unscopeParams *params) {
  isGood = Pa_Initialize()==paNoError;
  running = false;
  conf.channels = params->channels;
  if (conf.channels > 3) conf.channels = 3; // implot limitation...
  conf.sampleRate = params->sampleRate;
  conf.frameSize = params->audioFrameSize;

  conf.device = 0;

  buffer.size = params->audioBufferSize;
  buffer.data = new float*[conf.channels];
  buffer.dataCopy = new float*[conf.channels];

  holdoffTimer = 0;

  if (!buffer.data) {
    isGood = false;
    return;
  }
  for (unsigned char i = 0; i < conf.channels; i++) {
    buffer.data[i] = new float[buffer.size];
    buffer.dataCopy[i] = new float[buffer.size];
    if (!buffer.data[i] || !buffer.dataCopy[i]) {
      isGood = false;
      return;
    }
    memset(buffer.data[i],      0, buffer.size*sizeof(float));
    memset(buffer.dataCopy[i],  0, buffer.size*sizeof(float));
  }

  updateAudio = true;
}

int USCAudioInput::_PaCallback(
  const void *inputBuffer, void *outputBuffer,
  unsigned long int framesPerBuffer,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void *userData ) {
    return ((USCAudioInput*)userData)->bufferGetCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}

int USCAudioInput::bufferGetCallback(
  const void *inputBuffer, void *outputBuffer,
  unsigned long int framesPerBuffer,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags) {
    const float *audIn = (const float*)inputBuffer;
    float *audOut = (float*)outputBuffer;
  
    (void) outputBuffer;
    (void) timeInfo;
    (void) statusFlags;

    unsigned char j = 0;

    const int nChans = conf.channels;
  
    unsigned int *triggerPoint = new unsigned int[nChans];
    memset(triggerPoint,0,conf.channels*sizeof(int));
  
    bool *triggerLow = new bool[nChans];
    bool *triggerHigh = new bool[nChans];

    memset(triggerLow,0,conf.channels*sizeof(bool));
    memset(triggerHigh,0,conf.channels*sizeof(bool));

    if (inputBuffer==NULL) {
      for (buffer.index = 0; buffer.index < buffer.size*conf.channels; buffer.index++) {
        buffer.data[buffer.index] = 0;
        if (outputBuffer != NULL) *audOut++ = 0;
      }
    } else {
      // push vaules back
      for (j = 0; j < conf.channels; j++) {
        memcpy(buffer.data[j],
        buffer.data[j] + framesPerBuffer,
        (buffer.size-framesPerBuffer)*sizeof(float));
      }
      // get data
      for (buffer.index = 0; buffer.index < framesPerBuffer; buffer.index++) {
        for (j = 0; j < conf.channels; j++) {
          float dat = *audIn++;
          buffer.data[j][buffer.size - framesPerBuffer + buffer.index] = dat;
          if (outputBuffer != NULL) *audOut++ = dat;
        }
      }
      if (updateAudio) {
        for (j = 0; j < conf.channels; j++) memcpy(buffer.dataCopy[j],buffer.data[j],buffer.size*sizeof(float));
      }
      // for (j = 0; j < conf.channels; j++) {
      //   if(!trigger->trigger(j, alignParams->waveLen)) {}
      // }
      // buffer.alignRamp = trigger->getAlignBuffer();
    }

    delete[] triggerPoint;
    delete[] triggerLow;
    delete[] triggerHigh;

    return paContinue;
}

std::vector<DeviceEntry> USCAudioInput::enumerateDevs() {
  devs.clear();
  devs.push_back(DeviceEntry(Pa_GetDefaultInputDevice(),true,"Default device"));
  for (int i = 0; i < Pa_GetDeviceCount(); i++) {
    const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
    if (info == NULL) continue;
    // if (info->maxInputChannels < conf.channels) continue;
    if (info->maxOutputChannels > 0) continue;
    if (info->defaultSampleRate < conf.sampleRate) continue;
    devs.push_back(DeviceEntry(i, info->maxInputChannels == info->maxOutputChannels,
      std::to_string(i) + ": " +
      std::string(Pa_GetHostApiInfo(info->hostApi)->name) + " | " + 
      std::string(info->name)));
  }
  printf(INFO_MSG "%lu devices found" MSG_END, devs.size());
  return devs;
}

int USCAudioInput::init(PaDeviceIndex dev, bool loopback) {
  if (!isGood) return UAUDIOERR_NOGOOD;
  if (running) return UAUDIOERR_NOERR;
  int nDevs = Pa_GetDeviceCount();
  if (nDevs == 0) return UAUDIOERR_NODEVS;
  if (nDevs < 0) return nDevs;

  streamParams.device = dev;
  if (streamParams.device == paNoDevice) return UAUDIOERR_NODEV;
  streamParams.channelCount = conf.channels;
  streamParams.sampleFormat = paFloat32;
  streamParams.suggestedLatency = Pa_GetDeviceInfo(streamParams.device)->defaultLowInputLatency;
  streamParams.hostApiSpecificStreamInfo = NULL;
  err = Pa_OpenStream(
    &stream,
    &streamParams,
    loopback?&streamParams:NULL,
    conf.sampleRate,
    conf.frameSize,
    paClipOff,
    &USCAudioInput::_PaCallback,
    this
  );

  if (err == paInvalidChannelCount) {
    if (streamParams.channelCount == Pa_GetDeviceInfo(dev)->maxInputChannels) {
      running = err==paNoError;
      return err;
    }
    printf(INFO_MSG "trying with device-preferred channel count..." MSG_END);
    streamParams.channelCount = Pa_GetDeviceInfo(dev)->maxInputChannels;
    conf.channels = streamParams.channelCount;
    err = Pa_OpenStream(
      &stream,
      &streamParams,
      loopback?&streamParams:NULL,
      conf.sampleRate,
      conf.frameSize,
      paClipOff,
      &USCAudioInput::_PaCallback,
      this
    );

    if (err == paInvalidChannelCount) {
      if (streamParams.channelCount == Pa_GetDeviceInfo(dev)->maxInputChannels) {
        running = err==paNoError;
        return err;
      }
      printf(ERROR_MSG "retry failed!" MSG_END);
      return err;
    }
  }

  if (err!=paNoError) return err;

  err = Pa_StartStream(stream);

  
  conf.device=dev;
  running = err==paNoError;
  return err;
}

int USCAudioInput::stop() {
  if (!running) return 0;
  running = false;
  return Pa_CloseStream(stream);
}

bool USCAudioInput::didTrigger(unsigned char chan) {
  // if (chan == 255) {
  //   for (unsigned char i = 0; i < conf.channels; i++) if (triggered[i]) return true;
  //   return false;
  // }
  // return triggered[chan];
  return false;
}

void USCAudioInput::setUpdateState(bool u) {
  updateAudio = u;
}

float **USCAudioInput::getData() {
  return buffer.dataCopy;
}

const PaDeviceInfo *USCAudioInput::getDeviceInfo() {
  return Pa_GetDeviceInfo(conf.device);
}

unsigned char USCAudioInput::getChannelCount() {
  return conf.channels;
}

USCAudioInput::~USCAudioInput() {
  if (isGood) Pa_Terminate();
  DELETE_DOUBLE_PTR(buffer.data, conf.channels)
  DELETE_DOUBLE_PTR(buffer.dataCopy, conf.channels)
}