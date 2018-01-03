#include <string>
// this is the class for potion/magic effects

class Effect {
 public:
	enum EffectType {POISON, BLINDNESS, CONFUSION};
	
	Effect(EffectType type, int duration);

	void begin(Actor* target);
	void update(Actor* target);
	void end(Actor* target);
	EffectType getType() const { return type; }
	int getDuration() const { return duration; }
	const int getStartDuration() const { return startDuration; }
	std::string getName() const;

 private:
	EffectType type;
	int duration;
	const int startDuration;
	 // effects often need to save information about the target to go back to when they wear off
	Actor* savedState;
};
