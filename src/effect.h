// this is the class for potion/magic effects
#ifndef EFFECT_H
#define EFFECT_H

#include <string>

class Actor;

class Effect {
 public:
	enum EffectType {POISON, BLINDNESS, CONFUSION, PROTECTION, WASTING, HEALTH, STRENGTH, NUM_EFFECT_TYPES};
	
	Effect(EffectType type, int duration);

	static std::string effectTypeToString(EffectType t);
	std::string getName() const;
	void begin(Actor* target);
	void update(Actor* target);
	void end(Actor* target);
	EffectType getType() const { return type; }
	int getDuration() const { return duration; }
	const int getStartDuration() const { return startDuration; }

 private:
	EffectType type;
	int duration;
        const int startDuration;
	 // effects often need to save information about the target to go back to when they wear off
	Actor* savedState;
};

#endif
