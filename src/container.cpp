#include <algorithm>
#include "main.h"
#include "container.h"
#include "actor.h"

Container::Container(int size): size(size) {}

Container::Container(const Container& other): size(other.getSize()) {
	for(Actor* item : other.inventory) {
		Actor* myItem = new Actor(*item);
		inventory.push_back(myItem);
	}
}

Container::~Container() {
	inventory.clear();
}

Container& Container::operator=(const Container& rhs) {
	Container temp(rhs);
	int _size = temp.getSize();
	
	std::swap(size, _size);
	std::swap(inventory, temp.inventory);

	return *this;
}

// check if container isn't full
bool Container::add(Actor* actor) {
	// use static cast to get past warnings since size shouldn't be big enough to cause issues w/ comparison
        if(size > 0 && static_cast<int>(inventory.size()) >= size) return false;
	inventory.push_back(actor);
	return true;
}

void Container::remove(Actor* actor) {
	std::vector<Actor*> newInventory;
	for(Actor* a : inventory) {
		if(a != actor) newInventory.push_back(a);
	}
	inventory = newInventory;
}
