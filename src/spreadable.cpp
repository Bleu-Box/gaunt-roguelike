#include "main.h"
#include <iostream> // TODO: REMOVE
#include <functional> // TODO: perhaps remove

Spreadable::Spreadable(int lifespan): lifespan(lifespan) {}
// spread children from parent
void Spreadable::spread(Actor* owner) {
	int numChildren = 0;
	for(int y = -1; y <= 1; y++) {
		for(int x = -1; x <= 1; x++) {
			if(engine.map->canWalk(owner->x+x, owner->y+y)) {
				addActor(owner, owner->x+x, owner->y+y);
				numChildren++;
			}
		}
	}

	if(owner->spreadable) owner->spreadable->lifespan--;

	if(numChildren > 0) {
		engine.gui->message(Gui::OBSERVE, "More slimes appear!");
	}
}

// add slightly modified clone of owner to actor list
void Spreadable::addActor(Actor* owner, int x, int y) {
	if(lifespan > 0) {
		// configure child 
		Actor* a = new Actor(*owner);
		a->x = x;
		a->y = y;
		a->color = a->color*0.8;
		if(a->spreadable) a->spreadable->lifespan--;
		if(a->destructible) a->destructible->heal(a->destructible->getMaxHp());
		if(a->ai) {
			MonsterAi* aai = dynamic_cast<MonsterAi*>(a->ai);
			MonsterAi* oai = dynamic_cast<MonsterAi*>(owner->ai);
		        aai->spreadPredicate = oai->spreadPredicate;//[](const Actor& _) { return true; };
		}
		
	        engine.spawnActor(a);
	}
}
