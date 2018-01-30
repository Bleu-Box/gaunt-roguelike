#include "main.h"

Engine engine(100, 50);

int main() {
	engine.load();
	while(!TCODConsole::isWindowClosed()) {
		engine.update();
		engine.render();
		TCODConsole::flush();
	}
	
	return 0;
}
