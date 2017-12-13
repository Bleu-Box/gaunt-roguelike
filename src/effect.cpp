#include "main.h"

Effect::Effect(EffectType type, int duration): type(type), duration(duration) {}

// some things only need to be done once on the target - these go in here
void Effect::begin(Actor* target) {
	if(type == POISON) {
		engine.gui->message(Gui::ACTION, "%s has been poisoned!", target->name);
	} else if(type == BLINDNESS) {
		engine.gui->message(Gui::ACTION, "%s can no longer see!", target->name);
		engine.setRenderMap(false); // don't render map except for player
	}
}

void Effect::update(Actor* target) {
	if(engine.gameStatus != Engine::IDLE && duration > 0) {
		if(type == POISON) {
			if(target->destructible) target->destructible->takeDamage(target, 2.5);
		}
		
		duration--;
	}
}

// return target back to normal state
void Effect::end(Actor* target) {
	if(type == POISON) {
		engine.gui->message(Gui::ACTION, "The poison acting on %s wears off.", target->name);
	} else if(type == BLINDNESS) {
		engine.gui->message(Gui::ACTION, "%s\'s sight has returned.", target->name);
		engine.setRenderMap(true); // set mode to render full map again
	}
}
