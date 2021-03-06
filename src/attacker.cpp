#include <cassert>
#include <algorithm>
#include "main.h"
#include "attacker.h"
#include "pickable.h"
#include "actor.h"
#include "destructible.h"
#include "gui.h"
#include "map.h"

Attacker::Attacker(float power, float accuracy, std::string action): action(action), power(power),
								     natAccuracy(accuracy),
								     enchanted(false),
								     weapon(NULL) {
	// accuracy must be in range [0..100]
	assert(accuracy >= 0 && accuracy <= 100);
}

Attacker::Attacker(const Attacker& other): Attacker(other.getPower(), other.getNatAccuracy(), other.getAction()) {
	if(other.isEnchanted()) {
	        setEffect(other.getEffectType(), other.getEffectDuration());
	}
}

Attacker::~Attacker() {
	delete weapon;
}

Attacker& Attacker::operator=(const Attacker& rhs) {
	Attacker temp = Attacker(rhs);
	
	float _power = temp.getPower();
	float _acc = temp.getNatAccuracy();
	std::string _action = temp.getAction();
	bool _isEnchanted = temp.isEnchanted();
	Effect::EffectType _effectType = temp.getEffectType();
	int _effectDuration = temp.getEffectDuration();

	std::swap(power, _power);
	std::swap(natAccuracy, _acc);
	std::swap(action, _action);
	std::swap(enchanted, _isEnchanted);
	std::swap(effectType, _effectType);
	std::swap(effectDuration, _effectDuration);

	return *this;
}

void Attacker::setEffect(Effect::EffectType type, int duration) {
	effectType = type;
	effectDuration = duration;
	enchanted = true;
}

void Attacker::attack(Actor* owner, Actor* target) {
	TCODRandom* rand = TCODRandom::getInstance();
	
	if(target->destructible && !target->destructible->isDead()) {
		if(rand->getInt(0, 100) <= getAccuracy(owner)) {
			float dmg = target->destructible->takeDamage(target, getPower());
			
		        if(enchanted) {
			        target->addEffect(new Effect(effectType, effectDuration));
			}

			// if we've succeeded in killing the other, don't print attack messages
			if(target->destructible->isDead()) return;
			
			if(dmg > 0.0) {
				engine.gui->message(owner->name + " " + action + " " + target->name + ".");
				
				// put blood splatters everywhere
				for(int i = -1; i < 2; i++) {
					for(int j = 0; j < 2; j++) {
						if(rand->getInt(0, 100) < 40) {
							engine.map->addBloodstain(target->x+i, target->y+j,
										  target->destructible->getCorpseColor()*0.5);
						}
					}
				}
			} else {
			        engine.gui->message(owner->name + " " + action +
						    " " + target->name + ", but it does no damage!");            
			}
		} else {
		        engine.gui->message(owner->name + " misses " + target->name + ".");
		}
	} 
}

void Attacker::equipWeapon(Weapon* _weapon) {
	weapon = _weapon;
}

void Attacker::unequipWeapon() {
	weapon = NULL;
}

float Attacker::getPower() const {
	return power+(weapon? weapon->power : 0);
}

float Attacker::getAccuracy(Actor* owner) const {
        float acc = natAccuracy/(weapon? weapon->weight/owner->stren : 1);
	return acc < 100? acc : 100;
}
