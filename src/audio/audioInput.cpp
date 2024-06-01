#include "audio.h"

AudioInput::AudioInput(unsigned int frameSize, unsigned int bufferSize, unsigned char channelsDef, unsigned int sampleRateDef) {
  isGood = Pa_Initialize()==paNoError;
  running = false;
  conf.channels = channelsDef;
  if (conf.channels > 3) conf.channels = 3; // implot limitation...
  conf.sampleRate = sampleRateDef;
  conf.frameSize = frameSize;

  conf.device = 0;

  buffer.size = bufferSize;
  buffer.data = new float*[conf.channels];
  buffer.alignRamp = new float[buffer.size];
  memset(buffer.alignRamp, 0, buffer.size*sizeof(float));
  if (!buffer.data) {
    isGood = false;
    return;
  }
  for (unsigned char i = 0; i < conf.channels; i++) {
    buffer.data[i] = new float[buffer.size];
    if (!buffer.data[i]) {
      isGood = false;
      return;
    }
    memset(buffer.data[i],0,buffer.size*sizeof(float));
  }
}

int AudioInput::_PaCallback(
  const void *inputBuffer, void *outputBuffer,
  unsigned long int framesPerBuffer,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags,
  void *userData ) {
    return ((AudioInput*)userData)->bufferGetCallback(inputBuffer, outputBuffer, framesPerBuffer, timeInfo, statusFlags);
}

int AudioInput::bufferGetCallback(
  const void *inputBuffer, void *outputBuffer,
  unsigned long int framesPerBuffer,
  const PaStreamCallbackTimeInfo* timeInfo,
  PaStreamCallbackFlags statusFlags) {
    const float *audIn = (const float*)inputBuffer;
  
    (void) outputBuffer;
    (void) timeInfo;
    (void) statusFlags;

    unsigned char j = 0;

    if (inputBuffer==NULL) {
      for (buffer.index = 0; buffer.index < buffer.size*conf.channels; buffer.index++) {
        buffer.data[buffer.index] = 0;
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
          buffer.data[j][buffer.size - framesPerBuffer + buffer.index] = *audIn++;
        }
      }
    }
    return paContinue;
}

std::vector<DeviceEntry> AudioInput::enumerateDevs() {
  devs.clear();
  devs.push_back(DeviceEntry(Pa_GetDefaultInputDevice(),true,"Default device"));
  for (int i = 0; i < Pa_GetDeviceCount(); i++) {
    const PaDeviceInfo *info = Pa_GetDeviceInfo(i);
    if (info == NULL) continue;
    if (info->maxInputChannels < 1) continue;
    if (info->maxInputChannels > conf.channels) continue;
    if (info->maxOutputChannels > 0) continue;
    if (info->defaultSampleRate < conf.sampleRate) continue;
    devs.push_back(DeviceEntry(i, true,
      std::to_string(i) + ": " +
      std::string(Pa_GetHostApiInfo(info->hostApi)->name) + " | " + 
      std::string(info->name)));
  }
  printf("%lu devices found\n", devs.size());
  return devs;
}

int AudioInput::init(PaDeviceIndex dev) {
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
    NULL,
    conf.sampleRate,
    conf.frameSize,
    paClipOff,
    &AudioInput::_PaCallback,
    this
  );
  Pa_Sleep(100);

  if (err!=paNoError) return err;

  err = Pa_StartStream(stream);
  
  conf.device=dev;
  running = err==paNoError;
  return err;
}

int AudioInput::stop() {
  if (!running) return 0;
  running = false;
  return Pa_CloseStream(stream);
}

inline float clamp(float a) {
  if (a > 1.0f) return 1.0f;
  if (a < -1.0f) return -1.0f;
  return a;
}

float *AudioInput::getAlignRamp(unsigned char chan, float trigger, unsigned long int waveLen, long int offset, bool edge=false) {
  memset(buffer.alignRamp,-1.0f,sizeof(float)*buffer.size);

  unsigned int triggerPoint = 0;
  bool triggerLow = false, triggerHigh = false;
  unsigned long int i = buffer.size - waveLen;
  while (i != 0 && i > (buffer.size - 2*waveLen)) {
    triggered = (triggerLow && triggerHigh);
    i--;
    if (buffer.data[chan][i] < trigger) {
      triggerLow = true;
      if (triggered && edge) break;
    }
    if (buffer.data[chan][i] > trigger) {
      triggerHigh = true;
      if (triggered && !edge) break;
    }
  }
  triggerPoint = i;

  float delta = 0;

  if (triggered) {
    delta = 2.0f/(float)(waveLen);
    for (;i < buffer.size; i++) {
      buffer.alignRamp[i-offset] = -1.0f + delta*(i - triggerPoint);
    }
  } else {
    delta = ((float)buffer.size/((float)waveLen))/(float)buffer.size;
    buffer.alignRamp[buffer.size-1] = 1.0f;
    for (i = buffer.size-2; i > 0; i--) {
      buffer.alignRamp[i] = clamp(buffer.alignRamp[i+1] - 2*delta);
    }
    buffer.alignRamp[0] = -1.0f;
  }

  i = 0;
  return buffer.alignRamp;
}

bool AudioInput::didTrigger() {
  return triggered;
}

AudioInput::~AudioInput() {
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
    delete[] buffer.alignRamp;
    buffer.alignRamp = NULL;
  }
}

float *AudioInput::getData(unsigned char chan) {
  return (buffer.data)[chan];
}

const PaDeviceInfo *AudioInput::getDeviceInfo() {
  return Pa_GetDeviceInfo(conf.device);
}