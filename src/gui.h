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
	// messages sent to the GUI can be categorized into a few types
	// the type dictates what color the message will be
	enum MessageType {OBSERVE, ACTION, ATTACK};

	Menu menu;
	
	Gui();
	~Gui();

	void render();
	void message(MessageType, std::string text);
	void clear();
	
 protected:
	TCODConsole* dataConsole;
	TCODConsole* messageConsole;

	struct Message {
		std::string text;
		TCODColor color;
		Message(std::string text, const TCODColor& color);
		~Message();
	};
	
	std::vector<Message*> log;
	
	void renderBar(int x, int y, int width, std::string name,
		       float value, float maxValue, const TCODColor& barColor,
		       const TCODColor& backColor);

	void renderMouseLook();
};

#endif
