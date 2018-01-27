// implementation for Actor class
#include <algorithm>
#include <math.h>
#include "main.h"
#include "actor.h"
#include "effect.h"
#include "destructible.h"
#include "attacker.h"
#include "ai.h"
#include "pickable.h"
#include "container.h"
#include "spreadable.h"

Actor::Actor(int x, int y, int ch, std::string name, const TCODColor& color):
	x(x), y(y), ch(ch), blocks(true), color(color),
	attacker(NULL), destructible(NULL), ai(NULL), pickable(NULL), container(NULL), spreadable(NULL),
        name(name) {}

Actor::Actor(const Actor& other): x(other.x), y(other.y), ch(other.ch), blocks(other.blocks),
				  color(other.color), name(other.name) {
	if(other.attacker) {
		attacker = new Attacker(0, 0, "");
		*attacker = *other.attacker;
	}

	if(other.destructible) {
		destructible = new Destructible(0, 0, 0);
		*destructible = *other.destructible;
	}

	if(other.ai) {
		if(dynamic_cast<MonsterAi*>(other.ai) != nullptr) {
			ai = new MonsterAi(0, 0);
		} else {
			ai = new PlayerAi(0);
		}
		*ai = *other.ai;
	}

	if(other.pickable) {
		pickable = new Pickable();
		*pickable = *other.pickable;
	}
	
	if(other.spreadable) {
		spreadable = new Spreadable(0);
		*spreadable = *other.spreadable;
	}

	/*
	if(other.container) {
		container = new Container(0);
		*container = *other.container;
	}
	*/
}
  
Actor::~Actor() {
	effects.clear();
	if(attacker) delete attacker;
	if(destructible) delete destructible;
	if(ai) delete ai;
	if(pickable) delete pickable;
	if(container) delete container;
	if(spreadable) delete spreadable;
}

Actor& Actor::operator=(const Actor& rhs) {
	Actor temp(rhs);
	
	std::swap(x, temp.x);
	std::swap(y, temp.y);
	std::swap(ch, temp.ch);
	std::swap(blocks, temp.blocks);
	std::swap(color, temp.color);
	std::swap(name, temp.name);
	std::swap(attacker, temp.attacker);
	std::swap(destructible, temp.destructible);
	std::swap(ai, temp.ai);
	std::swap(pickable, temp.pickable);
	std::swap(container, temp.container);
	std::swap(spreadable, temp.spreadable);
	
	return *this;
}

void Actor::addEffect(Effect* effect) {
	// if we already have an effect, don't do anything -- otherwise it would cause
	// unreasonably long-lasting effects
	for(Effect* e : effects) {
	        if(e->getType() == effect->getType())
			return;
	}
	
	effect->begin(this);
	effects.push_back(effect);
}
 
void Actor::render() const {
	TCODConsole::root->putChar(x, y, ch);
	TCODConsole::root->setCharForeground(x, y, color);
}

void Actor::update() {
        if(ai) ai->update(this);

	std::vector<Effect*> toRemove;
	// update any magic effects on self
	for(Effect* effect : effects) {
		if(effect->getDuration() <= 0) {
			effect->end(this);
			toRemove.push_back(effect);
		}
	        else effect->update(this);
	}

	// remove finished effects
	for(Effect* e : toRemove) effects.erase(std::find(effects.begin(), effects.end(), e));
	
}

float Actor::getDistance(int cx, int cy) const {
	int dx = x-cx;
	int dy = y-cy;
	return sqrtf(dx*dx+dy*dy);
}
