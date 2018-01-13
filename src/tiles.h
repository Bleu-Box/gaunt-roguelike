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
};

const Tile FLOOR_TILE = Tile('.', true, true, TCODColor::lightGrey);
const Tile WALL_TILE = Tile('.', false, false, TCODColor::lightGrey);
const Tile WATER_TILE = Tile('~', true, true, TCODColor::blue, TCODColor::darkBlue);
const Tile GRASS_TILE = Tile('\'', true, true, TCODColor::desaturatedGreen);

#endif
