// implementation for AIs
#include <math.h>
#include "main.h"

static const int TRACKING_TURNS = 5; // how long monster follows player

Ai::Ai(int speed): confused(false), speed(speed) {}
  
PlayerAi::PlayerAi(int stealth): Ai(1), stealth(stealth) {}

void PlayerAi::update(Actor* owner) {
	// don't do anything if owner is dead
	if(owner->destructible && owner->destructible->isDead()) return;
	// otherwise, respond to keyboard input
	int dx = 0, dy = 0;
	bool action = false; // whether the key input counts as an action
	switch(engine.getLastKey().vk) {
	case TCODK_UP:
		dy = -1;
		action = true;
		break;
	case TCODK_DOWN:
		dy = 1;
		action = true;
		break;
	case TCODK_LEFT:
		dx = -1;
		action = true;
		break;
	case TCODK_RIGHT:
		dx = 1;
		action = true;
		break;
	case TCODK_CHAR:
		handleActionKey(owner, engine.getLastKey().c);
		action = true;
		break;
	default:
		break;
	}

	if(confused) {
		TCODRandom* rand = TCODRandom::getInstance();
		dx = rand->getInt(-1, 1);
		dy = rand->getInt(-1, 1);
	}
	
	if(action) {
	        engine.gameStatus = Engine::NEW_TURN;
		// update FOV if player moved; otherwise, regenerate health
		if(dx != 0 || dy != 0) {
		        if(moveOrAttack(owner, owner->x+dx, owner->y+dy)) engine.map->computeFov();
		} else {
			// if we're not attacking, we can regenerate health
			if(owner->destructible) owner->destructible->regenerate();
		}
	}
}
 
void PlayerAi::handleActionKey(Actor* owner, int ascii) {
	switch(ascii) {
	// 'g' = pick up item 
	case 'g': {
		bool found = false;
		// look for items in the map that owner is on top of
		for(Actor* actor : engine.actors) {
			if(actor->pickable && actor->x == owner->x && actor->y == owner->y) {
				// add item to inventory if inventory isn't full
				if(actor->pickable->pick(actor, owner)) {
					found = true;
				        engine.gui->message(Gui::OBSERVE,"You put the " + actor->getName() + " into your pack.");
					break;
				} else if(!found) {
					found = true;
				        engine.gui->message(Gui::OBSERVE, "Your inventory is full.");
				}
			}
		}
		// display a message if we didn't find anything
		if(!found) {
		        engine.gui->message(Gui::OBSERVE, "There's nothing here that you can pick up.");
		}
		
	        engine.gameStatus = Engine::NEW_TURN;
	}
		break;
	// 'i' = show inventory
	case 'i': {
		Actor* actor = getFromInventory(owner);
		if(actor) {
			actor->pickable->use(actor, owner);
		        engine.gameStatus = Engine::NEW_TURN;
		}
	}
		break;
        // drop an item
	case 'd': {
		Actor* actor = getFromInventory(owner);
		if(actor) {
		        actor->pickable->drop(actor, owner);
		        engine.gameStatus = Engine::NEW_TURN;
		}
	}
		break;
        // open door in indicated direction 
	case 'o': {
		engine.gui->message(Gui::OBSERVE, "Where would you like to open a door? Indicate direction using an arrow key.");
		TCOD_key_t key;
		TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL, true);
		int door_x = owner->x;
		int door_y = owner->y;
		
		if(key.vk == TCODK_UP) door_y--;
		else if(key.vk == TCODK_DOWN) door_y++;
		else if(key.vk == TCODK_LEFT) door_x--;
		else if(key.vk == TCODK_RIGHT) door_x++;
		else engine.gui->message(Gui::ACTION, "That\'s not a valid direction.");

		engine.map->openDoor(door_x, door_y);
	}
		break;
	// close door
	case 'c': {
		engine.gui->message(Gui::OBSERVE, "Where would you like to close a door? Indicate direction using an arrow key.");
		TCOD_key_t key;
		TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL, true);
		int door_x = owner->x;
		int door_y = owner->y;
		
		if(key.vk == TCODK_UP) door_y--;
		else if(key.vk == TCODK_DOWN) door_y++;
		else if(key.vk == TCODK_LEFT) door_x--;
		else if(key.vk == TCODK_RIGHT) door_x++;
		else engine.gui->message(Gui::ACTION, "That\'s not a valid direction.");

		engine.map->closeDoor(door_x, door_y);
	}
		break;
	// 'r' = rest 1 turn
	case 'r': {
	        engine.gameStatus = Engine::NEW_TURN;
	}
		break;
	// press '>' to go down stairs
	case '>': {
		if(engine.stairs->x == owner->x && engine.stairs->y == owner->y) {
			engine.nextLevel();
		} else {
		        engine.gui->message(Gui::OBSERVE, "There aren\'t any stairs here.");
		}
	}
		break;
	}
}
 
bool PlayerAi::moveOrAttack(Actor* owner, int targetx, int targety) {
	if(speed == 0 || engine.map->isWall(targetx, targety)) return false;
	// look for living actors to attack
	for(Actor* actor : engine.actors) {
		// attack any living actor occupying the place we want to be
		if(actor->blocks && actor->destructible && !actor->destructible->isDead()
		   && actor->x == targetx && actor->y == targety) {
			owner->attacker->attack(owner, actor);
			// return false since we didn't move
			return false;
		}
	}

	// return true if we've gotten far enough to update the location
	owner->x = targetx;
	owner->y = targety;
	return true;
}

Actor* PlayerAi::getFromInventory(Actor* owner) {
	// static variables only need to be made the first time they're needed
	static const int INV_WIDTH = 28;
	static const int INV_HEIGHT = 50;
	static TCODConsole console(INV_WIDTH, INV_HEIGHT);

	console.setDefaultForeground(TCODColor::white);
	console.printFrame(0, 0, INV_WIDTH, INV_HEIGHT, true, TCOD_BKGND_DEFAULT, "INVENTORY");

	int shortcut = 'a';
	int y = 1;
	for(Actor* actor : owner->container->inventory) {
		console.print(2, y, "(%c) %s", shortcut, actor->getName().c_str());
		y++;
		shortcut++;
	}

	// blit the inventory console on the root console
	TCODConsole::blit(&console, 0, 0, INV_WIDTH, INV_HEIGHT, TCODConsole::root, 0, 0);
	TCODConsole::flush();

	// wait for a key press
	TCOD_key_t key;
	TCODSystem::waitForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL, true);
	if(key.vk == TCODK_CHAR) {
		int actorIndex = key.c-'a';
		if(actorIndex >= 0 && actorIndex < owner->container->getSize()) {
			// handle out-of range exceptions
			try {
				return owner->container->inventory.at(actorIndex);
			} catch(std::out_of_range& x) {
			        engine.gui->message(Gui::OBSERVE, "There is no item here.");
			}
		}
	}
	return NULL;
}

MonsterAi::MonsterAi(int speed, int range): Ai(speed), range(range) {}

void MonsterAi::update(Actor* owner) {	
	// don't do anything if owner is dead
	if(owner->destructible && owner->destructible->isDead()) return;
	// otherwise, move towards the player or attack it (if it's visible to the owner)
	if(owner->getDistance(engine.player->x, engine.player->y) <= range+dynamic_cast<PlayerAi*>(engine.player->ai)->stealth) {
		// we know where the player is, so we can stop blindly following it and restore tracking turns
		moveCount = TRACKING_TURNS;
	} else {
		// use our tracking turns if we're following blindly
		moveCount--;
	}

	if(moveCount > 0) moveOrAttack(owner, engine.player->x, engine.player->y);
 
	if(owner->spreadable && spreadPredicate && spreadPredicate(*owner)) {
		owner->spreadable->spread(owner);
	}
}

void MonsterAi::moveOrAttack(Actor* owner, int targetx, int targety) {
	// if monster is confused, just move around randomly and don't do anything else
	if(confused) {
		TCODRandom* rand = TCODRandom::getInstance();
	        int randxstep = rand->getInt(-1, 1);
		int randystep = rand->getInt(-1, 1);
		
		if(engine.map->canWalk(owner->x+randxstep, owner->y)) owner->x += randxstep;
		if(engine.map->canWalk(owner->x, owner->y+randystep)) owner->y += randystep;
		
		return;
	}
  	
	int dx = targetx-owner->x;
	int dy = targety-owner->y;
	int stepdx = (dx == 0? 0 : dx > 0? 1 : -1);
	int stepdy = (dy == 0? 0 : dy > 0? 1 : -1);
   
	float distance = sqrtf(dx*dx+dy*dy);
 
	if(distance <= 1 && owner->attacker) {
		owner->attacker->attack(owner, engine.player);
	} else {
		// if we're not attacking, we can regenerate health
		if(owner->destructible) owner->destructible->regenerate();
		// if the monster's speed is high and the player is straight ahead, we can sprint
		// otherwise, just move one space at a time
		if(speed > 1 && (owner->x == engine.player->x || owner->y == engine.player->y)) {
			int i = speed;
			if(owner->y == engine.player->y) {
				while(engine.map->canWalk(owner->x+stepdx, owner->y) && --i > 0) {
					owner->x += stepdx;
				}
			} else if(owner->x == engine.player->x) {
				while(engine.map->canWalk(owner->x, owner->y+stepdy) && --i > 0) {
					owner->y += stepdy;
				}
			}
		} else {
			if(engine.map->canWalk(owner->x+stepdx, owner->y+stepdy)) {
				owner->x += stepdx;
				owner->y += stepdy;
			} else if(engine.map->canWalk(owner->x+stepdx, owner->y)) {
				owner->x += stepdx;
			} else if(engine.map->canWalk(owner->x, owner->y+stepdy)) {
				owner->y += stepdy;
			}
		} 
	}
}
