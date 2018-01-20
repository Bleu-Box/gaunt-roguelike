#include "main.h"

Tile::Tile(int ch, bool transparent, bool walkable,
	   const TCODColor& fgColor, const TCODColor& bgColor):
        ffillFlag(false), explored(true), transparent(transparent), walkable(walkable),
	ch(ch), fgColor(fgColor), bgColor(bgColor) {}

bool Tile::operator==(const Tile& rhs) const {
	return ch == rhs.ch &&
		transparent == rhs.transparent &&
		walkable == rhs.walkable &&
		fgColor == rhs.fgColor &&
		bgColor == rhs.bgColor;
}

bool Tile::operator!=(const Tile& rhs) const {
	return !(*this == rhs);
}
