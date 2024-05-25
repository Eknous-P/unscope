#include "audio/audio.h"
#include "gui/gui.h"
#include "../shared.h"
#include <string>

struct unscopeParams {
  unsigned int audioBufferSize;
  unsigned int audioFrameSize;
  unsigned char channels;
  unsigned int sampleRate;
  int audioDevice;

  // gui ...
  unsigned int bufferSize;
  float scale;
  float trigger;
  unscopeParams():
    audioBufferSize(65536),
    audioFrameSize(128),
    channels(2),
    sampleRate(48000),
    audioDevice(0),
    bufferSize(1600),
    scale(2.0f),
    trigger(0.0f) {}
};

const char* errMsgs[] = {
  "no devices found\n",
  "cant open device\n",
  "cant start device\n",
  "cant init portaudio\n"
};

std::string getErrorMsg(int e) {
  std::string errm = "cant init audio!\n";
  switch (e) {
    case NOERR:
      return "";
    case NODEVS:
      return errm + errMsgs[0];
    case NODEV:
      return errm + errMsgs[1];
    case NOSTART:
      return errm + errMsgs[2];
    case NOGOOD:
      return errm + errMsgs[3];
    default: 
      return errm + Pa_GetErrorText(e) + '\n';
  }
}

int main() {
  unscopeParams params;

  GUI g(params.audioBufferSize,
        params.channels,
        params.bufferSize,
        params.scale,
        params.trigger);

  g.init();

  AudioInput i(params.audioFrameSize,
               params.audioBufferSize,
               params.channels,
               params.sampleRate);

  int e;

  params.audioDevice = Pa_GetDefaultInputDevice();
  g.setAudioDeviceSetting(params.audioDevice);

  g.getDevices(i.enumerateDevs());

  AudioProcess p(params.audioBufferSize,params.channels);

  while (g.isRunning()) {
    if (g.doRestartAudio()) {
      e = i.stop();
      params.audioDevice = g.getAudioDeviceSetting();
      printf("opening device %d...\n",params.audioDevice);
      e = i.init(params.audioDevice);
      if (e != paNoError) {
        printf("%d:%s", e, getErrorMsg(e).c_str());
        // try again
        printf("trying default device...\n");
        params.audioDevice = Pa_GetDefaultInputDevice();
        e = i.init(params.audioDevice);
        if (e != paNoError) {
          printf("%d:%s", e, getErrorMsg(e).c_str());
          return e;
        }
      }
      g.setAudioDeviceSetting(params.audioDevice);
      g.audioSet();
    }
    for (unsigned char j = 0; j < params.channels; j++) {
      p.writeDataIn(i.getData(j),j);
      g.writeOscData(j,
        p.alignWave(j,g.getTrigger(),g.getTraceSize(),0,0),
        i.getData(j));
    }
    g.doFrame();
    }
  e = i.stop();
  printf("%s\n", Pa_GetErrorText(e));
  return e!=paNoError;
}