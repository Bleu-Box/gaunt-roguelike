// definition for Map
#ifndef MAP_H
#define MAP_H

// structs are better for data-only classes and have public access by default
struct Tile {
	bool explored;
        Tile(): explored(false) {}
};

class Map {
 public:
	Map(int width, int height);
	~Map();

	bool isWall(int x, int y) const;
	bool canWalk(int x, int y) const;	
	void render(int xshift = 0, int yshift = 0) const;
	bool isInFov(int x, int y) const;
	bool isExplored(int x, int y) const;
	void computeFov();
	void addMonster(int x, int y);
	void addItem(int x, int y);
	void shift(int x, int y); // shift tiles a certain amount
	int getWidth() const { return width; }
	int getHeight() const { return height; }
	
 protected:
	friend class BspListener;
	
	Tile* tiles; // dynamically allocated array- you don't have to hardcode size here
	TCODMap* map;
	
	void dig(int x1, int y1, int x2, int y2);
	void createRoom(bool first, int x1, int y1, int x2, int y2);
	
 private:
	int width, height;
};

// a class for making a BSP tree for dungeon generation
class BspListener: public ITCODBspCallback {
 public:
       BspListener(Map& map): map(map), roomNum(0) {}

	// make rooms in the map
	bool visitNode(TCODBsp* node, void* userData);
	
 private:
	Map& map; // reference to Map we're putting rooms in
	int roomNum; // for keeping track of rooms
	int lastx, lasty; // center coords of last room
};

#endif
