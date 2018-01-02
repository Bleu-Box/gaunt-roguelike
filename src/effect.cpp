#include "main.h"

Effect::Effect(EffectType type, int duration): type(type), duration(duration) {}

// some things only need to be done once on the target - these go in here
void Effect::begin(Actor* target) {
	if(type == POISON) {
	        engine.gui->message(Gui::ACTION, target->getName() + " has been poisoned!");
	} else if(type == BLINDNESS) {
	        engine.gui->message(Gui::ACTION, target->getName() + " can no longer see!");
	        engine.renderMap = false; // don't render map except for player
	} else if(type == CONFUSION) {
		if(target->ai) {
		        engine.gui->message(Gui::ACTION, target->getName() + " is confused!");
			target->ai->confused = true;
		}
	}
}

void Effect::update(Actor* target) {
	if(engine.gameStatus != Engine::IDLE && duration > 0) {
		if(type == POISON) {
			if(target->destructible) {
				// poison damages actors proportional to their health and defense
				target->destructible->takeDamage(target,
								 target->destructible->getMaxHp()*0.01+target->destructible->defense);
			}
		}
		
		duration--;
	}
}
 
// return target back to normal state
void Effect::end(Actor* target) {
	if(type == POISON) {
	        engine.gui->message(Gui::ACTION, "The poison acting on " + target->getName() + " wears off.");
	} else if(type == BLINDNESS) {
	        engine.gui->message(Gui::ACTION, target->getName() + "\'s sight has returned.");
	        engine.renderMap = true; // set mode to render full map again
	} else if(type == CONFUSION) {
	        engine.gui->message(Gui::ACTION, target->getName() + " is no longer confused.");
		target->ai->confused = false;
	}
}
