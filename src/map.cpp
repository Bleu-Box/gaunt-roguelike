// implementation for Map
#include <vector>
#include "main.h"

static const int MIN_ITEMS = 1;
static const int MAX_ITEMS = 10;

static const int MIN_HORDES = 8;
static const int MAX_HORDES = 25;
static const int HORDE_SIZE = 3;

static const int MIN_PUDDLES = 0;
static const int MAX_PUDDLES = 15;
static const int MIN_PUDDLE_SIZE = 10;
static const int MAX_PUDDLE_SIZE = 25;

Map::Map(int width, int height): width(width), height(height) {
	tiles = new Tile*[width*height];
	map = new TCODMap(width, height);

	init();
}

Map::~Map() {
	delete[] tiles;
	delete map;
}

// generate map, add monsters, etc.
void Map::init() {
	TCODRandom* rand = TCODRandom::getInstance();

	// remake map until the playable area is big enough
	do {
		generateMap();
		int floodX, floodY;
		do {
			floodX = rand->getInt(0, width);
			floodY = rand->getInt(0, height);
		} while(!canWalk(floodX, floodY));
	
		floodFill(floodX, floodY);
		removeDisjointRooms();
	} while(getWalkableCoverage() < 0.38);

	// create puddles
	for(int i = 0; i < rand->getInt(MIN_PUDDLES, MAX_PUDDLES); i++) {
		int x, y;
		do {
			x = rand->getInt(0, width);
			y = rand->getInt(0, height);
		} while(!canWalk(x, y));
		spreadTile(x, y, rand->getInt(MIN_PUDDLE_SIZE, MAX_PUDDLE_SIZE), WATER_TILE);
	}

	// place player and stairs
	place(engine.player);
	place(engine.stairs);

	// spawn monsters
	for(int i = 0; i < rand->getInt(MIN_HORDES, MAX_HORDES); i++) {
		int x, y;
		do {
			x = rand->getInt(0, width);
			y = rand->getInt(0, height);
		} while(!canWalk(x, y));
		spawnHorde(x, y);
	}

	// spawn items
	for(int i = 0; i < rand->getInt(MIN_ITEMS, MAX_ITEMS); i++) {
		int x, y;
		do {
			x = rand->getInt(0, width);
			y = rand->getInt(0, height);
		} while(!canWalk(x, y));
		addItem(x, y);
	}
}

// TODO: make it so edges of map aren't so flat
void Map::generateMap() {
	TCODRandom* rand = TCODRandom::getInstance();
	for(int x = 0; x < width; x++) {
		for(int y = 0; y < height; y++) {
			if(rand->getInt(0, 100) < 45) {
				tiles[x+y*width] = new Tile('#', false, false, TCODColor::lightGrey);
				map->setProperties(x, y, false, false);
			} else {
				tiles[x+y*width] = new Tile('.', true, true, TCODColor::lightGrey);
				map->setProperties(x, y, true, true);
			}
		}
	}
	
	int i = 0;
	while(i < 7) {
		Tile** newTiles = new Tile*[width*height];
		for(int x = 0; x < width; x++) {
			for(int y = 0; y < height; y++) {			
				if((i < 3 && nbs(x, y, WALL_TILE.ch >= 5 || nbs(x, y, WALL_TILE.ch) <= 2)) ||
				   nbs(x, y, WALL_TILE.ch) >= 5 || x == 0 || x == width-1 || y == 0 || y == height-1) {
					newTiles[x+y*width] = new Tile('#', false, false, TCODColor::lightGrey);
					map->setProperties(x, y, false, false);
				} else {
					newTiles[x+y*width] = new Tile('.', false, false, TCODColor::lightGrey);
					map->setProperties(x, y, true, true);
				}
			}
		}

		tiles = newTiles;
		
		i++;
	}
}

// count number of neighboring tiles with given char
int Map::nbs(int x, int y, int ch) const {
	int count = 0;
	
	for(int i = -1; i < 2; i++) {
		for(int j = -1; j < 2; j++) {
			int nb_x = x+i;
			int nb_y = y+j;
			
			if(i != 0 || j != 0) {
				// tiles near the edges get a bonus to their count
				if(nb_x < 0 || nb_y < 0 || nb_x >= width || nb_y >= height) {
					count++;
				} else if(tiles[nb_x+nb_y*width]->ch == ch) {
					count++;
				}
			}
		}
	}

	return count;
}

// spread a tile around the map in a blob shape
void Map::spreadTile(int x, int y, int count, const Tile& tile) {
	TCODRandom* rand = TCODRandom::getInstance();
	if(canWalk(x, y)) {
		setTile(x, y, tile);
	}
	
	if(count > 0) {
		for(int i = -1; i < 1; i++) {
			for(int j = -1; j < 1; j++) {
				if(x+i > 0 && x+i < width && y+j > 0 && y+j < height && rand->getInt(0, 100) < 40) {
					spreadTile(x+i, y+j, count-1, tile);
				}
			}
		}
	}
}

Tile Map::getTile(int x, int y) const {
        return *tiles[x+y*width];
}

void Map::setTile(int x, int y, const Tile& tile) {
	if(x < 0 || x > width || y < 0 || y > height) return;

	Tile* t = tiles[x+y*width];
        t->ch = tile.ch;
	t->transparent = tile.transparent;
	t->walkable = tile.walkable;
	t->fgColor = tile.fgColor;
	t->bgColor = tile.bgColor;
	map->setProperties(x, y, t->transparent, t->walkable);
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
		rat->ai = new MonsterAi(2, 10);
		engine.actors.push_back(rat);
	} else if(choice < 90) {
		Actor* shroom = new Actor(x, y, 'm', "Mushroom", TCODColor::brass);
		shroom->attacker = new Attacker(3, 20, "thumps");	
		shroom->attacker->setEffect(Effect::POISON, rand->getInt(2, 10));
		shroom->destructible = new MonsterDestructible(10, 3, 0.5, TCODColor::brass);
		shroom->ai = new MonsterAi(1, 4);
		engine.actors.push_back(shroom);
	} else if(choice < 95) {
		Actor* slime = new Actor(x, y, 's', "Slime", TCODColor::green);
		slime->attacker = new Attacker(3, 10, "smudges");
		slime->spreadable = new Spreadable(1);
		slime->destructible = new MonsterDestructible(5, 3, 0, TCODColor::green);
		slime->ai = new MonsterAi(1, 3);
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
		redcap->ai = new MonsterAi(1, 5);
		engine.actors.push_back(redcap);
	}
}

void Map::floodFill(int x, int y) {
	if(!isWall(x, y) && !tiles[x+y*width]->ffillFlag)
		tiles[x+y*width]->ffillFlag = true;
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
			if(!isWall(x, y) && !tiles[x+y*width]->ffillFlag) {
			        tiles[x+y*width] = new Tile('#', false, false, TCODColor::lightGrey);;
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

// returns a monster to spawn based on level and randomness
Map::MonsterKind Map::chooseMonsterKind() {
	TCODRandom* rand = TCODRandom::getInstance();
	std::vector<MonsterKind> candidates;
	int level = engine.getLevel();
	
	candidates.push_back(RAT);
	if(level >= 2) candidates.push_back(MUSHROOM);
	if(level >= 3) candidates.push_back(SLIME);
	if(level >= 4) candidates.push_back(REDCAP);

	return candidates[rand->getInt(0, candidates.size()-1)];
}

void Map::spawnMonster(int x, int y, MonsterKind kind) {
	TCODRandom* rand = TCODRandom::getInstance();
	
	switch(kind) {
	case RAT: {
		Actor* rat = new Actor(x, y, 'r', "Rat", TCODColor::lightGrey);
		rat->attacker = new Attacker(3, 30, "bites");
		rat->destructible = new MonsterDestructible(10, 1, 0);
		rat->ai = new MonsterAi(2, 10);
		engine.actors.push_back(rat);
		break;
	}
	
	case MUSHROOM: {
		Actor* shroom = new Actor(x, y, 'm', "Mushroom", TCODColor::brass*0.5);
		shroom->attacker = new Attacker(3, 20, "thumps");	
		shroom->attacker->setEffect(Effect::POISON, rand->getInt(2, 10));
		shroom->destructible = new MonsterDestructible(10, 3, 0.5, TCODColor::brass*0.5);
		shroom->ai = new MonsterAi(1, 4);
		engine.actors.push_back(shroom);
		break;
	}
	
	case SLIME: {
		Actor* slime = new Actor(x, y, 's', "Slime", TCODColor::green);
		slime->attacker = new Attacker(3, 10, "smudges");
		slime->spreadable = new Spreadable(1);
		slime->destructible = new MonsterDestructible(5, 3, 0, TCODColor::green);
		slime->ai = new MonsterAi(1, 3);
		MonsterAi* mai = dynamic_cast<MonsterAi*>(slime->ai);
		mai->spreadPredicate = [](const Actor& self)->bool {
			return self.destructible->getHp() < self.destructible->getMaxHp();
		};		
		engine.actors.push_back(slime);
		break;
	}
	
	case REDCAP: {
		Actor* redcap = new Actor(x, y, 'R', "Redcap", TCODColor::darkerRed);
		redcap->attacker = new Attacker(4, 80, "clubs");
		Effect::EffectType types[2] = {Effect::POISON, Effect::BLINDNESS};
		redcap->attacker->setEffect(types[rand->getInt(0, 1)], rand->getInt(25, 100));
		redcap->destructible = new MonsterDestructible(10, 3, 0.5, TCODColor::brass*0.5);
		redcap->ai = new MonsterAi(1, 5);
		engine.actors.push_back(redcap);
		break;
	}
	}
}

// spawns a horde of monsters
void Map::spawnHorde(int x, int y) {
	TCODRandom* rand = TCODRandom::getInstance();

	for(int i = -HORDE_SIZE; i < HORDE_SIZE; i++) {
		for(int j = -HORDE_SIZE; j < HORDE_SIZE; j++) {
			int monst_x = x+i;
			int monst_y = y+i;
			MonsterKind kind = chooseMonsterKind();
			if(monst_x > 0 && monst_x < width && monst_y > 0 && monst_y < height
			   && canWalk(monst_x, monst_y) && rand->getInt(0, 100) < 70) {
				spawnMonster(monst_x, monst_y, kind);
			}
		}
	}
}

