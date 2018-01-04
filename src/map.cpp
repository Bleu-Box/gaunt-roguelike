// implementation for Map
#include <functional>
#include "main.h"

// `static' in this context means that the vars aren't visible outside of file
static const int ROOM_MAX_SIZE = 8;
static const int ROOM_MIN_SIZE = 4;
static const int MAX_ROOM_MONSTERS = 4;
static const int MAX_ROOM_ITEMS = 1;

Map::Map(int width, int height): width(width), height(height) {
	tiles = new Tile[width*height];
	map = new TCODMap(width, height);
	// add a BSP tree and have it make rooms in the map
        TCODBsp bsp(0, 0, width, height);
	// split recursively up to 8 times to make rooms at least the MAX_SIZEs; 1.5fs are for room dimensions/flatness
	bsp.splitRecursive(NULL, 50, ROOM_MAX_SIZE, ROOM_MAX_SIZE, 1.5f, 1.5f);
	BspListener listener(*this);
	bsp.traverseInvertedLevelOrder(&listener, NULL); // void* params can be just about anything
}

Map::~Map() {
	delete[] tiles;
	delete map;
}

bool Map::isWall(int x, int y) const {
	return !map->isWalkable(x, y);
}

bool Map::canWalk(int x, int y) const  {
	if(isWall(x, y)) return false;
	
	for(Actor* actor : engine.actors) {
		if(actor->blocks && actor->x == x && actor->y == y) return false;
	}

	return true;
}

bool Map::isExplored(int x, int y) const {
        return tiles[x+y*width].explored;
}

bool Map::isInFov(int x, int y) const {
	if (x < 0 || x >= width || y < 0 || y >= height) return false;
	
	if(map->isInFov(x, y)) {
		tiles[x+y*width].explored = true;
		return true;
	}

	return false;
}

void Map::computeFov() {
	map->computeFov(engine.player->x, engine.player->y, engine.getFovRadius());
}

// add items to the map randomly and send them to back
void Map::addItem(int x, int y) {
	TCODRandom* rand = TCODRandom::getInstance();
	int choice = rand->getInt(0, 100);
	
	if(choice < 90) {
		Actor* healthPotion = new Actor(x, y, '!', "health potion", TCODColor::yellow);
		healthPotion->blocks = false;
		healthPotion->pickable = new Healer(5);
		engine.actors.push_back(healthPotion);
		engine.sendToBack(healthPotion);
	}
}
 
void Map::addMonster(int x, int y) {
	TCODRandom* rand = TCODRandom::getInstance();
	int choice = rand->getInt(0, 100);
	   
	if(choice < 50) {
		Actor* rat = new Actor(x, y, 'r', "Rat", TCODColor::lightGrey);
		rat->attacker = new Attacker(3, 30, "bites");
		rat->destructible = new MonsterDestructible(10, 1, 0);
		rat->ai = new MonsterAi(2);
		engine.actors.push_back(rat);
	}  else if(choice < 80) {
		Actor* shroom = new Actor(x, y, 'm', "Mushroom", TCODColor::brass);
		shroom->attacker = new Attacker(3, 20, "thumps");
		// get random effect
		Effect::EffectType types[1] = {Effect::POISON};		
		shroom->attacker->setEffect(types[rand->getInt(0, 0)], rand->getInt(2, 10));
		shroom->destructible = new MonsterDestructible(10, 3, 0.5, TCODColor::brass);
		shroom->ai = new MonsterAi(1);
		engine.actors.push_back(shroom);
	} else if(choice < 95) {
		Actor* slime = new Actor(x, y, 's', "Slime", TCODColor::green);
		slime->attacker = new Attacker(3, 10, "smudges");
		slime->spreadable = new Spreadable(2);
		slime->destructible = new MonsterDestructible(5, 3, 0, TCODColor::green);
		slime->ai = new MonsterAi(1);
		MonsterAi* mai = dynamic_cast<MonsterAi*>(slime->ai);
	        mai->spreadPredicate = [](const Actor& self)->bool {
				return self.destructible->getHp() < self.destructible->getMaxHp();
		};		
		engine.actors.push_back(slime);
	} else {
		Actor* redcap = new Actor(x, y, 'r', "Redcap", TCODColor::darkerRed);
		redcap->attacker = new Attacker(4, 80, "clubs");
		Effect::EffectType types[2] = {Effect::POISON, Effect::BLINDNESS};
		redcap->attacker->setEffect(types[rand->getInt(0, 1)], rand->getInt(25, 100));
		redcap->destructible = new MonsterDestructible(10, 3, 0.5, TCODColor::brass);
		redcap->ai = new MonsterAi(1);
		engine.actors.push_back(redcap);
	}
}

void Map::dig(int x1, int y1, int x2, int y2) {
	// swap values if they appear to be in wrong order
	if(x2 < x1) {
		int temp = x2;
		x2 = x1;
		x1 = temp;
	}

	if(y2 < y1) {
		int temp = y2;
		y2 = y1;
		y1 = temp;
	}
        
	// open up tiles within rectangular area given
	for(int tilex = x1; tilex < x2; tilex++) {
		for(int tiley = y1; tiley < y2; tiley++) {
			map->setProperties(tilex, tiley, true, true);
		}
	}
}

// TODO: use flood fill technique to ensure all rooms are connected
void Map::createRoom(bool first, int x1, int y1, int x2, int y2) {
        TCODRandom* rand = TCODRandom::getInstance();
	// make a room
	dig(x1, y1, x2, y2);
	// decide what to put in room
	if(first) {
		// put player in center of room we made
		engine.player->x = (x1+x2)/2;
		engine.player->y = (y1+y2)/2;
	} else {
		// possibly put an NPC in the room
	        int numMonsters = rand->getInt(0, MAX_ROOM_MONSTERS);
		while(numMonsters > 0) {
			int x = rand->getInt(x1, x2);
			int y = rand->getInt(y1, y2);
			if(canWalk(x, y)) addMonster(x, y);
			numMonsters--;
		}
		// possibly put items into room
		int numItems = rand->getInt(0, MAX_ROOM_ITEMS);
		while(numItems > 0) {
			int x = rand->getInt(x1, x2);
			int y = rand->getInt(y1, y2);
			if(canWalk(x, y)) addItem(x, y);
			numItems--;
		}
		// place stairs in room so that stairs will end up in last room made
		int wallNum = rand->getInt(0, 3); // pick a wall to put the stairs on (represented by one of four ints)
		int wallx = (wallNum == 0? x1+1 : wallNum == 1? x2-1 : rand->getInt(x1+1, x2-1));
		int wally = (wallNum == 2? y1+1 : wallNum == 3? y2-1 : rand->getInt(y1+1, y2-1));
		// set location of stairs
		engine.stairs->x = wallx;
		engine.stairs->y = wally;
	}
}

void Map::render(int xshift, int yshift) const {
	// scan the map and color everything accordingly
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			if(isInFov(x, y)) {
				TCODConsole::root->setChar(x+xshift, y+yshift, isWall(x, y)? '#' : '.');
				TCODConsole::root->setCharForeground(x+xshift, y+yshift,
								     isWall(x, y)? TCODColor::lighterGrey : TCODColor::lightGrey);
			} else if(isExplored(x, y)) {
				TCODConsole::root->setChar(x+xshift, y+yshift, isWall(x, y)? '#' : '.');
				TCODConsole::root->setCharForeground(x+xshift, y+yshift, isWall(x, y)? TCODColor::darkestGrey : TCODColor::darkestGrey);
			}
		}
        }		
}

// make rooms in the map
bool BspListener::visitNode(TCODBsp* node, void* userData) {
	if(node->isLeaf()) {
		int x, y, w, h;
	        TCODRandom* rand = TCODRandom::getInstance();

		// make a room with random dimensions that fit current node size
		w = rand->getInt(ROOM_MIN_SIZE, node->w-2);
		h = rand->getInt(ROOM_MIN_SIZE, node->h-2);
		x = rand->getInt(node->x+1, node->x+node->w-w-1);
		y = rand->getInt(node->y+1, node->y+node->h-h-1);
		map.createRoom(roomNum == 0, x, y, x+w-1, y+h-1);
		// if this isn't the first room, dig corridors to last room
		if(roomNum != 0) {
			// dig a corridor connecting to last room
			map.dig(x+w/2, y+h/2, lastx, y+1+h/2);
			map.dig(x+w/2, y+h/2, x+1+w/2, lasty);
		}
			
		lastx = x+w/2;
		lasty = y+h/2;
		roomNum++;
	}
	
	return true; // this tells TCODBsp class to keep traversing tree
}
