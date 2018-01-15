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

	// initialize rooms at random locations
	// they *should* be non-overlapping
	// TODO: make sure rooms don't end on edges of map
	for(int i = 0; i < 20; i++) {
		int x2 = rand->getInt(7, width);
		int y2 = rand->getInt(7, height);
		int x1 = x2-rand->getInt(5, 10);
		int y1 = y2-rand->getInt(5, 10);
		// prevent rooms from intersection each other
		// TODO: fix this because it doesn't work of course
		bool intersects = false;
		for(Room other : rooms) {
			if((other.x1 >= x1 && other.x1 <= x2) || (other.y1 >= y1 && other.y1 < y2)) {
				intersects = true;
				break;
			}
		}

		if(intersects) continue;
		Room room {x1, y1, x2, y2};
		digRoom(room);
		rooms.push_back(room);
		engine.player->x = (x1+x2)/2;
		engine.player->y = (y1+y2)/2;
	}

	// sort rooms by position
	std::sort(rooms.begin(), rooms.end(),
		   [](Room a, Room b) {
			  return a.x1+a.y1 < b.x1+b.y1;
		  });

	// TODO: make sure to remove any unmatched rooms, or match them
	std::deque<Room> candidates = rooms;	
	for(Room room : rooms) {
		if(candidates.size() <= 1) break;
		digTunnel(room, candidates.at(1));
		candidates.pop_front();
	}

	// make dummy tiles (i.e. the ones that aren't part of rooms are tunnels) into solid rock
	// TODO: fix the bug where there are leftover dummy tiles
	for(int x = 0; x < width; x++) {
		for(int y = 0; y < height; y++) {
		        if(*tiles[x+y*width] == tiles::DUMMY_TILE)
			        setTile(x, y, tiles::ROCK_TILE);
		}
	}

	// spawn monsters - TODO: improve spawning and also spawn items and features
	for(Room room : rooms) {
		if(rand->getInt(0, 100) < 60)
			spawnHorde(rand->getInt(room.x1+1, room.x2-1),
				  rand->getInt(room.y1+1, room.y2-1));
	}
}

void Map::initTile(int x, int y, const Tile& tile) {
	tiles[x+y*width] = new Tile(tile.ch, tile.transparent, tile.walkable,
				    tile.fgColor, tile.bgColor);
	map->setProperties(x, y, tile.transparent, tile.walkable);
}

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
void Map::digTunnel(Room from, Room to) {
	TCODRandom* rand = TCODRandom::getInstance();
	int startx = rand->getInt(from.x1+2, from.x2-2);
	int starty = rand->getInt(0, 1) == 0? from.y1 : from.y2;
	int endx = rand->getInt(0, 1) == 0? to.x1 : to.x2;
	int endy = rand->getInt(to.y1+1, to.y2-1);

	setTile(startx, starty, tiles::FLOOR_TILE);
	setTile(endx, endy, tiles::FLOOR_TILE);
	
	TCODPath* path = new TCODPath(map, 0);
        path->compute(startx, starty, endx, endy);
	for(int i = 0; i < path->size(); i++) {
		int x, y;
		path->get(i, &x, &y);
		setTile(x, y, tiles::TUNNEL_TILE);
	}
}

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

void Map::dig(int x, int y, int w, int h) {
	for(int i = 0; i < w; i++) {
		for(int j = 0; j < h; j++) {
			setTile(x+i, y+j, tiles::TUNNEL_TILE);
		}
	}
}

Tile Map::getTile(int x, int y) const {
        return *tiles[x+y*width];
}

void Map::setTile(int x, int y, const Tile& tile) {
	if(x > 0 && x < width && y > 0 && y < height) {
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
