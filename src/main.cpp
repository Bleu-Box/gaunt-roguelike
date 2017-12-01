// This is a test program for the Libtcod roguelike library
#include "main.h"

Engine engine(70, 60);

int main() {
	engine.load();
	while(!TCODConsole::isWindowClosed()) {
		engine.update();
		engine.render();
		TCODConsole::flush();
	}
	return 0;
}
