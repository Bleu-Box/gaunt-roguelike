// the game's GUI
#ifndef GUI_H
#define GUI_H

#include <vector>
#include <string>
#include "lib/libtcod.hpp"

class Menu {
 public:
	enum MenuItemCode {NONE, NEW_GAME, CONTINUE, EXIT};
	
	~Menu();
	
	void clear();
	void addItem(MenuItemCode code, std::string label);
	MenuItemCode pick();
	
 protected:
	struct MenuItem {
		MenuItemCode code;
		std::string label;
	};

	std::vector<MenuItem*> items;
};

class Gui {
 public:
	Menu menu;
	
	Gui();
	~Gui();

	void render();
	void message(std::string text);
	void clearMessages();
	int getDataConsoleWidth() const { return dataConsole->getWidth(); }
	int getDataConsoleHeight() const { return dataConsole->getHeight(); }
	int getMessageConsoleWidth() const { return messageConsole->getWidth(); }
	int getMessageConsoleHeight() const { return messageConsole->getHeight(); }
	
 protected:
	TCODConsole* dataConsole;
	TCODConsole* messageConsole;
	// this stores all the message content to be shown next turn
	std::vector<std::string> messages;
	
	void renderBar(int x, int y, int width, std::string name,
		       float value, float maxValue, const TCODColor& barColor,
		       const TCODColor& backColor);
};

#endif
