#include <cassert>
#include "main.h"
#include "attacker.h"
#include "actor.h"
#include "destructible.h"
#include "gui.h"
#include "map.h"

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
			float dmg = target->destructible->takeDamage(target, power);
		        if(enchanted) {
			        target->addEffect(new Effect(effectType, effectDuration));
			}
			
			if(dmg > 0.0) {
				engine.gui->message(Gui::ATTACK, owner->getName() + " " + action + " " + target->getName()
						    + ", inflicting " +
						    std::to_string(dmg)
						    + " damage.");
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
			        engine.gui->message(Gui::ATTACK, owner->getName() + " " + action +
						    " " + target->getName() + ", but it has no effect!");            
			}
		} else {
		        engine.gui->message(Gui::OBSERVE, owner->getName() + " misses " + target->getName() + ".");
		}		
	} 
}
