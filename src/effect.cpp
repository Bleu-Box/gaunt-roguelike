#include "main.h"
#include "effect.h"
#include "actor.h"
#include "ai.h"
#include "destructible.h"

Effect::Effect(EffectType type, int duration): type(type), duration(duration), startDuration(duration) {
	// make a dummy saved state
	savedState = new Actor(0, 0, 'D', "dummy actor", TCODColor::lightestRed);
}

// some things only need to be done once on the target - these go in here
void Effect::begin(Actor* target) {
	if(type == BLINDNESS && target == engine.player) {
	        engine.renderMap = false; // don't render map
	} else if(type == CONFUSION) {
		if(target->ai)
			target->ai->confused = true;
	} else if(type == PROTECTION) {
	        *savedState = *target;
		if(target->destructible) target->destructible->invincible = true;
	} else if(type == WASTING) {
		*savedState = *target;
		if(target->destructible) target->destructible->setRegen(0);
	} 
}

void Effect::update(Actor* target) {
	if(engine.gameStatus != Engine::IDLE && duration > 0) {
		if(type == POISON) {
			if(target->destructible) {
				// poison damages actors proportional to their health and defense
				target->destructible->takeDamage(target, target->destructible->getMaxHp()*0.01
								 +target->destructible->getDefense());
			}
		} else if(type == HEALTH) {
			if(target->destructible)
				target->destructible->heal(target->destructible->getMaxHp()/duration);
		} else if(type == STRENGTH) {
		        target->stren += 0.25;
		}
		
		duration--;
	}
}
 
// return target back to normal state
void Effect::end(Actor* target) {
        if(type == BLINDNESS && target == engine.player) {
	        engine.renderMap = true; // set mode to render full map again
	} else if(type == CONFUSION) {
		target->ai->confused = false;
	} else if(type == PROTECTION) {
		if(target->destructible)
			target->destructible->invincible = savedState->destructible->invincible;
	} else if(type == WASTING) {
		if(target->destructible)
			target->destructible->setRegen(savedState->destructible->getRegen());
	}
}

// turns an effect type into a string
std::string Effect::effectTypeToString(EffectType t) {
	return Effect(t, 0).getName();
}

// returns string version of the effect's name for printing purposes (i.e. in GUI)
std::string Effect::getName() const {
        switch(type) {
	case POISON: return "Poison";
	case BLINDNESS: return "Blindness";
	case CONFUSION: return "Confusion";
	case PROTECTION: return "Protection";
	case WASTING: return "Wasting";
	case HEALTH: return "Health";
	case STRENGTH: return "Strength";
	default: return "Effect";
	}
}
