#include <stdexcept>
#include "main.h"

Attacker::Attacker(float power, int accuracy, const char* action): power(power), accuracy(accuracy), action(action), enchanted(false) {
	// accuracy must be in range [0..100]
	if(accuracy < 0 || accuracy > 100) {
		throw std::invalid_argument("Last argument must be within range 0..100.");
	}
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
			
			if(power-target->destructible->defense > 0) {
				engine.gui->message(Gui::ATTACK, "%s %s %s, inflicting %g damage.",
						   owner->name, action, target->name, power-target->destructible->defense);
			} else { 
				engine.gui->message(Gui::ATTACK, "%s %s %s, but it has no effect!",
						    owner->name, action, target->name);            
			}
		} else {
			engine.gui->message(Gui::OBSERVE, "%s misses %s.", owner->name, target->name);
		}
		
		target->destructible->takeDamage(target, power);		
	} else {
		// if the owner is trying to attack something that can't be harmed
		engine.gui->message(Gui::OBSERVE, "%s %s %s.", owner->name, action, target->name);
	}
}
