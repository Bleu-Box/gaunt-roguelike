// implementation for Map
#include <vector>
#include <algorithm>
#include "main.h"
#include "map.h"
#include "actor.h"
#include "destructible.h"
#include "attacker.h"
#include "ai.h"
#include "spreadable.h"
#include "pickable.h"

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
	// also, turn open doors into closed doors
	for(int x = 0; x < width; x++) {
		for(int y = 0; y < height; y++) {
		        if(*tiles[x+y*width] == tiles::DUMMY_TILE)
			        setTile(x, y, tiles::ROCK_TILE);
			else if(*tiles[x+y*width] == tiles::OPEN_DOOR_TILE)
				setTile(x, y, tiles::CLOSED_DOOR_TILE);
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
	        
		if(rand->getInt(0, 100) < 70) {
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

	// don't spawn stairs on last level - instead spawn Amulet of Yendor
	if(engine.getLevel() < engine.MAX_LEVEL) {
		Room stairsRoom = rooms[rooms.size()-1];
		int stairs_x, stairs_y;
		do {
			stairs_x = rand->getInt(stairsRoom.x1+1, stairsRoom.x2-1);
			stairs_y = rand->getInt(stairsRoom.y1+1, stairsRoom.y2-1);
		} while(!canWalk(stairs_x, stairs_y));
		engine.stairs->x = stairs_x;
		engine.stairs->y = stairs_y;
	} else {
		Room amuletRoom = rooms[rooms.size()-1];
		int am_x, am_y;
		do {
		        am_x = rand->getInt(amuletRoom.x1+1, amuletRoom.x2-1);
		        am_y = rand->getInt(amuletRoom.y1+1, amuletRoom.y2-1);
		} while(!canWalk(am_x, am_y));
		engine.amulet->x = am_x;
		engine.amulet->y = am_y;
	}
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
	tiles[x+y*width] = new Tile(tile.ch, tile.transparent, tile.walkable, tile.name,
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
	
	// set the corners of the room
	setTile(room.x1, room.y1, tiles::NW_WALL_TILE);
	setTile(room.x2, room.y1, tiles::NE_WALL_TILE);
	setTile(room.x1, room.y2, tiles::SW_WALL_TILE);
	setTile(room.x2, room.y2, tiles::SE_WALL_TILE); 
}

// use A* to find a path between two rooms, and then dig it
void Map::digTunnel(Room& from, Room& to) {
	TCODRandom* rand = TCODRandom::getInstance();
	// randomly choose tunnel openings
	int startx = rand->getInt(from.x1+2, from.x2-2);
	int starty = rand->getInt(0, 1) == 0? from.y1 : from.y2;
	int endx = rand->getInt(0, 1) == 0? to.x1 : to.x2;
	int endy = rand->getInt(to.y1+1, to.y2-1);

	// there's a random chance of doors spawning instead of a plain opening
	// doors start out open so the pathfinding algorithm can go through them
	if(rand->getInt(0, 100) < 40) setTile(startx, starty, tiles::OPEN_DOOR_TILE);
	else setTile(startx, starty, tiles::FLOOR_TILE);
	if(rand->getInt(0, 100) < 40) setTile(endx, endy, tiles::OPEN_DOOR_TILE);	
	else setTile(endx, endy, tiles::FLOOR_TILE);
	
	// calculate the path
	TCODPath path = findPath(startx, starty, endx, endy, 0);
	// dig the tunnel
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
		t->name = tile.name;
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
	if(x < 0 || x >= width || y < 0 || y >= height) return;
	tiles[x+y*width]->fgColor = color;
}

void Map::setTileBackground(int x, int y, const TCODColor& color) {
	if(x < 0 || x >= width || y < 0 || y >= height) return;
	tiles[x+y*width]->bgColor = color;
}

// adding a bloodstain to heroes' bane will spawn redcaps
// heroes' bane is good for one spawn and then it's done
void Map::addBloodstain(int x, int y, const TCODColor& color) {
	Tile* tile = tiles[x+y*width];	
	if(*tile == tiles::HEROESBANE_TILE && canWalk(x, y)) {
		spawnMonster(x, y, REDCAP);
		*tile = tiles::FLOOR_TILE;
	}

	if(*tile == tiles::FLOOR_TILE)
		tile->fgColor = color;
}

void Map::openDoor(int x, int y) {
	Tile tile = getTile(x, y);
	if(tile == tiles::CLOSED_DOOR_TILE) {
		setTile(x, y, tiles::OPEN_DOOR_TILE);
	} 
}

void Map::closeDoor(int x, int y) {
	Tile tile = getTile(x, y);
	if(tile == tiles::OPEN_DOOR_TILE) {
		setTile(x, y, tiles::CLOSED_DOOR_TILE);
	} 
}

bool Map::isExplored(int x, int y) const {
        return tiles[x+y*width]->explored;
}

bool Map::isInFov(int x, int y) const {
	if(x < 0 || x >= width || y < 0 || y >= height) return false;
	
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
	
	if(choice < 70) {
		Potion* pick = new Potion();
		Actor* potion = new Actor(x, y, '!', pick->getName(), TCODColor::cyan);
		potion->blocks = false;
		potion->pickable = pick;
		engine.actors.push_back(potion);
		engine.sendToBack(potion);
	} else if(choice < 75) {
		Armor* pick = new Armor();
		Actor* armor = new Actor(x, y, ']', pick->getName(), TCODColor::cyan);
	        armor->blocks = false;
	        armor->pickable = pick;
		engine.actors.push_back(armor);
		engine.sendToBack(armor);
	} else {
		Weapon* pick = new Weapon();
		Actor* weapon = new Actor(x, y, '/', pick->getName(), TCODColor::cyan);
		weapon->blocks = false;
		weapon->pickable = pick;
		engine.actors.push_back(weapon);
		engine.sendToBack(weapon);
	}
}

// returns a monster to spawn based on level and randomness
Map::MonsterKind Map::chooseMonsterKind() {
	TCODRandom* rand = TCODRandom::getInstance();
	std::vector<MonsterKind> candidates;
	int level = engine.getLevel();
	
	if(level >= 1 && level <= 5) candidates.push_back(RAT);
	if(level >= 2) {
		std::vector<MonsterKind> shroomKinds = {MUSHROOM, BLUE_SHROOM, PURPLE_SHROOM};
		candidates.push_back(shroomKinds[rand->getInt(0, shroomKinds.size()-1)]);
		candidates.push_back(KOBOLD);
	}
	if(level >= 3) candidates.push_back(SLIME);
	// redcaps are rare
	if(level >= 4 && rand->getInt(0, 100) < 20) candidates.push_back(REDCAP);
	if(level >= 5) candidates.push_back(CENTIPEDE);
	if(level >= 6 && rand->getInt(0, 100) < 30) candidates.push_back(RED_CENTIPEDE);
	if(level >= 7) candidates.push_back(GOBLIN);
	if(level >= 8) candidates.push_back(NENN);
	if(level >= 9) candidates.push_back(YRCH);
	if(level >= 10 && rand->getInt(0, 100) < 30) candidates.push_back(BRIGHT);

	return candidates[rand->getInt(0, candidates.size()-1)];
}

// based on the supplied monster type, create a certain monster
void Map::spawnMonster(int x, int y, MonsterKind kind) {
	TCODRandom* rand = TCODRandom::getInstance();
	
	switch(kind) {
	case RAT: {
		Actor* rat = new Actor(x, y, 'r', "Rat", TCODColor::lightGrey);
		rat->attacker = new Attacker(3, 30, "bites");
		rat->destructible = new MonsterDestructible(5, 1, 0);
		rat->ai = new MonsterAi(2, 10);
		engine.actors.push_back(rat);
		break;
	}
	
	case MUSHROOM: {
		Actor* shroom = new Actor(x, y, 'm', "Mushroom", TCODColor::white*0.5);
		shroom->attacker = new Attacker(0, 50, "thumps");	
		shroom->attacker->setEffect(Effect::POISON, rand->getInt(2, 10));
		shroom->destructible = new MonsterDestructible(5, 2, 0.5, TCODColor::brass*0.5);
		shroom->ai = new MonsterAi(1, 4);
		engine.actors.push_back(shroom);
		break;
	}

	case BLUE_SHROOM: {
		Actor* shroom = new Actor(x, y, 'm', "Blue Mushroom", TCODColor::darkAzure);
		shroom->attacker = new Attacker(0, 50, "thumps");	
		shroom->attacker->setEffect(Effect::WASTING, rand->getInt(2, 10));
		shroom->destructible = new MonsterDestructible(5, 2, 0.5, TCODColor::brass*0.5);
		shroom->ai = new MonsterAi(1, 4);
		engine.actors.push_back(shroom);
		break;
	}

	case PURPLE_SHROOM: {
		Actor* shroom = new Actor(x, y, 'm', "Purple Mushroom", TCODColor::darkerPurple);
		shroom->attacker = new Attacker(0, 50, "thumps");	
		shroom->attacker->setEffect(Effect::BLINDNESS, 3);
		shroom->destructible = new MonsterDestructible(5, 2, 0.5, TCODColor::brass*0.5);
		shroom->ai = new MonsterAi(1, 4);
		engine.actors.push_back(shroom);
		break;
	}
	
	case KOBOLD: {
		Actor* kobold = new Actor(x, y, 'k', "Kobold", TCODColor::amber*0.5);
		kobold->attacker = new Attacker(5, 50, "hits");	
	        kobold->destructible = new MonsterDestructible(7, 3, 1, TCODColor::darkerGrey);
	        kobold->ai = new MonsterAi(1, 4, true);
		engine.actors.push_back(kobold);
		break;
	}
		
	case SLIME: {
		Actor* slime = new Actor(x, y, 's', "Slime", TCODColor::celadon);
		slime->attacker = new Attacker(3, 10, "smudges");
		slime->spreadable = new Spreadable(rand->getInt(1, 2));
		slime->destructible = new MonsterDestructible(5, 2, 0, TCODColor::celadon*0.5);
		slime->ai = new MonsterAi(1, 3, false, [](MonsterAi* ai, Actor* owner) {
				ai->pursuePlayer(owner);
				if(owner->destructible->getHp() < owner->destructible->getMaxHp()) {
					ai->spread(owner);
				}
			});
		engine.actors.push_back(slime);
		break;
	}
	
	case REDCAP: {
		Actor* redcap = new Actor(x, y, 'R', "Redcap", TCODColor::darkerRed);
		redcap->attacker = new Attacker(4, 80, "clubs");
		Effect::EffectType types[2] = {Effect::POISON, Effect::BLINDNESS};
		redcap->attacker->setEffect(types[rand->getInt(0, 1, 0)], rand->getInt(25, 50));
		redcap->destructible = new MonsterDestructible(10, 3, 0.5, TCODColor::brass*0.5);
		redcap->ai = new MonsterAi(1, 5, true);
		engine.actors.push_back(redcap);
		break;
	}

	case CENTIPEDE: {
		Actor* centi = new Actor(x, y, 'c', "Centipede", TCODColor::orange);
		centi->attacker = new Attacker(6, 90, "stings");
	        centi->destructible = new MonsterDestructible(15, 2, 1, TCODColor::darkGreen);
	        centi->ai = new MonsterAi(2, 3);
		engine.actors.push_back(centi);
		break;
	}

	case RED_CENTIPEDE: {
		Actor* centi = new Actor(x, y, 'c', "Red Centipede", TCODColor::red);
		centi->attacker = new Attacker(6, 90, "stings");
		centi->attacker->setEffect(Effect::POISON, rand->getInt(10, 50));
	        centi->destructible = new MonsterDestructible(15, 2, 1, TCODColor::darkGreen);
	        centi->ai = new MonsterAi(2, 3);
		engine.actors.push_back(centi);
		break;
	}

	case GOBLIN: {
		Actor* goblin = new Actor(x, y, 'g', "Goblin", TCODColor::darkGreen);
		goblin->attacker = new Attacker(7, 60, "stabs");
		goblin->destructible = new MonsterDestructible(10, 2, 1, TCODColor::darkerGrey);
	        goblin->ai = new MonsterAi(1, 10, true);
		engine.actors.push_back(goblin);
		break;
	}
	// a special type of magic-resistant goblin
	case NENN: {
		Actor* nenn = new Actor(x, y, 'n', "Nenn", TCODColor::darkHan);
		nenn->attacker = new Attacker(7, 60, "stabs");
	        nenn->destructible = new MonsterDestructible(10, 2, 1, TCODColor::darkerGrey);
	        nenn->ai = new MonsterAi(1, 10, true);
		nenn->resistsMagic = true;
		engine.actors.push_back(nenn);
		break;
	}

	case YRCH: {
		Actor* yrch = new Actor(x, y, 'y', "Yrch", TCODColor::darkGrey);
	        yrch->attacker = new Attacker(12, 80, "stabs");
		if(rand->getInt(0, 100) <= 40) yrch->attacker->setEffect(Effect::POISON, 5);
	        yrch->destructible = new MonsterDestructible(10, 5, 1.5, TCODColor::darkerGrey);
	        yrch->ai = new MonsterAi(1, 10, true);
		engine.actors.push_back(yrch);
		break;
	}
		
	case BRIGHT: {
		Actor* bright = new Actor(x, y, 'b', "Bright", TCODColor::lightYellow);
		bright->attacker = new Attacker(10, 70, "bites");
	        bright->destructible = new MonsterDestructible(20, 8, 3, TCODColor::darkGreen);
	        bright->ai = new MonsterAi(4, 10);
		engine.actors.push_back(bright);
		break;
	}
	}
}

// spawns a horde of monsters
void Map::spawnHorde(int x, int y) {
	static const int HORDE_AREA = 3;
	static const int MAX_HORDE_MEMBERS = 3;
	TCODRandom* rand = TCODRandom::getInstance();

	int hordeMembers = 0;
	for(int i = -HORDE_AREA; i < HORDE_AREA; i++) {
		for(int j = -HORDE_AREA; j < HORDE_AREA; j++) {
			int monst_x = x+i;
			int monst_y = y+j;
			MonsterKind kind = chooseMonsterKind();
			if(monst_x > 0 && monst_x < width && monst_y > 0 && monst_y < height
			   && canWalk(monst_x, monst_y) && rand->getInt(0, 100) < 30) {
				spawnMonster(monst_x, monst_y, kind);
				hordeMembers++;
			}
			
			if(hordeMembers >= MAX_HORDE_MEMBERS) break;
		}
	}
}

// Based on a chance, maybe spawn a monster.
// (used for in-level monster spawning)
void Map::maybeSpawnMonster(int chance) {
	TCODRandom* rand = TCODRandom::getInstance();

	if(rand->getInt(0, 100) < chance) {
		MonsterKind kind = chooseMonsterKind();
		int i = rand->getInt(0, rooms.size()-1);
		Room room = rooms[i];
		
		int x = rand->getInt(room.x1+1, room.x2-1);
		int y = rand->getInt(room.y1+1, room.y2-1);
		spawnMonster(x, y, kind);
	}
}
