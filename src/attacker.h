// the class for things that can attack
#ifndef ATTACKER_H
#define ATTACKER_H

#include <string>
#include "effect.h"

class Actor;
class Weapon;

class Attacker {
 public:	
	Attacker(float power, float accuracy, std::string action);
	Attacker(const Attacker& other);
	~Attacker();

	Attacker& operator=(const Attacker& rhs);
		
	void attack(Actor* owner, Actor* target);
	Effect::EffectType getEffectType() const { return effectType; }
	int getEffectDuration() const { return effectDuration; }
	bool isEnchanted() const { return enchanted; }
	void setEffect(Effect::EffectType type, int duration);
        void setAccuracy(float val) { if(val >= 0 && val <= 100) accuracy = val; }
	float getPower() const;
        float getAccuracy() const;
	std::string getAction() const { return action; }
	void equipWeapon(Weapon* _weapon);
	void unequipWeapon();

 private:
	std::string action; // the action the attacker does to its target
	float power;
	float accuracy;
	Effect::EffectType effectType; // for magical effects
	int effectDuration;
	bool enchanted; // flags whether or not attacker has effect
	Weapon* weapon;
};

#endif
