// This is a test program for the Libtcod roguelike library
#include "main.h"

Engine engine(130, 50);

int main() {
	engine.load();
	while(!TCODConsole::isWindowClosed()) {
		engine.update();
		engine.render();
		TCODConsole::flush();
	}
	
	return 0;
}
