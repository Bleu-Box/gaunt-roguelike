#include "main.h"
#include "tiles.h"

Tile::Tile(int ch, bool transparent, bool walkable, std::string name,
	   const TCODColor& fgColor, const TCODColor& bgColor):
        ffillFlag(false),  transparent(transparent), walkable(walkable),
	name(name), ch(ch), fgColor(fgColor), bgColor(bgColor) {
	#if DEBUG_MODE == 1
		explored = true;
        #else
		explored = false;
	#endif
}

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
