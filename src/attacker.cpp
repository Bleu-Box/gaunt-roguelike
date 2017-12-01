#include "main.h"

Attacker::Attacker(float power): power(power) {}

void Attacker::attack(Actor* owner, Actor* target) {
	if (target->destructible && !target->destructible->isDead()) {
		// if the target can be hurt and is alive:
		if (power-target->destructible->defense > 0) {
			engine.gui->message(Gui::ATTACK, "%s attacks %s, inflicting \n %g damage.", owner->name, target->name, power-target->destructible->defense);
		} else { 
			engine.gui->message(Gui::ATTACK, "%s attacks %s, but it has \n no effect!", owner->name, target->name);            
		}
		
		target->destructible->takeDamage(target, power);		
	} else {
		// if the owner is trying to attack something that can't be harmed
		engine.gui->message(Gui::OBSERVE, "%s hits %s.", owner->name,target->name);
	}
}
