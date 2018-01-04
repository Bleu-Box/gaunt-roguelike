// the Spreadable class - for things that spread (like fire and gas)
#ifndef SPREADABLE_H
#define SPREADABLE_H

class Spreadable {
 public:
	int lifespan;

        Spreadable(int lifespan);

	void spread(Actor* owner);

 private:
	void addActor(Actor* owner, int x, int y);
};

#endif
