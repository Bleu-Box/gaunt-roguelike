class Container {
 public:
	int size; // max # of contents - 0 = unlimited
	TCODList<Actor*> inventory;

	Container(int size);
	~Container();

	bool add(Actor* actor);
	void remove(Actor* actor);
};
