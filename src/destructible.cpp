// implementation for Destructibles
#include "main.h"

Destructible::Destructible(float maxHp, float defense, const char* corpseName):
	maxHp(maxHp), hp(maxHp), defense(defense) {
	this->corpseName = _strdup(corpseName);
}

Destructible::~Destructible() {
	free((char*) corpseName);
}

// take damage and die if needed
float Destructible::takeDamage(Actor* owner, float damage) {
	damage -= defense;

	if(damage > 0) {
		hp -= damage;
		if(hp <= 0) die(owner);
	} else {
		damage = 0;
	}

	return damage;
}

void Destructible::die(Actor* owner) {
	// make the owner into a bloodstain/corpse that can be walked upon
	owner->ch = '*';
	owner->color = TCODColor::darkRed;
	owner->name = corpseName;
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

MonsterDestructible::MonsterDestructible(float maxHp, float defense, const char* corpseName):
	Destructible(maxHp, defense, corpseName) {}

void MonsterDestructible::die(Actor* owner) {
	engine.gui->message(Gui::ATTACK, "The %s died.", owner->name);
	Destructible::die(owner);
}

PlayerDestructible::PlayerDestructible(float maxHp, float defense, const char* corpseName):
	Destructible(maxHp, defense, corpseName) {}

void PlayerDestructible::die(Actor* owner) {
	engine.gui->message(Gui::ATTACK, "You died!");
	Destructible::die(owner);
	engine.gameStatus = Engine::DEFEAT;
}
