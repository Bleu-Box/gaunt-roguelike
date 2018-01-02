#include <functional>

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
	PlayerAi();
	
	void update(Actor* owner);
	
 protected:
	bool moveOrAttack(Actor* owner, int targetx, int targety);
	void handleActionKey(Actor* owner, int ascii);
	Actor* getFromInventory(Actor* owner);
};

class MonsterAi: public Ai {
 public:	
	MonsterAi(int speed);
	
	void update(Actor* owner);
	std::function<bool(const Actor&)> spreadPredicate; // this decides whether or not owner will spread	

 protected: 
	int moveCount; // how long it takes to stop chasing target
	
	void moveOrAttack(Actor* owner, int targetx, int targety);
};
