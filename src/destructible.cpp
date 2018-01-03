// implementation for Destructibles
#include "main.h"

Destructible::Destructible(float maxHp, float defense, float regen, const TCODColor& corpseColor):
        defense(defense), maxHp(maxHp), hp(maxHp), regen(regen), corpseColor(corpseColor) {}

// take damage and die if needed
float Destructible::takeDamage(Actor* owner, float damage) {
	damage /= defense;

	hp -= damage;
	if(hp <= 0) die(owner); 
	
	return damage;
}

void Destructible::die(Actor* owner) {
	// TODO: destroy dead actors and just make the bloodstain a colored tile
	// make the owner into a bloodstain/corpse that can be walked upon
	owner->ch = '.';
	owner->color = corpseColor;
	owner->blocks = false;
	// this ensures the corpse isn't drawn on top of living things
        engine.sendToBack(owner);
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

MonsterDestructible::MonsterDestructible(float maxHp, float defense, float regen, const TCODColor& corpseColor):
	Destructible(maxHp, defense, regen, corpseColor) {}

void MonsterDestructible::die(Actor* owner) {
        engine.gui->message(Gui::ATTACK, "The " + owner->getName() + " died.");
	Destructible::die(owner);
}

PlayerDestructible::PlayerDestructible(float maxHp, float defense, float regen):
	Destructible(maxHp, defense, regen) {}

void PlayerDestructible::die(Actor* owner) {
        engine.gui->message(Gui::ATTACK, "You died!");
	Destructible::die(owner);
        engine.gameStatus = Engine::DEFEAT;
}
