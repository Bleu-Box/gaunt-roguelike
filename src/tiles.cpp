#include "main.h"

Tile::Tile(int ch, bool transparent, bool walkable,
	   const TCODColor& fgColor, const TCODColor& bgColor):
        ffillFlag(false), explored(false), transparent(transparent), walkable(walkable),
	ch(ch), fgColor(fgColor), bgColor(bgColor) {}
