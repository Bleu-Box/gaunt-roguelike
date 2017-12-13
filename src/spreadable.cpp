#include "main.h"

Spreadable::Spreadable(int lifespan): lifespan(lifespan) {}

// TODO: abstract out the repetetiveness
// spread children from parent
void Spreadable::spread(Actor* owner) {
	if(engine.map->canWalk(owner->x-1, owner->y)) addActor(owner, owner->x-1, owner->y);
	if(engine.map->canWalk(owner->x+1, owner->y)) addActor(owner, owner->x+1, owner->y);
	if(engine.map->canWalk(owner->x, owner->y-1)) addActor(owner, owner->x, owner->y-1);
	if(engine.map->canWalk(owner->x, owner->y+1)) addActor(owner, owner->x, owner->y+1);
}

// add slightly modified clone of owner to actor list
void Spreadable::addActor(Actor* owner, int x, int y) {
	if(lifespan >= 0) {
		// configure child 
		// TODO: this copying is kind of crude. You should prob improve it sometime
		Actor* a = new Actor(x, y, owner->ch, owner->name, owner->color*0.8);
		
		if(owner->attacker) a->attacker = new Attacker(owner->attacker->power, owner->attacker->accuracy, owner->attacker->action);
	        if(owner->attacker->isEnchanted()) a->attacker->setEffect(owner->attacker->getEffectType(), owner->attacker->getEffectDuration());
			
		if(owner->destructible) a->destructible = new Destructible(owner->destructible->maxHp, owner->destructible->hp,
									   owner->destructible->corpseName, owner->destructible->corpseColor);
		if(owner->ai) a->ai = new MonsterAi();
		if(owner->container) a->container = new Container(owner->container->size);
		
		a->spreadable = new Spreadable(owner->spreadable->lifespan-1);
		
		engine.actors.push(a);
		/*
		Actor a = *owner;
		a.x = x;
		a.y = y;
		a.color = owner->color*0.8;
		// copy the components so the child doesn't point to the parent's components
		a.destructible = NULL;
		a.attacker = NULL;
		a.pickable = NULL;
		a.ai = new MonsterAi();//NULL;
		a.pickable = NULL;
		a.container = NULL;
		a.spreadable = NULL;

		engine.actors.push(&a);
		*/
	}
}
