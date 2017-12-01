// This contains code for the actor class definition

class Actor {
 public:
	int x, y;
	int ch; // using an int rather than a char allows you to use more than 256 different chars
	const char* name;
	bool blocks; // whether the actor can be walked upon
	Attacker* attacker; // a thing that deals damage
	Destructible* destructible; // smth that can be hurt
	Ai* ai; // smth that's self-updating
	Pickable* pickable; // something that can be picked and used
	Container* container; // something that can contain other actors
	TCODColor color;

	Actor(int x, int y, int ch, const char* name, const TCODColor& color);
	~Actor();
	
	void render(int xshift = 0, int yshift = 0) const;
	void update();
	bool moveOrAttack(int x, int y);
	float getDistance(int cx, int cy) const;
};
