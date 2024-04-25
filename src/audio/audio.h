#include "portaudio.h"

class AudioInput {
	private:

		struct AudioConfig{
			int sampleRate;
			int frameSize;
			unsigned char channels;
		};

		struct audioBuffer {
			float *data;
			unsigned int index;
		};

		AudioConfig conf;
		audioBuffer buffer;

		PaStream *stream;
		PaError err;
		PaStreamParameters streamParams;

	public:

		static int bufferWriteCallback(void* inputBuffer,
																	 void* outputBuffer,
																	 unsigned long framesPerBuffer,
        													 const PaStreamCallbackTimeInfo* timeInfo,
        													 PaStreamCallbackFlags statusFlags,
        													 void *userData );

		AudioInput();
		~AudioInput();
};