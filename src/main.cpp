#include "audio/audio.h"
#include "gui/gui.h"
#include <iostream>

int main() {
	AudioInput i;
	GUI g;
	float data[4]={-1.0f,0.0f,1.0f,0.0f};
	g.init();
	i.~AudioInput();
	g.writeOscData(data,4);
	while (g.isRunning()) {
		g.doFrame();
	}
	return 0;
}