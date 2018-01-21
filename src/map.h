// definition for Map
#ifndef MAP_H
#define MAP_H

#include <vector>

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
	void openDoor(int x, int y);
	void closeDoor(int x, int y);
	void render() const;
	bool isInFov(int x, int y) const;
	bool isExplored(int x, int y) const;
	void computeFov();
	void shift(int x, int y); // shift tiles a certain amount
	int getWidth() const { return width; }
	int getHeight() const { return height; }
	void setTile(int x, int y, const Tile& tile);
	Tile getTile(int x, int y) const;
	TCODPath findPath(int x1, int y1, int x2, int y2, float diagCost=1.41f);
	
 private:
	Tile** tiles;
	TCODMap* map;
	std::vector<Room> rooms;
	int width, height;
	int* regions;

	void init();
	void initTile(int x, int y, const Tile& tile);
	void makeRooms(int count);
	void connectRooms();
	void digRoom(Room room);
	void digTunnel(Room& from, Room& to);
	void dig(int x, int y, int w, int h);
	void spreadTile(int x, int y, int count, const Tile& tile);
        void addItem(int x, int y);
	MonsterKind chooseMonsterKind();
	void spawnMonster(int x, int y, MonsterKind kind);
	void spawnHorde(int x, int y);
};

#endif
