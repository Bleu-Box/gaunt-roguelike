// implementation for Actor class
#include "main.h"
#include <math.h>
 
Actor::Actor(int x, int y, int ch, const char* name,  const TCODColor& color):
	x(x), y(y), ch(ch), name(name), blocks(true), attacker(NULL), destructible(NULL), ai(NULL), pickable(NULL), container(NULL), color(color) {}

Actor::~Actor() {
	if(attacker) delete attacker;
	if(destructible) delete destructible;
	if(ai) delete ai;
	if(pickable) delete pickable;
	if(container) delete container;
}

void Actor::render(int xshift, int yshift) const {
	TCODConsole::root->setChar(x+xshift, y+yshift, ch);
	TCODConsole::root->setCharForeground(x+xshift, y+yshift, color);
}

void Actor::update() {
        if(ai) ai->update(this);
}

float Actor::getDistance(int cx, int cy) const {
	int dx = x-cx;
	int dy = y-cy;
	return sqrtf(dx*dx+dy*dy);
}
