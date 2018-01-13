// definition for Map
#ifndef MAP_H
#define MAP_H

class Map {
 public:
	enum MonsterKind {RAT, MUSHROOM, SLIME, REDCAP};
	
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
	void setTile(int x, int y, const Tile& tile);
	Tile getTile(int x, int y) const;
	
 private:
	Tile** tiles;
	TCODMap* map;
	int width, height;

	void init();
	void spreadTile(int x, int y, int count, const Tile& tile);
	float getWalkableCoverage();
        void addMonster(int x, int y);
        void addItem(int x, int y);
	void generateMap();
	int nbs(int x, int y, int ch) const;
	void floodFill(int x, int y);
        void removeDisjointRooms();
	void place(Actor* actor);
	MonsterKind chooseMonsterKind();
	void spawnMonster(int x, int y, MonsterKind kind);
	void spawnHorde(int x, int y);
};

#endif
