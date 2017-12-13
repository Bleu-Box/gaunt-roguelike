// this is the class for potion/magic effects

class Effect {
 public:
	enum EffectType {POISON, BLINDNESS};
	
	Effect(EffectType type, int duration);

	void begin(Actor* target);
	void update(Actor* target);
	void end(Actor* target);
	EffectType getType() const { return type; }
	int getDuration() const { return duration; }

 private:
	EffectType type;
	int duration;
	 // effects often need to save information about the target to go back to when they wear off
	Actor* savedState;
};
