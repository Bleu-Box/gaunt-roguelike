// the Engine handles the game - it's more convenient than putting everything in main()

class Engine {
 public:
	enum GameStatus {
		STARTUP, // 1st frame of game
		VICTORY, // the player won
		DEFEAT, // the player died
		IDLE, // no new turn - just the same screen as before
		NEW_TURN // update things
	} gameStatus;
	
	TCODList<Actor*> actors;
	Actor* player;
	Actor* stairs;
	Map* map;
	int fovRadius; // you could change radius for a potion of blindess/confusion
	int screenWidth;
	int screenHeight;
	Gui* gui;
	TCOD_key_t lastKey;
	TCOD_mouse_t mouse;
	int level;

	Engine(int screenWidth, int screenHeight);
	~Engine();

	void update();
	void render();
	void sendToBack(Actor* actor);
	Actor* getClosestMonster(int x, int y, float range) const;
	bool pickTile(int* x, int* y, float maxRange = 0.0f);
	void init();
	void load();
	void terminate();
	void nextLevel();
};

// This tells the compiler that there's a global var named engine somewhere in a .cpp file
extern Engine engine;
