class Ai {
 public:
	virtual ~Ai() {};
	
	virtual void update(Actor* owner) = 0; // this is an abstract method meant to be implemented only by subclasses
};

class PlayerAi: public Ai {
 public:
	void update(Actor* owner);
	
 protected:
	bool moveOrAttack(Actor* owner, int targetx, int targety);
	void handleActionKey(Actor* owner, int ascii);
	Actor* getFromInventory(Actor* owner);
};

class MonsterAi: public Ai {
 public:
	void update(Actor* owner);
	
 protected:
	int moveCount; // how long it takes to stop chasing target
	
	void moveOrAttack(Actor* owner, int targetx, int targety); 
};
