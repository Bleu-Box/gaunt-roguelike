// the Spreadable class - for things that spread (like fire and gas)

class Spreadable {
 public:
	int lifespan;

        Spreadable(int lifespan);

	void spread(Actor* owner);

 private:
	void addActor(Actor* owner, int x, int y);
};
