// this is the class for things that can be destroyed/hurt

class Destructible {
 public:
	float maxHp; // maximum health
        float hp; // current health
        float defense; // hit points deflected
	const char* corpseName;
	TCODColor corpseColor;

	Destructible(float maxHp, float defense, const char* corpseName, const TCODColor& corpseColor=TCODColor::darkRed);
	virtual ~Destructible();
	
        bool isDead() { return hp <= 0; } 
	float takeDamage(Actor* owner, float damage);
	float heal(float amt);
	virtual void die(Actor* owner); // death is handled by subclasses - this allows death to be handled differently
};

// these are subclasses that are specialized in how they die
class MonsterDestructible: public Destructible {
 public:
	MonsterDestructible(float maxHp, float defense, const char* corpseName,
			    const TCODColor& corpseColor=TCODColor::darkRed);
		
	void die(Actor* owner);
};

class PlayerDestructible: public Destructible {
 public:
        PlayerDestructible(float maxHp, float defense, const char* corpseName);
	void die(Actor* owner);
};
