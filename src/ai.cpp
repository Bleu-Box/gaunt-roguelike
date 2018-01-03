// implementation for AIs
#include <math.h>
#include "main.h"

static const int TRACKING_TURNS = 5; // how long monster follows player

Ai::Ai(int speed): confused(false), speed(speed) {}
  
PlayerAi::PlayerAi(): Ai(1) {}

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

// TODO: using things doesn't clear their names from inventory (but it does remove them from container)
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

MonsterAi::MonsterAi(int speed): Ai(speed) {}

void MonsterAi::update(Actor* owner) {	
	// don't do anything if owner is dead
	if(owner->destructible && owner->destructible->isDead()) return;
	// otherwise, move towards the player or attack it (if it's visible to the owner)
	if(engine.map->isInFov(owner->x,owner->y)) {
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
   int dx = targetx-owner->x;
   int dy = targety-owner->y;
 
   int stepdx = (dx == 0? 0 : dx > 0? 1 : -1);
   int stepdy = (dy == 0? 0 : dy > 0? 1 : -1);

   if(confused) {
	   TCODRandom* rand = TCODRandom::getInstance();
	   dx = rand->getInt(-1, 1);
	   dy = rand->getInt(-1, 1);
   }
   
   float distance = sqrtf(dx*dx+dy*dy);
 
   if(distance <= 1 && owner->attacker) {
	   owner->attacker->attack(owner, engine.player);
   } else {
	   // if we're not attacking, we can regenerate health
	   if(owner->destructible) owner->destructible->regenerate();
	   for(int i = speed; i > 0; i--) {
		   if(engine.map->canWalk(owner->x+stepdx*i, owner->y+stepdy*i)) {
			   owner->x += stepdx*i;
			   owner->y += stepdy*i;
			   break;
		   } else if(engine.map->canWalk(owner->x+stepdx*i, owner->y)) {
			   owner->x += stepdx*i;
			   break;
		   } else if(engine.map->canWalk(owner->x, owner->y+stepdy*i)) {
			   owner->y += stepdy*i;
			   break;
		   }
	   }
   }
}
