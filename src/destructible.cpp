// implementation for Destructibles
#include "main.h"

Destructible::Destructible(float maxHp, float defense, float regen, const TCODColor& corpseColor):
        defense(defense), maxHp(maxHp), hp(maxHp), regen(regen), corpseColor(corpseColor) {}

// take damage and die if needed
float Destructible::takeDamage(Actor* owner, float damage) {
	// add a bloodstain
	engine.map->setTileForeground(owner->x, owner->y, corpseColor*0.5);
		
	if(defense != 0) damage /= defense;

	hp -= damage;
	if(hp <= 0) die(owner); 
	
	return damage;
}

void Destructible::die(Actor* owner) {
	// put a corpse on the map (the engine will clean up dead actors)
	engine.map->setTileForeground(owner->x, owner->y, corpseColor);
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
