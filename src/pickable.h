// the class for things that can be collected
#ifndef PICKABLE_H
#define PICKABLE_H

#include <map>
#include <vector>
#include <string>
#include "effect.h"

class Actor;

class Pickable {
 public:
	virtual ~Pickable() {};

	virtual bool use(Actor* owner, Actor* wearer);
	bool pick(Actor* owner, Actor* wearer);
	void drop(Actor* owner, Actor* wearer);
};

class Potion: public Pickable {
 public:
        enum Color {CYAN, VERMILLION, AQUAMARINE, BLACK, MIDNIGHT_BLUE, COPPER, NUM_COLORS};
	static std::map<Color, Effect::EffectType> potionNames;
	
	Potion();

	static void assignColors();
	static void learnColor(Color c);
	static bool colorIsKnown(Color c);
	void update(Actor* owner, Actor* wearer);
	bool use(Actor* owner, Actor* wearer);
	std::string getName();

 private:
	static std::vector<Color> knownColors;
        Color color;
	int amt;
};

#endif
