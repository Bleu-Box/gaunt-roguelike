// this is the class for things that can be destroyed/hurt
#ifndef DESTRUCTIBLE_H
#define DESTRUCTIBLE_H

#include "lib/libtcod.hpp"

class Actor;
class Armor;

class Destructible {
 public:
	bool invincible;
	
	Destructible(float maxHp, float defense, float regen, const TCODColor& corpseColor=TCODColor::darkRed);
	virtual ~Destructible() {}
	
        bool isDead() { return hp <= 0; } 
	float takeDamage(Actor* owner, float damage);
	float heal(float amt);
	virtual void die(Actor* owner); // death is handled by subclasses - this allows death to be handled differently
	int getMaxHp() const { return maxHp; }
	int getHp() const { return hp; }
	void regenerate();
        float getDefense() const { return defense+armorDefense; }
        void setDefense(float val) { if(val >= 0) defense = val; }
	float getRegen() const { return regen; }
	void setRegen(float val) { if(val >= 0) regen = val; }
	TCODColor getCorpseColor() const { return corpseColor; }
	// getting/setting armor is done with more fitting names
	void equipArmor(Armor* armor);
	void unequipArmor() { armorDefense = 0; }

 protected:
	float defense; // `defense' is the natural defense level,  
	float armorDefense; // and `armorDefense' is extra defense granted by armor
	float maxHp;
	float hp;
	float regen;
	TCODColor corpseColor;
};

// these are subclasses that are specialized in how they die
class MonsterDestructible: public Destructible {
 public:
	MonsterDestructible(float maxHp, float defense, float regen,
			    const TCODColor& corpseColor=TCODColor::darkRed);
		
	void die(Actor* owner);
};

class PlayerDestructible: public Destructible {
 public:
        PlayerDestructible(float maxHp, float defense, float regen);
	
	void die(Actor* owner);
};

#endif
