#include <cassert>
#include "main.h"

Attacker::Attacker(float power, float accuracy, std::string action): power(power), accuracy(accuracy), action(action),
								     enchanted(false) {
	// accuracy must be in range [0..100]
	assert(accuracy >= 0 && accuracy <= 100);
}

void Attacker::setEffect(Effect::EffectType type, int duration) {
	effectType = type;
	effectDuration = duration;
	enchanted = true;
}

void Attacker::attack(Actor* owner, Actor* target) {
	TCODRandom* rand = TCODRandom::getInstance();
	
	if(target->destructible && !target->destructible->isDead()) {
		if(rand->getInt(0, 100) <= accuracy) {
		        if(enchanted) {
			        target->addEffect(new Effect(effectType, effectDuration));
			}
			
			if(power-target->destructible->getDefense() > 0) {
				engine.gui->message(Gui::ATTACK, owner->getName() + " " + action + " " + target->getName()
						    + ", inflicting " +
						    std::to_string((int) (power-target->destructible->getDefense()))
						    + " damage.");
			} else { 
			        engine.gui->message(Gui::ATTACK, owner->getName() + " " + action +
						    " " + target->getName() + ", but it has no effect!");            
			}
		} else {
		        engine.gui->message(Gui::OBSERVE, owner->getName() + " misses " + target->getName() + ".");
		}
		
		target->destructible->takeDamage(target, power);		
	} 
}
