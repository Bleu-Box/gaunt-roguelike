// definition for Map
#ifndef MAP_H
#define MAP_H

#include <deque>

struct Room {
	int x1, y1, x2, y2;
};

class Map {
 public:
	enum MonsterKind {RAT, MUSHROOM, SLIME, REDCAP};
	
        Map(int width, int height);
        ~Map();

	bool isWall(int x, int y) const;
	bool canWalk(int x, int y) const;
	void setTileForeground(int x, int y, const TCODColor& color);
	void setTileBackground(int x, int y, const TCODColor& color);
	void addBloodstain(int x, int y, const TCODColor& color);
	void render(int xshift = 0, int yshift = 0) const;
	bool isInFov(int x, int y) const;
	bool isExplored(int x, int y) const;
	void computeFov();
	void shift(int x, int y); // shift tiles a certain amount
	int getWidth() const { return width; }
	int getHeight() const { return height; }
	void setTile(int x, int y, const Tile& tile);
	Tile getTile(int x, int y) const;
	
 private:
	Tile** tiles;
	TCODMap* map;
	std::deque<Room> rooms;
	int width, height;
	int* regions;

	void init();
	void initTile(int x, int y, const Tile& tile);
	void digRoom(Room room);
	void digTunnel(Room from, Room to);
	int nbs(int x, int y, int ch) const;
	void dig(int x, int y, int w, int h);
        void addItem(int x, int y);
	MonsterKind chooseMonsterKind();
	void spawnMonster(int x, int y, MonsterKind kind);
	void spawnHorde(int x, int y);
};

#endif
