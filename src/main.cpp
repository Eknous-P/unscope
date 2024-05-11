#include "audio/audio.h"
#include "gui/gui.h"
#include <iostream>

int main() {
	AudioInput i;
	if (i.init() != 0) {
		std::cout << "cant init audio you fuk\n";
		return 1;
	}
	GUI g;
	g.init();
	while (g.isRunning()) {
		g.writeOscData(i.getData(),i.getDataSize());
		g.doFrame();
	}
	std::cout << i.stop();
	return 0;
}