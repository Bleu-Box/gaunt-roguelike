//  the class for things that can attack
class Attacker {
 public:
	float power;
	int accuracy; // must be in range [0..100]
	const char* action; // the action the attacker does to its target
	
	Attacker(float power, int accuracy, const char* action);
		
	void attack(Actor* owner, Actor* target);
	Effect::EffectType getEffectType() const { return effectType; }
	int getEffectDuration() const { return effectDuration; }
	bool isEnchanted() const { return enchanted; }
	void setEffect(Effect::EffectType type, int duration);

 private:
	Effect::EffectType effectType; // for magical effects
	int effectDuration;
	bool enchanted; // flags whether or not attacker has effect
};
