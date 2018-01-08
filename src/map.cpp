// implementation for Map
#include <functional>
#include "main.h"

static const int MAX_ITEMS = 10;
static const int MAX_MONSTERS = 50;

Tile::Tile(int ch, const TCODColor& fgColor, const TCODColor& bgColor):
	visited(false), explored(false), ch(ch), fgColor(fgColor), bgColor(bgColor) {}

Map::Map(int width, int height): width(width), height(height) {
	tiles = new Tile*[width*height];
	map = new TCODMap(width, height);

	do {
		init();
	} while(getWalkableCoverage() < 0.38);
}

Map::~Map() {
	delete[] tiles;
	delete map;
}

// generate map, add monsters, etc.
void Map::init() {
	generateMap();

	TCODRandom* rand = TCODRandom::getInstance();
	int floodX, floodY;
        do {
		floodX = rand->getInt(0, width);
		floodY = rand->getInt(0, height);
	} while(!canWalk(floodX, floodY));
	
	floodFill(floodX, floodY);
	removeDisjointRooms();
	
	place(engine.player);
	place(engine.stairs);

	int numMonsters = 0;
	while(numMonsters < MAX_MONSTERS) {
		int x, y;
		do {
			x = rand->getInt(0, width);
			y = rand->getInt(0, height);
		} while(!canWalk(x, y));

		addMonster(x, y);
		numMonsters++;
	}

	int numItems = 0;
	while(numItems < MAX_ITEMS) {
		int x, y;
		do {
			x = rand->getInt(0, width);
			y = rand->getInt(0, height);
		} while(!canWalk(x, y));

		addItem(x, y);
		numItems++;
	}
}

void Map::generateMap() {
	TCODRandom* rand = TCODRandom::getInstance();
	for(int x = 0; x < width; x++) {
		for(int y = 0; y < height; y++) {
			if(rand->getInt(0, 100) < 45) {
				tiles[x+y*width] = new Tile('#', TCODColor::lightGrey);
			} else {
				tiles[x+y*width] = new Tile('.', TCODColor::lightGrey);
				map->setProperties(x, y, true, true);
			}
		}
	}
	
	int i = 0;
	while(i < 7) {
		Tile** newTiles = new Tile*[width*height];
		for(int x = 0; x < width; x++) {
			for(int y = 0; y < height; y++) {
				if((i < 3 && (nbs(x, y) >= 5 || nbs(x, y) <= 2)) || nbs(x, y) >= 5
				   || x == 0 || x == (width-1) || y == 0 || y == (height-1)) {
					newTiles[x+y*width] = new Tile('#', TCODColor::lightGrey);
					map->setProperties(x, y, false, false);
				} else {
					newTiles[x+y*width] = new Tile('.', TCODColor::lightGrey);
					map->setProperties(x, y, true, true);
				}
			}
		}

		tiles = newTiles;
		
		i++;
	}
}

// count number of neighboring walls
int Map::nbs(int x, int y) const {
	int count = 0;
	for(int i = -1; i < 2; i++) {
		for(int j = -1; j < 2; j++) {
			int nb_x = x+i;
			int nb_y = y+j;

			if(i != 0 || j != 0) {
				// tiles near the edges basically get a bonus to their count
				if(nb_x < 0 || nb_y < 0 || nb_x >= width || nb_y >= height) {
					count++;
				} else if(tiles[nb_x+nb_y*width]->ch == '#') {
					count++;
				}
			}
		}
	}

	return count;
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

void Map::setTileForeground(int x, int y, const TCODColor& color) {
	tiles[x+y*width]->fgColor = color;
}

void Map::setTileBackground(int x, int y, const TCODColor& color) {
	tiles[x+y*width]->bgColor = color;
}

bool Map::isExplored(int x, int y) const {
        return tiles[x+y*width]->explored;
}

bool Map::isInFov(int x, int y) const {
	if (x < 0 || x >= width || y < 0 || y >= height) return false;
	
	if(map->isInFov(x, y)) {
		tiles[x+y*width]->explored = true;
		return true;
	}

	return false;
}

void Map::computeFov() {
	map->computeFov(engine.player->x, engine.player->y, engine.getFovRadius());
}

void Map::render(int xshift, int yshift) const {
	// scan the map and color everything accordingly
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			if(isInFov(x, y)) {
				TCODConsole::root->setChar(x+xshift, y+yshift, tiles[x+y*width]->ch);
				TCODConsole::root->setCharForeground(x+xshift, y+yshift, tiles[x+y*width]->fgColor);
				TCODConsole::root->setCharBackground(x+xshift, y+yshift, tiles[x+y*width]->bgColor);
			} else if(isExplored(x, y)) {
				TCODConsole::root->setChar(x+xshift, y+yshift, tiles[x+y*width]->ch);
				TCODConsole::root->setCharForeground(x+xshift, y+yshift, tiles[x+y*width]->fgColor*0.2);
				TCODConsole::root->setCharBackground(x+xshift, y+yshift, tiles[x+y*width]->bgColor);
			}
		}
        }		
}

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
	} else if(choice < 70) {
		Actor* shroom = new Actor(x, y, 'm', "Mushroom", TCODColor::brass);
		shroom->attacker = new Attacker(3, 20, "thumps");	
		shroom->attacker->setEffect(Effect::POISON, rand->getInt(2, 10));
		shroom->destructible = new MonsterDestructible(10, 3, 0.5, TCODColor::brass);
		shroom->ai = new MonsterAi(1);
		engine.actors.push_back(shroom);
	} else if(choice < 90) {
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
		Actor* redcap = new Actor(x, y, 'R', "Redcap", TCODColor::darkerRed);
		redcap->attacker = new Attacker(4, 80, "clubs");
		Effect::EffectType types[2] = {Effect::POISON, Effect::BLINDNESS};
		redcap->attacker->setEffect(types[rand->getInt(0, 1)], rand->getInt(25, 100));
		redcap->destructible = new MonsterDestructible(10, 3, 0.5, TCODColor::brass);
		redcap->ai = new MonsterAi(1);
		engine.actors.push_back(redcap);
	}
}

void Map::floodFill(int x, int y) {
	if(!isWall(x, y) && !tiles[x+y*width]->visited)
		tiles[x+y*width]->visited = true;
	else return;
	
	floodFill(x, y-1);
	floodFill(x-1, y);
	floodFill(x+1, y);
	floodFill(x, y+1);
}

// see how much of the map is covered in walkable tiles
float Map::getWalkableCoverage() {
	float walkableCount = 0.0;
	
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			if(!isWall(x, y)) walkableCount++;
		}
	}

	return walkableCount/(width*height);
}

// fill rooms that are closed off from main map
void Map::removeDisjointRooms() {
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			if(!isWall(x, y) && !tiles[x+y*width]->visited) {
			        tiles[x+y*width] = new Tile('#', TCODColor::lightGrey);;
				map->setProperties(x, y, false, false);
			}
		}
	}
}

// place an actor on a tile, making sure it is a walkable tile
// for anything that's not a player, make sure it doesn't spawn in FOV
void Map::place(Actor* actor) {
	TCODRandom* rand = TCODRandom::getInstance();

	while(true) {
		int x = rand->getInt(0, width);
		int y = rand->getInt(0, height);

		if(canWalk(x, y)) {
			actor->x = x;
			actor->y = y;
			break;
		}
	}
}
