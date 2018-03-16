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
static const int DATA_CONSOLE_WIDTH = BAR_WIDTH*1.2;
static const int MSG_X = 1;

Gui::Gui() {
	dataConsole = new TCODConsole(BAR_WIDTH*2, engine.getScreenHeight());
	messageConsole = new TCODConsole(engine.getScreenWidth()-BAR_WIDTH*2, 3);
}

Gui::~Gui() {
        messages.clear();
	delete dataConsole;
	delete messageConsole;
}

void Gui::render() {
	dataConsole->setDefaultBackground(TCODColor::black);
	dataConsole->clear();
 
	int data_y = 0;
	#if DEBUG_MODE == 1
	dataConsole->setDefaultForeground(TCODColor::green);
	dataConsole->print(0, data_y++, "DEBUG MODE");
	#endif
	// print stats
	dataConsole->setDefaultForeground(TCODColor::white);
	dataConsole->print(0, data_y++, "Level %i", engine.getLevel());
	dataConsole->print(0, data_y++, "Turns: %i", engine.getTurnCount());
	dataConsole->print(1, data_y++, "Defense: %.1f", engine.player->destructible->getDefense());
	dataConsole->print(1, data_y++, "Regen: %.1f", engine.player->destructible->getRegen());
	dataConsole->print(1, data_y++, "Stren: %.1f", engine.player->stren);
	dataConsole->print(1, data_y++, "Acc: %.1f", engine.player->attacker->getAccuracy(engine.player));
	dataConsole->print(1, data_y++, "Dmg: %.1f", engine.player->attacker->getPower());
	dataConsole->print(1, data_y++, "Stealth: %i",
			   dynamic_cast<PlayerAi*>(engine.player->ai)->stealth);

	// print the health of visible actors
	// get visible actors and sort them by proximity to player
	std::vector<Actor*> visibleActors(engine.actors.size());
	auto it = std::copy_if(engine.actors.begin(), engine.actors.end(), visibleActors.begin(), [](Actor* a) {
			return (!a->destructible || (a->destructible && !a->destructible->isDead()))
			&& engine.map->isInFov(a->x, a->y) && (engine.renderMap || a == engine.player);
		});
	visibleActors.resize(std::distance(visibleActors.begin(), it));
	// now sort actors based on distance to player and whether or not they're a monster
	std::sort(visibleActors.begin(), visibleActors.end(), [](Actor* a, Actor* b) {
			return (a->getDistance(engine.player->x, engine.player->y)
				< b->getDistance(engine.player->x, engine.player->y))
				|| (a->destructible && !b->destructible);
		});

	int bar_y = 1;
	for(Actor* actor : visibleActors) {
		// If the actor has a destructible (meaning it's some sort of creature)
		// then print out health and effects. Otherwise, it's an item or stairs
		// or something else, so just show its name.
		if(actor->destructible) {
			int health_y = data_y+bar_y;
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
		} else {
			// actor's name may need to be truncated to fit on data console
			std::string name = (actor->name.length() > DATA_CONSOLE_WIDTH-2)?
				actor->name.substr(0, DATA_CONSOLE_WIDTH-6)+"..." : actor->name;
			// print actor's char/color and then name
		        dataConsole->putChar(0, data_y+bar_y, actor->ch);
		        dataConsole->setCharForeground(0, data_y+bar_y, actor->color);
			dataConsole->print(2, data_y+bar_y, name.c_str());
			bar_y++;
		}		
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
			  TCODConsole::root, engine.getScreenWidth()-DATA_CONSOLE_WIDTH, 1);
}

// show a final closing message, i.e. for death or victory
void Gui::renderFinalMessage(std::string msg) {
	messageConsole->setDefaultBackground(TCODColor::black);
	messageConsole->setDefaultForeground(TCODColor::white);
        messageConsole->clear();
	messageConsole->print(MSG_X, 0, msg.c_str());
	TCODConsole::blit(messageConsole, 0, 0, messageConsole->getWidth(), messageConsole->getHeight(), 
			  TCODConsole::root, engine.getScreenWidth()/2-messageConsole->getWidth()/2,
			  engine.getScreenHeight()/2-messageConsole->getHeight());
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

// add another message
void Gui::message(std::string text) {
	// if there are too many messages, remove the first message to make room
	if(messages.size() >= 2) {
		std::string temp = messages.back();
		messages.clear();
		messages.push_back(temp);
	}
	
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
	
	int selectedItem = 0;
	while(!TCODConsole::isWindowClosed()) {
		img.blit2x(TCODConsole::root, 0, 0, 300, 100);

		int con_x = 0;
		int con_y = 0;
		int con_w = 40;
		int con_h = 20;
		TCODConsole* options = new TCODConsole(con_w, con_h);
	        options->printFrame(con_x, con_y, con_w, con_h, true,
					      TCOD_BKGND_DEFAULT, "GAUNT");		
		int currentItem = 0;
		for(MenuItem* item : items) {
			if(currentItem == selectedItem) {
			        options->setDefaultForeground(TCODColor::white);
			} else {
			        options->setDefaultForeground(TCODColor::grey);
			}

		        options->print(con_x+2, (con_y+2)+currentItem*2, item->label.c_str());
			currentItem++;
		}

		TCODConsole::blit(options, 0, 0, 0, 0, TCODConsole::root, con_x, con_y);
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
