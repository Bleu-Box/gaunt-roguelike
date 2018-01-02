#include <vector>

class Container {
 public:
	std::vector<Actor*> inventory;

	Container(int size);
	Container(const Container& other);
	~Container();

	Container& operator=(const Container& rhs);
	bool add(Actor* actor);
	void remove(Actor* actor);
	int getSize() const { return size; }

 private:
	int size;
};
