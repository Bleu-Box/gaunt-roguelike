#include "main.h"

// try to add owner to player's container, and then get rid of self
bool Pickable::pick(Actor *owner, Actor *wearer) {
       if(wearer->container && wearer->container->add(owner)) {
	       engine.actors.remove(owner);
	       return true;
       }
       return false;
}

// the only thing to do here (since it's the base class) is to delete owner upon use, which most items do
bool Pickable::use(Actor *owner, Actor *wearer) {
	if(wearer->container) {
		wearer->container->remove(owner);
		delete owner;
		return true;
	}
	return false;
}

// drop an item by removing it from wearer's inventory and putting it back into actor list
void Pickable::drop(Actor* owner, Actor* wearer) {
	if(wearer->container) {
		wearer->container->remove(owner);
		engine.actors.push(owner);
		owner->x = wearer->x;
		owner->y = wearer->y;
		engine.gui->message(Gui::ACTION, "%s drops the %s.", wearer->name, owner->name);
	}
}

Healer::Healer(float amt): amt(amt) {}

bool Healer::use(Actor *owner, Actor *wearer) {
	if(wearer->destructible) {
		float amountHealed = wearer->destructible->heal(amt);
		engine.gui->message(Gui::ACTION, "%s drinks a health potion, gaining %.1f health.", wearer->name, amountHealed);
		if(amountHealed > 0) return Pickable::use(owner, wearer);
	}
	return false;
}

StyxRifle::StyxRifle(float range, float damage, int ammo): range(range), damage(damage), ammo(ammo), maxAmmo(ammo) {}

bool StyxRifle::use(Actor* owner, Actor* wearer) {
	engine.gui->message(Gui::OBSERVE, "Left-click a space to shoot your %s at it.", owner->name);
	int x, y;
	// if engine.pickTile succeeds, it will set x and y to what the player selected
	if(!engine.pickTile(&x, &y, range)) return false;
	
	engine.gui->message(Gui::ATTACK, "You shoot the %s at your target.", owner->name);
	
	for(Actor** iterator = engine.actors.begin(); iterator != engine.actors.end(); iterator++) {
		Actor* actor = *iterator;
		if(actor != wearer && actor->destructible && !actor->destructible->isDead() && actor->getDistance(x, y) <= range) {
			engine.gui->message(Gui::ATTACK, "The %s gets hit and suffers %.1f damage.", actor->name, damage);
			actor->destructible->takeDamage(actor, damage);
		}
	}

	if(--ammo <= 0) return Pickable::use(owner, wearer);
	return true;
}

Crossbow::Crossbow(float range, float damage, int ammo): StyxRifle(range, damage, ammo) {}

bool Crossbow::use(Actor* owner, Actor* wearer) {
	return StyxRifle::use(owner, wearer);
}

