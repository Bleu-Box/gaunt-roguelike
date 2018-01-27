// This contains code for the actor class definition
#ifndef ACTOR_H
#define ACTOR_H

#include <vector>
#include <string>
#include "lib/libtcod.hpp"

class Attacker;
class Destructible;
class Ai;
class Pickable;
class Container;
class Spreadable;
class Effect;

class Actor {	
 public:
	int x, y;
	int ch; // using an int rather than a char allows you to use more than 256 different chars
	bool blocks; // whether the actor can be walked upon
	TCODColor color;
	Attacker* attacker; // a thing that deals damage
	Destructible* destructible; // smth that can be hurt
	Ai* ai; // smth that's self-updating
	Pickable* pickable; // something that can be picked and used
	Container* container; // something that can contain other actors
	Spreadable* spreadable; // allows actor to spread children around itself (i.e. fire spreading)
	std::vector<Effect*> effects; // magic effects acting on actors
	std::string name;
	
	Actor(int x, int y, int ch, std::string name, const TCODColor& color);
	Actor(const Actor& other);
	~Actor();

	Actor& operator=(const Actor& rhs);
	void render() const;
	void update();
	bool moveOrAttack(int x, int y);
	float getDistance(int cx, int cy) const;
	void addEffect(Effect* effect);
};

#endif
