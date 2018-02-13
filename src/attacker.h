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
	float getPower() const;
        float getAccuracy(Actor* owner) const;
	std::string getAction() const { return action; }
	void equipWeapon(Weapon* _weapon);
	void unequipWeapon();

 protected:
	float getNatAccuracy() const { return natAccuracy; }
	
 private:
	std::string action; // the action the attacker does to its target
	float power;
	float natAccuracy;
	Effect::EffectType effectType; // for magical effects
	int effectDuration;
	bool enchanted; // flags whether or not attacker has effect
	Weapon* weapon;
};

#endif
