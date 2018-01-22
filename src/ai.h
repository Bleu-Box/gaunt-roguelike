#ifndef AI_H
#define AI_H

#include <functional>

class Actor;

class Ai {
 public:
	bool confused;
	
	Ai(int speed);	
	virtual ~Ai() {};
	
	virtual void update(Actor* owner) = 0; // this is an abstract method meant to be implemented only by subclasses

 protected:
	int speed;
};

class PlayerAi: public Ai {
 public:
	int stealth;
	
	PlayerAi(int stealth);
	
	void update(Actor* owner);
	
 protected:
	bool moveOrAttack(Actor* owner, int targetx, int targety);
	void handleActionKey(Actor* owner, int ascii);
	Actor* getFromInventory(Actor* owner);
};

class MonsterAi: public Ai {
 public:
	int range;
	
	MonsterAi(int speed, int range,
		  std::function<void(MonsterAi*, Actor*)> behavior =
		  [](MonsterAi* ai, Actor* owner) {
			  ai->pursuePlayer(owner);
		  });
	
	void update(Actor* owner);
	void pursuePlayer(Actor* owner);
	void spread(Actor* owner);
	std::function<void(MonsterAi*, Actor*)> executeBehavior;
	
 protected:
	void moveOrAttack(Actor* owner, int targetx, int targety);
};

#endif
