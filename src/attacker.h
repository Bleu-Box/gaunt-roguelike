//  the class for things that can attack
#ifndef ATTACKER_H
#define ATTACKER_H

#include <string>
#include "effect.h"

class Actor;

class Attacker {
 public:	
	Attacker(float power, float accuracy, std::string action);
	Attacker(const Attacker& other);

	Attacker& operator=(const Attacker& rhs);
		
	void attack(Actor* owner, Actor* target);
	Effect::EffectType getEffectType() const { return effectType; }
	int getEffectDuration() const { return effectDuration; }
	bool isEnchanted() const { return enchanted; }
	void setEffect(Effect::EffectType type, int duration);
	float getPower() const { return power; }
        void setPower(float val) { if(val >= 0) power = val; }
        float getAccuracy() const { return accuracy; }
        void setAccuracy(float val) { if(val >= 0 && val <= 100) accuracy = val; }
	std::string getAction() const { return action; }

 private:
	std::string action; // the action the attacker does to its target
	float power;
	float accuracy;
	Effect::EffectType effectType; // for magical effects
	int effectDuration;
	bool enchanted; // flags whether or not attacker has effect
};

#endif
