#include "audio/audio.h"
#include "gui/gui.h"
#include <iostream>

int main() {
	AudioInput i;
	GUI g;
	g.init();
	i.~AudioInput();
	while (g.isRunning()) {
		g.writeOscData(i.getData(),2048);
		g.doFrame();
	}
	std::cout << i.getData();
	return 0;
}