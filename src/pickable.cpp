#include <algorithm>
#include <vector>
#include <cassert>
#include "lib/libtcod.hpp"
#include "main.h"
#include "pickable.h"
#include "actor.h"
#include "container.h"
#include "gui.h"
#include "destructible.h"

std::map<Potion::Color, Effect::EffectType> Potion::potionNames;
std::vector<Potion::Color> Potion::knownColors;

// try to add owner to player's container, and then get rid of self
bool Pickable::pick(Actor* owner, Actor* wearer) {
       if(wearer->container && wearer->container->add(owner)) {
	       auto i = std::find(engine.actors.begin(), engine.actors.end(), owner);
	       engine.actors.erase(i);
	       return true;
       }
       return false;
}

// the only thing to do here (since it's the base class) is to delete owner upon use, which most items do
bool Pickable::use(Actor* owner, Actor* wearer) {
	if(wearer->container) {
		wearer->container->remove(owner);
		delete owner;
		return true;
	}
	return false;
}

// drop an item by removing it from wearer's inventory and putting it back into actor list
void Pickable::drop(Actor* owner, Actor* wearer) {
	if(wearer->container) {
		wearer->container->remove(owner);
	        engine.actors.push_back(owner);
		owner->x = wearer->x;
		owner->y = wearer->y;
	        engine.gui->message(wearer->name + " drops the " + owner->name + ".");
	}
}

Potion::Potion() {
	TCODRandom* rand = TCODRandom::getInstance();
	color = (Color) (rand->getInt(0, ((int)NUM_COLORS)-1));
}

// assigns colors to effects
void Potion::assignColors() {
	// make sure that there are as many potion colors as effects
	assert((int)Potion::Color::NUM_COLORS == (int)Effect::EffectType::NUM_EFFECT_TYPES);
	TCODRandom* rand = TCODRandom::getInstance();
        std::vector<Effect::EffectType> effects;
	
	for(int i = 0; i < Effect::EffectType::NUM_EFFECT_TYPES; i++) {
		effects.push_back((Effect::EffectType) i);
	}
	
        for(int i = 0; i < NUM_COLORS; i++) {
		Color c = (Color) i;
		Effect::EffectType type = effects[rand->getInt(0, effects.size()-1)];
		effects.erase(std::remove(effects.begin(), effects.end(), type), effects.end());
	        potionNames.insert(std::pair<Color, Effect::EffectType>(c, type));
	}
}

// learn that a certain color means a certain effect
void Potion::learnColor(Color c) {
	if(!colorIsKnown(c)) knownColors.push_back(c);
	for(Actor* item : engine.actors) {
		if(item->pickable) {
			Potion* potion = dynamic_cast<Potion*>(item->pickable);
			if(potion != nullptr) {
				item->name = potion->getName();
			}
		}
	}
}

// see if player knows what effect a color gives
bool Potion::colorIsKnown(Color c) {
	return std::find(knownColors.begin(), knownColors.end(), c) != knownColors.end();
}

// return the potion's name based on its color/effect type
std::string Potion::getName() {
	if(colorIsKnown(color)) {
		return Effect::effectTypeToString(potionNames[color])+" potion";
	} else {
		switch(color) {
		case CYAN: return "Cyan potion";
		case VERMILLION: return "Vermillion potion";
		case AQUAMARINE: return "Aquamarine potion";
		case BLACK: return "Black potion";
		case MIDNIGHT_BLUE: return "Midnight blue potion";
		case COPPER: return "Copper potion";
		default: return "Colorless potion";	
		}
	}
}

// drink a potion and learn what it does
bool Potion::use(Actor *owner, Actor *wearer) {
	TCODRandom* rand = TCODRandom::getInstance();
	Effect* effect = new Effect(potionNames[color], rand->getInt(5, 50));
	wearer->addEffect(effect);
	learnColor(color);
	
	return Pickable::use(owner, wearer);
}

