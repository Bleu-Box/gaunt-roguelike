#include <algorithm>
#include "main.h"

Container::Container(int size): size(size) {}

Container::Container(const Container& other) {
        inventory = other.inventory;
        size = other.getSize();
}

Container::~Container() {
	inventory.clear();
}

Container& Container::operator=(const Container& rhs) {
	Container temp(rhs);
	std::swap(inventory, temp.inventory);
	std::swap(size, temp.size);
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
	auto it = std::remove(inventory.begin(), inventory.end(), actor);
	inventory.erase(it, inventory.end());
}
