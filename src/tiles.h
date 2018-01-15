// the tiles included in the map
#ifndef TILES_H
#define TILES_H

struct Tile {
	bool ffillFlag; // used in flood fill
	bool explored;
	bool transparent;
	bool walkable;
	int ch;
	TCODColor fgColor;
	TCODColor bgColor;
	
	Tile(int ch, bool transparent, bool walkable,
	     const TCODColor& fgColor=TCODColor::white, const TCODColor& bgColor=TCODColor::black);

	bool operator==(const Tile& rhs) const;
	bool operator!=(const Tile& rhs) const;
};

namespace tiles {
	// dummy tiles are just walkable tiles used before tunnels are dug (so A* can go through them)
	// after tunnels are made, they're meant to be filled in with rock
	const Tile DUMMY_TILE = Tile('*', true, true);
	const Tile ROCK_TILE = Tile('#', false, false, TCODColor::black);
	const Tile FLOOR_TILE = Tile('.', true, true);
	const Tile TUNNEL_TILE = Tile(TCOD_CHAR_BLOCK2, true, true);
	const Tile TOP_WALL_TILE = Tile('-', false, false);
	const Tile SIDE_WALL_TILE = Tile('|', false, false);
	// spawns redcaps on contact with blood
	const Tile HEROESBANE_TILE = Tile('"', true, true, TCODColor::copper);
}
 
#endif
