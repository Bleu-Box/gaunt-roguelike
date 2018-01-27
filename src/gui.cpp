#include <algorithm>
#include <string.h>
#include "main.h"
#include "gui.h"
#include "map.h"
#include "actor.h"
#include "destructible.h"
#include "attacker.h"
#include "ai.h"
#include "effect.h"

static const int BAR_WIDTH = 15;
static const int DATA_CONSOLE_WIDTH = BAR_WIDTH*2;
static const int MSG_X = 1;

Gui::Gui() {
	dataConsole = new TCODConsole(BAR_WIDTH*2, engine.getScreenHeight());
	messageConsole = new TCODConsole(engine.getScreenWidth()-BAR_WIDTH*2, 3);
}

Gui::~Gui() {
        clearMessages();
	delete dataConsole;
	delete messageConsole;
}

void Gui::clearMessages() {
	messages.clear();
}

void Gui::render() {
	dataConsole->setDefaultBackground(TCODColor::black);
	dataConsole->clear();
	// print stats
	dataConsole->setDefaultForeground(TCODColor::white);
	dataConsole->print(0, 0, "Level %i", engine.getLevel());
	dataConsole->print(1, 1, "Defense: %.1f", engine.player->destructible->getDefense());
	dataConsole->print(1, 2, "Regen: %.1f", engine.player->destructible->getRegen());
	dataConsole->print(1, 3, "Acc: %.1f", engine.player->attacker->getAccuracy());
	dataConsole->print(1, 4, "Dmg: %.1f", engine.player->attacker->getPower());
	dataConsole->print(1, 5, "Stealth: %.1f", dynamic_cast<PlayerAi*>(engine.player->ai)->stealth);

	// print the health of visible actors
	// get visible actors and sort them by proximity to player
	std::vector<Actor*> visibleActors(engine.actors.size());
	auto it = std::copy_if(engine.actors.begin(), engine.actors.end(), visibleActors.begin(), [](Actor* a) {
			return a->destructible && !a->destructible->isDead()
			         && engine.map->isInFov(a->x, a->y);
		});
	visibleActors.resize(std::distance(visibleActors.begin(), it));
	std::sort(visibleActors.begin(), visibleActors.end(), [](Actor* a, Actor* b) {
			return a->getDistance(engine.player->x, engine.player->y)
				< b->getDistance(engine.player->x, engine.player->y);
		});

	int bar_y = 0;
	for(Actor* actor : visibleActors) {
		int health_y = 7+bar_y;
		renderBar(0, health_y, BAR_WIDTH, actor->name,
			  actor->destructible->getHp(), 
			  actor->destructible->getMaxHp(), 
			  TCODColor::darkAzure, TCODColor::darkestAzure);
		// print out any effects on the actor
		int offset = 0;
		for(Effect* effect : actor->effects) {
			renderBar(0, health_y+offset+1, BAR_WIDTH, effect->getName(),
				  effect->getDuration(), effect->getStartDuration(),
				  TCODColor::darkRed, TCODColor::darkestRed);
		        offset++;
		}
		
		bar_y += 2+offset;
	}

	// print message log	
	messageConsole->setDefaultBackground(TCODColor::black);
        messageConsole->clear();
	
	int i = 0;
	for(std::string msg : messages)
		messageConsole->print(MSG_X, i++, msg.c_str());
	
	// blit the GUI console on the root console
	TCODConsole::blit(messageConsole, 0, 0, messageConsole->getWidth(), messageConsole->getHeight(), 
			  TCODConsole::root, MSG_X, engine.getScreenHeight()-getMessageConsoleHeight());

	TCODConsole::blit(dataConsole, 0, 0, dataConsole->getWidth(), dataConsole->getHeight(), 
			  TCODConsole::root, engine.getScreenWidth()-BAR_WIDTH*1.2, 1);
}

// shows health bar
void Gui::renderBar(int x, int y, int width, std::string name, float value,
		    float maxValue, const TCODColor& barColor, const TCODColor& backColor) {
	dataConsole->setDefaultBackground(backColor);
	dataConsole->rect(x, y, width, 1, false, TCOD_BKGND_SET);
	
	int barWidth = (int) (value/maxValue*width);
	if(barWidth > 0) {
		// draw the bar
		dataConsole->setDefaultBackground(barColor);
		dataConsole->rect(x, y, barWidth, 1, false, TCOD_BKGND_SET);
	}

	// print text on top of the bar
	dataConsole->setDefaultForeground(TCODColor::white);
	dataConsole->printEx(x+width/2, y, TCOD_BKGND_NONE, TCOD_CENTER, "%s", name.c_str());
}

// add another message unless there's 2 or more already
void Gui::message(std::string text) {
	if(messages.size() < 2)
		messages.push_back(text);
}

Menu::~Menu() {
	clear();
}

void Menu::clear() {
	items.clear();
}

void Menu::addItem(MenuItemCode code, std::string label) {
   MenuItem* item = new MenuItem();
   item->code = code;
   item->label = label;
   items.push_back(item);
}

Menu::MenuItemCode Menu::pick() {
        static TCODImage img("./assets/images/background.png");
	static const int BKGND_IMG_SIZE = engine.getScreenWidth() <= engine.getScreenHeight()?
	        engine.getScreenWidth() : engine.getScreenHeight();
	
	int selectedItem = 0;
	while(!TCODConsole::isWindowClosed()) {
		img.blitRect(TCODConsole::root, 0, 0, BKGND_IMG_SIZE, BKGND_IMG_SIZE);
		
		int currentItem = 0;
		for(MenuItem* item : items) {
			if(currentItem == selectedItem) {
				TCODConsole::root->setDefaultForeground(TCODColor::white);
			} else {
				TCODConsole::root->setDefaultForeground(TCODColor::lightGrey);
			}
		   
			TCODConsole::root->print(30, 10+currentItem*3, item->label.c_str());
			currentItem++;
		}

		TCODConsole::flush();

		// check key presses
		TCOD_key_t key;
		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS, &key, NULL);

		// controls: UP/DOWN = navigate list; ENTER = select item
		switch(key.vk) {
		case TCODK_UP: 
			selectedItem--; 
			if(selectedItem < 0) selectedItem = items.size()-1; // wrap around list
			break;
		case TCODK_DOWN: 
			selectedItem = (selectedItem+1)%items.size(); // wrap around list
			break;
		case TCODK_ENTER: return items.at(selectedItem)->code;
		default: break;
		}
	}

	return NONE;
}
