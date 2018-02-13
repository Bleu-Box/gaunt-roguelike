#include <algorithm>
#include <vector>
#include <cassert>
#include <math.h>
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

//// POTION /////////////////////////////////////////

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
	if(!colorIsKnown(c)) {
		knownColors.push_back(c);
		for(Actor* item : engine.actors) {
			if(item->pickable) {
				Potion* potion = dynamic_cast<Potion*>(item->pickable);
				if(potion != nullptr) {
					item->name = potion->getName();
				}
			}
		}
	}
}

// see if player knows what effect a color gives
bool Potion::colorIsKnown(Color c) {
	return std::find(knownColors.begin(), knownColors.end(), c) != knownColors.end();
}

// return the potion's name based on its color/effect type
std::string Potion::getName() const {
	if(colorIsKnown(color)) {
		return "Potion of "+Effect::effectTypeToString(potionNames[color]);
	} else {
		switch(color) {
		case CYAN: return "Cyan potion";
		case VERMILLION: return "Vermillion potion";
		case AQUAMARINE: return "Aquamarine potion";
		case BLACK: return "Black potion";
		case MIDNIGHT_BLUE: return "Midnight blue potion";
		case CLEAR: return "Clear potion";
		case GREEN: return "Green potion";
		default: return "Strange potion";	
		}
	}
}

// drink a potion and learn what it does
void Potion::quaff(Actor* owner, Actor* wearer) {
	TCODRandom* rand = TCODRandom::getInstance();
	Effect* effect = new Effect(potionNames[color], rand->getInt(5, 50));
	
	wearer->addEffect(effect);	
	learnColor(color);
	use(owner, wearer);
}

// splash a space with the potion, and affect any actors at that location
void Potion::splash(Actor* owner, Actor* wearer, int x, int y) {
	TCODRandom* rand = TCODRandom::getInstance();
	Effect* effect = new Effect(potionNames[color], rand->getInt(5, 50));

	Actor* actor = engine.getActorAt(x, y);
			
	if(actor && actor->x == x && actor->y == y) {
		learnColor(color);
		actor->addEffect(effect);
		engine.gui->message("The "+getName()+" splashes all over the "+actor->name+"!");
	}

	use(owner, wearer);
}

// get rid of the potion upon use
void Potion::use(Actor* owner, Actor* wearer) {
	if(wearer->container) {
		wearer->container->remove(owner);
		delete owner;
	}
}

/////////// ARMOR ////////////////////
Armor::Armor(): equipped(false) {
	// based on a random number, set the defense and name of the armor
	TCODRandom* rand = TCODRandom::getInstance();
	int n = rand->getInt(0, 9);
	std::string armorType; // the main part of the name
		
	switch(n) {
	case 0: armorType = "leather"; break;
	case 1: armorType = "tin"; break;
	case 2: armorType = "bronze"; break;
	case 3: armorType = "chain mail"; break;
	case 4: armorType = "mithril"; break;
	case 5: armorType = "iron"; break;
	case 6: armorType = "steel"; break;
	case 7: armorType = "plate mail"; break;
	case 8: armorType = "corundum"; break;
	case 9: armorType = "diamond"; break;
	}

	weight = armorType == "mithril"? 2 : n*2;
	defense = (n+2)*5;
	// now compute the full name, which includes rounded defense/weight info
	std::string roundedWeight = std::to_string(round(weight*100)/100).substr(0, 4);
	std::string roundedDefense = std::to_string(round(defense*100)/100).substr(0, 4);
	name = "+"+roundedDefense+" "+armorType+" armor"+"<"+roundedWeight+">";
}

////////// WEAPON ///////////////////////

Weapon::Weapon(): equipped(false) {
	// naming and stuff works just like it does with armor
	TCODRandom* rand = TCODRandom::getInstance();
	int n = rand->getInt(0, 9);
	std::string weaponType;

	switch(n) {
	case 0: weaponType = "bokken"; break;
	case 1: weaponType = "dagger"; break;
	case 2: weaponType = "spear"; break;
	case 3: weaponType = "sword"; break;
	case 4: weaponType = "rapier"; break;
	case 5: weaponType = "katana"; break;
	case 6: weaponType = "war hammer"; break;
	case 7: weaponType = "battleaxe"; break;
	case 8: weaponType = "broadsword"; break;
	case 9: weaponType = "halberd"; break;
	}

	weight = n*2;
	power = (n+1)*2.2;
	// compute full name
	std::string roundedWeight = std::to_string(round(weight*100)/100).substr(0, 4);
	std::string roundedPower = std::to_string(round(power*100)/100).substr(0, 4);
	name = "+"+roundedPower+" "+weaponType+"<"+roundedWeight+">";
}
