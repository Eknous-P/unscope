#include "audio.h"

USCAudioInput::USCAudioInput(unsigned int frameSize, unsigned int bufferSize, unsigned char channelsDef, unsigned int sampleRateDef) {
  isGood = Pa_Initialize()==paNoError;
  running = false;
  conf.channels = channelsDef;
  if (conf.channels > 3) conf.channels = 3; // implot limitation...
  conf.channelsDef = channelsDef;
  conf.sampleRate = sampleRateDef;
  conf.frameSize = frameSize;

  conf.device = 0;

  buffer.size = bufferSize;
  buffer.data = new float*[conf.channels];
  buffer.alignRamp = new float*[buffer.size];

  holdoffTimer = 0;

  if (!buffer.data) {
    isGood = false;
    return;
  }
  for (unsigned char i = 0; i < conf.channels; i++) {
    buffer.data[i] = new float[buffer.size];
    buffer.alignRamp[i] = new float[buffer.size];
    if (!buffer.data[i] || !buffer.alignRamp[i]) {
      isGood = false;
      return;
    }
    memset(buffer.data[i],0,buffer.size*sizeof(float));
    memset(buffer.alignRamp[i], 0, buffer.size*sizeof(float));
  }

  doUpdate = true;
  triggered = new bool[conf.channels];
  memset(triggered,0,conf.channels*sizeof(bool));

  // alignParams = (AlignParams*)malloc(conf.channels*sizeof(AlignParams));
  // if (!alignParams) {
  //   isGood = false;
  //   printf("bru you fucked it up\n");
  //   return;
  // }
  // alignParams = new AlignParams[conf.channels];
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

    memset(triggered,0,conf.channels*sizeof(bool));
  
    unsigned int triggerPoint[conf.channels];
    memset(triggerPoint,0,conf.channels*sizeof(int));
  
    bool triggerLow[conf.channels];
    bool triggerHigh[conf.channels];

    memset(triggerLow,0,conf.channels*sizeof(bool));
    memset(triggerHigh,0,conf.channels*sizeof(bool));

    if (inputBuffer==NULL) {
      for (buffer.index = 0; buffer.index < buffer.size*conf.channels; buffer.index++) {
        buffer.data[buffer.index] = 0;
        if (outputBuffer != NULL) *audOut++ = 0;
      }
    } else {
      if (doUpdate) {
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
            // find trigger point
            // if (!triggered[j]) {
            //   triggered[j] = (triggerLow[j] && triggerHigh[j]);
            //   if (dat < alignParams[j].trigger) {
            //     triggerLow[j] = true;
            //     if (triggered[j] && alignParams[j].edge) triggerPoint[j] = buffer.index;
            //   }
            //   if (dat > alignParams[j].trigger) {
            //     triggerHigh[j] = true;
            //     if (triggered[j] && !alignParams[j].edge) triggerPoint[j] = buffer.index;
            //   }
            // }
          }
        }
      }
      for (j = 0; j < conf.channels; j++) { // trigger
        unsigned long int i = 0;
        float delta = 0;
        if (alignParams[j].holdoff == 0) holdoffTimer = 0;
        if (holdoffTimer>0) {
          holdoffTimer--;
          delta = ((float)buffer.size/((float)alignParams[j].waveLen))/(float)buffer.size;
          for (;i<buffer.size;i++) {
            buffer.alignRamp[j][i]-=delta;
          }
        }
        else {
          holdoffTimer = alignParams[j].holdoff;
        }
        memset(buffer.alignRamp[j],-1.0f,sizeof(float)*buffer.size);

        i = buffer.size - alignParams[j].waveLen;
        if (!triggered[j]) {
          memset(triggerLow,0,conf.channels*sizeof(bool));
          memset(triggerHigh,0,conf.channels*sizeof(bool));
          i -= framesPerBuffer;
          while (i != 0 && i > (buffer.size - 2*alignParams[j].waveLen)) {
            triggered[j] = triggerLow[j] && triggerHigh[j];
            i--;
            if (buffer.data[j][i] < alignParams[j].trigger) {
              triggerLow[j] = true;
              if (triggered[j] && alignParams[j].edge) break;
            }
            if (buffer.data[j][i] > alignParams[j].trigger) {
              triggerHigh[j] = true;
              if (triggered[j] && !alignParams[j].edge) break;
            }
          }
          triggerPoint[j]  =i;
        } else {
          triggerPoint[j] = i  + triggerPoint[j];
        }

        if (triggered[j]) {
          delta = 2.0f/(float)(alignParams[j].waveLen);
          for (;i < buffer.size; i++) {
            buffer.alignRamp[j][i-alignParams[j].offset] = -1.0f + delta*(i - triggerPoint[j]);
            if (buffer.alignRamp[j][i-alignParams[j].offset] >= 1.0f) {
              align(0);
            }
          }
        } else {
          delta = ((float)buffer.size/((float)alignParams[j].waveLen))/(float)buffer.size;
          buffer.alignRamp[j][buffer.size-1] = 1.0f;
          for (i = buffer.size-2; i > 0; i--) {
            buffer.alignRamp[j][i] = clamp(buffer.alignRamp[j][i+1] - 2*delta);
          }
          buffer.alignRamp[j][0] = -1.0f;
        }

        i = 0;
      }
    }
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
  printf("%lu devices found\n", devs.size());
  return devs;
}

int USCAudioInput::init(PaDeviceIndex dev, bool loopback) {
  if (!isGood) return UAUDIOERR_NOGOOD;
  if (running) return UAUDIOERR_NOERR;
  int nDevs = Pa_GetDeviceCount();
  if (nDevs == 0) return UAUDIOERR_NODEVS;
  if (nDevs < 0) return nDevs;

  conf.channels = conf.channelsDef;
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
    printf("trying with device-preferred channel count...\n");
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
      printf("retry failed!\n");
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

void USCAudioInput::setAlignParams(unsigned char chan, AlignParams ap) {
  alignParams[chan] = ap;
}

void USCAudioInput::align(unsigned char chan) {
}

float *USCAudioInput::getAlignRamp(unsigned char c) {
  return buffer.alignRamp[c];
}

bool USCAudioInput::didTrigger() {
  return triggered[0];
}

void USCAudioInput::setUpdateState(bool u) {
  doUpdate = u;
}

float *USCAudioInput::getData(unsigned char chan) {
  return (buffer.data)[chan];
}

const PaDeviceInfo *USCAudioInput::getDeviceInfo() {
  return Pa_GetDeviceInfo(conf.device);
}

unsigned char USCAudioInput::getChannelCount() {
  return conf.channels;
}

USCAudioInput::~USCAudioInput() {
  if (isGood) Pa_Terminate();
  if (buffer.data) {
    for (unsigned char i = 0; i < conf.channels; i++) {
      if (buffer.data[i]) {
        delete[] buffer.data[i];
        buffer.data[i] = NULL;
      }
    }
    delete[] buffer.data;
    buffer.data = NULL;
  }
  if (buffer.alignRamp) {
    for (unsigned char i = 0; i < conf.channels; i++) {
      if (buffer.alignRamp[i]) {
        delete[] buffer.alignRamp[i];
        buffer.alignRamp[i] = NULL;
      }
    }
    delete[] buffer.alignRamp;
    buffer.alignRamp = NULL;
  }
  if (triggered) {
    delete[] triggered;
    triggered = NULL;
  }
}