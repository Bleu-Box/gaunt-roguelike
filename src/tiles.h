// the tiles included in the map
#ifndef TILES_H
#define TILES_H

#include <string>
#include "lib/libtcod.hpp"

struct Tile {
	bool ffillFlag; // used in flood fill
	bool explored;
	bool transparent;
	bool walkable;
	int ch;
	std::string name;
	TCODColor fgColor;
	TCODColor bgColor;
	
	Tile(int ch, bool transparent, bool walkable, std::string name,
	     const TCODColor& fgColor=TCODColor::white, const TCODColor& bgColor=TCODColor::black);

	bool operator==(const Tile& rhs) const;
	bool operator!=(const Tile& rhs) const;
};

namespace tiles {
	// dummy tiles are just walkable tiles used before tunnels are dug (so A* can go through them)
	// after tunnels are made, they're meant to be filled in with rock
	const Tile DUMMY_TILE = Tile('*', true, true, "nope");
	const Tile ROCK_TILE = Tile('#', false, false, "rock wall", TCODColor::black);
	// ground
	const Tile FLOOR_TILE = Tile('.', true, true, "floor tile");
	const Tile TUNNEL_TILE = Tile(TCOD_CHAR_BLOCK2, true, true, "tunnel floor tile");
	// walls
	const Tile TOP_WALL_TILE = Tile(TCOD_CHAR_HLINE, false, false, "cobblestone wall");
	const Tile SIDE_WALL_TILE = Tile(TCOD_CHAR_VLINE, false, false, "cobblestone wall");
	const Tile NW_WALL_TILE = Tile(TCOD_CHAR_NW, false, false, "cobblestone wall");
	const Tile NE_WALL_TILE = Tile(TCOD_CHAR_NE, false, false, "cobblestone wall");
	const Tile SW_WALL_TILE = Tile(TCOD_CHAR_SW, false, false, "cobblestone wall");
	const Tile SE_WALL_TILE = Tile(TCOD_CHAR_SE, false, false, "cobblestone wall");
	// door stuff
	const Tile CLOSED_DOOR_TILE = Tile('+', false, false, "closed door", TCODColor::lighterSepia);
	const Tile OPEN_DOOR_TILE = Tile('\'', true, true, "open door", TCODColor::lighterSepia);
	// spawns redcaps on contact with blood
	const Tile HEROESBANE_TILE = Tile('"', true, true, "patch of heros' bane", TCODColor::copper);
}

#endif
