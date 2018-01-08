// definition for Map
#ifndef MAP_H
#define MAP_H

struct Tile {
	bool visited; // used in flood fill
	bool explored;
	int ch;
	TCODColor fgColor;
	TCODColor bgColor;
	Tile(int ch, const TCODColor& fgColor=TCODColor::white, const TCODColor& bgColor=TCODColor::black);
};

class Map {
 public:	
        Map(int width, int height);
        ~Map();

	bool isWall(int x, int y) const;
	bool canWalk(int x, int y) const;
	void setTileForeground(int x, int y, const TCODColor& color);
	void setTileBackground(int x, int y, const TCODColor& color);
	void render(int xshift = 0, int yshift = 0) const;
	bool isInFov(int x, int y) const;
	bool isExplored(int x, int y) const;
	void computeFov();
	void shift(int x, int y); // shift tiles a certain amount
	int getWidth() const { return width; }
	int getHeight() const { return height; }
	
 private:
	Tile** tiles;
	TCODMap* map;
	int width, height;

	void init();
	float getWalkableCoverage();
        void addMonster(int x, int y);
        void addItem(int x, int y);
	void generateMap();
	int nbs(int x, int y) const;
	void floodFill(int x, int y);
        void removeDisjointRooms();
	void place(Actor* actor);
};

#endif
