// implementation for Destructibles
#include "main.h"
#include "destructible.h"
#include "pickable.h"
#include "actor.h"
#include "gui.h"
#include "map.h"

Destructible::Destructible(float maxHp, float defense, float regen, const TCODColor& corpseColor):
        invincible(false), defense(defense), armorDefense(0), maxHp(maxHp), hp(maxHp), regen(regen), corpseColor(corpseColor) {}

// take damage and die if needed
float Destructible::takeDamage(Actor* owner, float damage) {
	// the total defense is based on defense and armor
	float totalDefense = getDefense();
	
	if(invincible) return 0.0;
	if(totalDefense != 0) damage /= totalDefense;

	hp -= damage;
	if(hp <= 0) die(owner); 
	
	return damage;
}

void Destructible::die(Actor* owner) {
	// put a corpse on the map (the engine will clean up dead actors)
	engine.map->addBloodstain(owner->x, owner->y, corpseColor);
}

float Destructible::heal(float amt) {
	hp += amt;
	
	if(hp > maxHp) {
		amt -= hp-maxHp;
		hp = maxHp;
	}
	
	return amt;
}

void Destructible::regenerate() {
	heal(regen);
}

void Destructible::equipArmor(Armor* armor) {
	armorDefense = armor->defense;
}

MonsterDestructible::MonsterDestructible(float maxHp, float defense, float regen, const TCODColor& corpseColor):
	Destructible(maxHp, defense, regen, corpseColor) {}

void MonsterDestructible::die(Actor* owner) {
        engine.gui->message("The " + owner->name + " dies.");
	Destructible::die(owner);
}

PlayerDestructible::PlayerDestructible(float maxHp, float defense, float regen):
	Destructible(maxHp, defense, regen) {}

void PlayerDestructible::die(Actor* owner) {
        engine.gui->message("You died...");
	Destructible::die(owner);
        engine.gameStatus = Engine::DEFEAT;
}
