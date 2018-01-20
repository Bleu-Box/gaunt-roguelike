// implementation for Map
#include <vector>
#include <algorithm>
#include "main.h"

static const int MIN_HORDES = 8;
static const int MAX_HORDES = 25;
static const int HORDE_SIZE = 3;

Map::Map(int width, int height): width(width), height(height) {
	tiles = new Tile*[width*height];
	regions = new int[width*height];
	map = new TCODMap(width, height);

	init();
}

Map::~Map() {
	delete[] tiles;
        rooms.clear();
	delete map;
}

// generate map, add monsters, etc.
void Map::init() {
	TCODRandom* rand = TCODRandom::getInstance();
	// set every tiles to walkable initially
	// dummy tiles are so named because they're only there so the tunnel-building algorithm
	// is able to 'walk' through them - they get turned into rock afterwards
	for(int x = 0; x < width; x++) {
		for(int y = 0; y < height; y++) {
			initTile(x, y, tiles::DUMMY_TILE);      
		}
	}

	makeRooms(rand->getInt(5, 15));
	connectRooms();
	
	// make dummy tiles (i.e. the ones that aren't part of rooms are tunnels) into solid rock
	for(int x = 0; x < width; x++) {
		for(int y = 0; y < height; y++) {
		        if(*tiles[x+y*width] == tiles::DUMMY_TILE)
			        setTile(x, y, tiles::ROCK_TILE);
		}
	}

	// on later levels, spawn heroes' bane
        if(engine.getLevel() >= 4) {
		int maxFields = rand->getInt(0, rooms.size()-2);
		int numFields = 0;
		for(Room room : rooms) {
			if(numFields >= maxFields) break;
			spreadTile(rand->getInt(room.x1+1, room.x2-1),
				   rand->getInt(room.y1+1, room.y2-1),
				   5, tiles::HEROESBANE_TILE);
			numFields++;
		}
	}

	// spawn monsters and items
	for(Room room : rooms) {
		if(rand->getInt(0, 100) < 60) {
			int x = rand->getInt(room.x1+1, room.x2-1);
			int y = rand->getInt(room.y1+1, room.y2-1);
		        spawnHorde(x, y);
		}
		
		if(rand->getInt(0, 100) < 30) {
			int x = rand->getInt(room.x1+1, room.x2-1);
			int y = rand->getInt(room.y1+1, room.y2-1);
		        addItem(x, y);
		}
	}

	// spawn player (and then stairs) on a walkable tile
	Room spawnpoint = rooms[0];
	int player_x, player_y;
	do {
	        player_x = rand->getInt(spawnpoint.x1+1, spawnpoint.x2-1);
	        player_y = rand->getInt(spawnpoint.y1+1, spawnpoint.y2-1);
	} while(!canWalk(player_x, player_y));
	engine.player->x = player_x;
	engine.player->y = player_y;

	Room stairsRoom = rooms[rooms.size()-1];
	int stairs_x, stairs_y;
	do {
	        stairs_x = rand->getInt(stairsRoom.x1+1, stairsRoom.x2-1);
	        stairs_y = rand->getInt(stairsRoom.y1+1, stairsRoom.y2-1);
	} while(!canWalk(stairs_x, stairs_y));
	engine.stairs->x = stairs_x;
	engine.stairs->y = stairs_y;
}

// make and dig all the rooms at random locations
void Map::makeRooms(int count) {
	TCODRandom* rand = TCODRandom::getInstance();
	
	for(int i = 0; i < count; i++) {
		int x2 = rand->getInt(15, width-1);
		int y2 = rand->getInt(15, height-1);
		int x1 = x2-rand->getInt(5, 10);
		int y1 = y2-rand->getInt(5, 10);
		// don't allow overlapping rooms
		bool overlap = false;
		for(int x = x1; x <= x2; x++) {
			for(int y = y1; y <= y2; y++) {
				if(getTile(x, y) != tiles::DUMMY_TILE)
					overlap = true;
			}
		}
		
		if(overlap) continue;
		Room room {x1, y1, x2, y2};
		digRoom(room);	
		rooms.push_back(room);
	}
}

// connect each room to the next in the list until you reach the end of the list
void Map::connectRooms() {
	int i = 0;
	for(Room room : rooms) {
	        if(i < static_cast<int>(rooms.size()-1)) {
			digTunnel(room, rooms[i+1]);
		} else {
			digTunnel(room, rooms[0]);
		}
		
		i++;
	}
}

// initialize a tile to a certain type (not to be confused with Map::setTile)
void Map::initTile(int x, int y, const Tile& tile) {
	tiles[x+y*width] = new Tile(tile.ch, tile.transparent, tile.walkable,
				    tile.fgColor, tile.bgColor);
	map->setProperties(x, y, tile.transparent, tile.walkable);
}

// dig a room with walls on the sides and floor tiles in the middle
void Map::digRoom(Room room) {
        for(int x = room.x1; x <= room.x2; x++) {
		for(int y = room.y1; y <= room.y2; y++) {
			if(x == room.x1 || x == room.x2) setTile(x, y, tiles::SIDE_WALL_TILE);
			else if(y == room.y1 || y == room.y2) setTile(x, y, tiles::TOP_WALL_TILE);
			else setTile(x, y, tiles::FLOOR_TILE);
		}
	}
}

// use A* to find a path between two rooms, and then dig it
void Map::digTunnel(Room& from, Room& to) {
	TCODRandom* rand = TCODRandom::getInstance();
	
	int startx = rand->getInt(from.x1+2, from.x2-2);
	int starty = rand->getInt(0, 1) == 0? from.y1 : from.y2;
	int endx = rand->getInt(0, 1) == 0? to.x1 : to.x2;
	int endy = rand->getInt(to.y1+1, to.y2-1);

	setTile(startx, starty, tiles::FLOOR_TILE);
	setTile(endx, endy, tiles::FLOOR_TILE);
	
	TCODPath path = findPath(startx, starty, endx, endy, 0);	
	for(int i = 0; i < path.size(); i++) {
		int x, y;
		path.get(i, &x, &y);
		if(*tiles[x+y*width] == tiles::DUMMY_TILE)
			setTile(x, y, tiles::TUNNEL_TILE);
	}
}

// spread a tile around the map in a blob shape using a random flood-fill-like method
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

// dig a rectangle
void Map::dig(int x, int y, int w, int h) {
	for(int i = 0; i < w; i++) {
		for(int j = 0; j < h; j++) {
			setTile(x+i, y+j, tiles::TUNNEL_TILE);
		}
	}
}

// compute a path using A*
TCODPath Map::findPath(int x1, int y1, int x2, int y2, float diagCost) {
	TCODPath path = TCODPath(map, diagCost);
	path.compute(x1, y1, x2, y2);
	return path;
}

Tile Map::getTile(int x, int y) const {
        return *tiles[x+y*width];
}

// not to be confused with Map::initTile
void Map::setTile(int x, int y, const Tile& tile) {
	if(x >= 0 && x <= width && y >= 0 && y <= height) {
		Tile* t = tiles[x+y*width];
		t->ch = tile.ch;
		t->transparent = tile.transparent;
		t->walkable = tile.walkable;
		t->fgColor = tile.fgColor;
		t->bgColor = tile.bgColor;
		map->setProperties(x, y, t->transparent, t->walkable);
	}
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

// adding a bloodstain to heroes' bane will spawn redcaps
// heroes' bane is good for one spawn and then it's done
void Map::addBloodstain(int x, int y, const TCODColor& color) {
	Tile* tile = tiles[x+y*width];	
	if(*tile == tiles::HEROESBANE_TILE) {
		spawnMonster(x, y, REDCAP);
		engine.gui->message(Gui::ACTION, "Your blood soaks the heroes\' bane. A redcap appears! ");
		*tile = tiles::FLOOR_TILE;
	}

	if(*tile != tiles::ROCK_TILE) tile->fgColor = color;
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

void Map::render() const {
	// scan the map and color everything accordingly
	for(int y = 0; y < height; y++) {
		for(int x = 0; x < width; x++) {
			if(isInFov(x, y)) {
				TCODConsole::root->setChar(x, y, tiles[x+y*width]->ch);
				TCODConsole::root->setCharForeground(x, y, tiles[x+y*width]->fgColor);
				TCODConsole::root->setCharBackground(x, y, tiles[x+y*width]->bgColor);
			} else if(isExplored(x, y)) {
				TCODConsole::root->setChar(x, y, tiles[x+y*width]->ch);
				TCODConsole::root->setCharForeground(x, y, tiles[x+y*width]->fgColor*0.2);
				TCODConsole::root->setCharBackground(x, y, tiles[x+y*width]->bgColor);
			}
		}
        }		
}

void Map::addItem(int x, int y) {
	if(!canWalk(x, y)) return;

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

// returns a monster to spawn based on level and randomness
Map::MonsterKind Map::chooseMonsterKind() {
	TCODRandom* rand = TCODRandom::getInstance();
	std::vector<MonsterKind> candidates;
	int level = engine.getLevel();
	
	candidates.push_back(RAT);
	if(level >= 2) candidates.push_back(MUSHROOM);
	if(level >= 3) candidates.push_back(SLIME);
	// redcaps are rare
	if(level >= 4 && rand->getInt(0, 100) < 20) candidates.push_back(REDCAP);
	// bias spawning towards higher-level enemies
	return candidates[rand->getInt(0, candidates.size()-1, candidates.size()-2)];
}

// based on the supplied monster type, create a certain monster
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
		shroom->attacker = new Attacker(0, 100, "thumps");	
		shroom->attacker->setEffect(Effect::POISON, rand->getInt(2, 10));
		shroom->destructible = new MonsterDestructible(5, 2, 0.5, TCODColor::brass*0.5);
		shroom->ai = new MonsterAi(1, 4);
		engine.actors.push_back(shroom);
		break;
	}
	
	case SLIME: {
		Actor* slime = new Actor(x, y, 's', "Slime", TCODColor::desaturatedGreen);
		slime->attacker = new Attacker(3, 10, "smudges");
		slime->spreadable = new Spreadable(1);
		slime->destructible = new MonsterDestructible(5, 3, 0, TCODColor::desaturatedYellow);
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
