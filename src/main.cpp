/*
  Gaunt is a roguelike game I'm making for fun.
  Hopefully I can keep adding new features in the future
  to make it (even) more fun.
*/

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
