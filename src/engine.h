// the Engine handles the game - it's more convenient than putting everything in main()
#include <vector>

class Engine {
 public:
	enum GameStatus {
		STARTUP, // 1st frame of game
		VICTORY, // the player won
		DEFEAT, // the player died
		IDLE, // no new turn - just the same screen as before
		NEW_TURN // update things
	} gameStatus;

	std::vector<Actor*> actors;
	Actor* player;
	Actor* stairs;
	Map* map;
	Gui* gui;
	bool renderMap; // this controls whether we show map and monsters (turn it off for 'blindness')
	
	Engine(int screenWidth, int screenHeight);
	~Engine();

	void sendToBack(Actor* actor);
	Actor* getClosestMonster(int x, int y, float range) const;
	bool pickTile(int* x, int* y, float maxRange = 0.0f);
	void update();
	void render();
	void init();
	void load();
	void terminate();
	void nextLevel();
	void spawnActor(Actor* a) { spawnQueue.push_back(a); }
	int getFovRadius() const { return fovRadius; }
	int getScreenWidth() const { return screenWidth; }
	int getScreenHeight() const { return screenHeight; }
	TCOD_key_t getLastKey() const { return lastKey; }
	TCOD_mouse_t getMouse() const { return mouse; }
	int getLevel() const { return level; }
	int getTurnCount() const { return turnCount; }
	
 private:
	std::vector<Actor*> spawnQueue;
	int fovRadius;
	int turnCount;
	int screenWidth;
	int screenHeight;
	TCOD_key_t lastKey;
	TCOD_mouse_t mouse;
	int level;
};

extern Engine engine;
