// implementation for Actor class
#include "main.h"
#include <math.h>
 
Actor::Actor(int x, int y, int ch, const char* name, const TCODColor& color):
	x(x), y(y), ch(ch), name(name), blocks(true),
	attacker(NULL), destructible(NULL), ai(NULL), pickable(NULL), container(NULL), spreadable(NULL),
	color(color) {}

Actor::~Actor() {
	effects.clearAndDelete();
	if(attacker) delete attacker;
	if(destructible) delete destructible;
	if(ai) delete ai;
	if(pickable) delete pickable;
	if(container) delete container;
	if(spreadable) delete spreadable;
}

void Actor::addEffect(Effect* effect) {
	// don't add the effect if we already have it
	for(Effect** iterator = effects.begin(); iterator != effects.end(); iterator++) {
		Effect* e = *iterator;
	        if(e->getType() == effect->getType()) return;
	}
	
	effect->begin(this);
	effects.push(effect);
}

void Actor::render(int xshift, int yshift) const {
	TCODConsole::root->setChar(x+xshift, y+yshift, ch);
	TCODConsole::root->setCharForeground(x+xshift, y+yshift, color);
}

void Actor::update() {
        if(ai) ai->update(this);
	// update any magic effects on self
	for(Effect** iterator = effects.begin(); iterator != effects.end(); iterator++) {
		Effect* effect = *iterator;
		// end and remove effects that are over
		if(effect->getDuration() <= 0) {
			effect->end(this);
			delete effect;
			iterator = effects.remove(iterator);
		} else {
			effect->update(this);
		}
	}
}

float Actor::getDistance(int cx, int cy) const {
	int dx = x-cx;
	int dy = y-cy;
	return sqrtf(dx*dx+dy*dy);
}
