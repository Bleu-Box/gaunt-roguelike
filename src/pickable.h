// the class for things that can be collected
#ifndef PICKABLE_H
#define PICKABLE_H

class Pickable {
 public:
	virtual ~Pickable() {};

	bool pick(Actor* owner, Actor* wearer);
	virtual bool use(Actor* owner, Actor* wearer);
	void drop(Actor* owner, Actor* wearer);
};

class Healer: public Pickable {
 public:
	Healer(float amt);
	
	bool use(Actor* owner, Actor* wearer);

 private:
	int amt;
};

/*
class StyxRifle: public Pickable {
 public:
	float range, damage;
	float ammo;
	float maxAmmo;

	StyxRifle(float range, float damage, int ammo);

        bool use(Actor* owner, Actor* wearer);
};

class Crossbow: public StyxRifle {
 public:
        Crossbow(float range, float distance, int ammo);

	bool use(Actor* owner, Actor*  wearer);
};
*/

#endif
