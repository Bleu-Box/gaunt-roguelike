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

	bool pick(Actor* owner, Actor* wearer);
	void drop(Actor* owner, Actor* wearer);
};

class Potion: public Pickable {
 public:
        enum Color {CYAN, VERMILLION, AQUAMARINE, BLACK, MIDNIGHT_BLUE, CLEAR, GREEN, NUM_COLORS};
	static std::map<Color, Effect::EffectType> potionNames;
	
	Potion();

	static void assignColors();
	static void learnColor(Color c);
	static bool colorIsKnown(Color c);
        void quaff(Actor* owner, Actor* wearer);
        void splash(Actor* owner, Actor* wearer, int x, int y);
	std::string getName() const;

 private:
	static std::vector<Color> knownColors;
        Color color;

	void use(Actor* owner, Actor* wearer);
};

class Armor: public Pickable {
 public:
	float defense;
	float weight;
	bool equipped;
	
	Armor();

	std::string getName() const { return name+(equipped? " (equipped)" : ""); }

 private:
	std::string name;
};

class Weapon: public Pickable {
 public:
	float power;
	float weight;
	bool equipped;

	Weapon();

	std::string getName() const { return name+(equipped? " (equipped)" : ""); }
 private:
	std::string name;
};

#endif
