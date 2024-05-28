#include "audio/audio.h"
#include "gui/gui.h"
#include "../shared.h"

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
    audioFrameSize(512),
    channels(2),
    sampleRate(48000),
    audioDevice(0),
    bufferSize(1600),
    scale(2.0f),
    trigger(0.0f) {}
};

std::string getErrorMsg(int e) {
  std::string errm = "cant init audio!\n";
  switch (e) {
    case UAUDIOERR_NOERR:
      return "";
    case UAUDIOERR_NODEVS:
      return errm + errMsgs[0];
    case UAUDIOERR_NODEV:
      return errm + errMsgs[1];
    case UAUDIOERR_NOSTART:
      return errm + errMsgs[2];
    case UAUDIOERR_NOGOOD:
      return errm + errMsgs[3];
    default: 
      return errm + Pa_GetErrorText(e) + '\n';
  }
}

int main(int argc, char** argv) {
  unscopeParams params;
  parseParams(params, argc, argv);

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
  g.attachAudioInput(&i);

  params.audioDevice = Pa_GetDefaultInputDevice();

  g.getDevices(i.enumerateDevs());
  g.setAudioDeviceSetting(params.audioDevice);

  AudioProcess p(params.audioBufferSize,params.channels);
  g.attachAudioProcess(&p);

  while (g.isRunning()) {
    g.doFrame();
  }
  e = i.stop();
  printf("%s\n", Pa_GetErrorText(e));
  return e!=paNoError;
}

const char* verMsg = PROGRAM_NAME " (version " PROGRAM_VER ")\n";
const char* helpMsg =
"Program arguments\n"
"  -h: print this message\n"
"  -b: set the audio buffer size (default: 65536)\n"
"  -f: set the audio frame size (default: 128)\n"
"  -c: set the channel amount (default: 2)\n"
"  -s: set the sample rate (default: 48000)\n"
"  -v: print the program version\n";
const char* errMsgs[] = {
  "no devices found\n",
  "cant open device\n",
  "cant start device\n",
  "cant init portaudio\n"
};