#include <algorithm>
#include "main.h"

static const int PANEL_HEIGHT = 7;
static const int BAR_WIDTH = 15;
static const int MSG_X = 1;
static const int MSG_HEIGHT = PANEL_HEIGHT-1;

Gui::Gui() {
	dataConsole = new TCODConsole(BAR_WIDTH*2, engine.getScreenHeight());
	messageConsole = new TCODConsole(engine.getScreenWidth(), PANEL_HEIGHT);
}

Gui::~Gui() {
	delete dataConsole;
	delete messageConsole;
        clear();
}

void Gui::clear() {
	log.clear();
}

Gui::Message::Message(std::string text, const TCODColor& color):
	text(text), color(color) {}

Gui::Message::~Message() {}

void Gui::render() {
	dataConsole->setDefaultBackground(TCODColor::black);
	dataConsole->clear();
	// print stats
	dataConsole->setDefaultForeground(TCODColor::white);
	dataConsole->print(0, 0, "Level %i", engine.getLevel());
	dataConsole->print(1, 1, "Defense: %.1f", engine.player->destructible->getDefense());
	dataConsole->print(1, 2, "Acc: %.1f", engine.player->attacker->getAccuracy());
	dataConsole->print(1, 3, "Dmg: %.1f", engine.player->attacker->getPower());

	// print effects acting on player
	int bar_y = 0;
	for(Effect* effect : engine.player->effects) {
		renderBar(0, 5+bar_y, BAR_WIDTH, effect->getName(),
			  effect->getDuration(), effect->getStartDuration(),
			  TCODColor::darkRed, TCODColor::darkestRed);
		bar_y += 2;
	}

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

	for(Actor* actor : visibleActors) {
		renderBar(0, 5+bar_y, BAR_WIDTH, actor->getName(),
			  actor->destructible->getHp(), 
			  actor->destructible->getMaxHp(), 
			  TCODColor::darkAzure, TCODColor::darkestAzure);
		bar_y += 2;
	}

	messageConsole->setDefaultBackground(TCODColor::black);
        messageConsole->clear();
	
	// the message log
	int msg_y = 1;
	float colorCoef = 0.4f; // modifier to make older lines appear to fade
	for(Message* message : log) {
		messageConsole->setDefaultForeground(message->color*colorCoef);
		messageConsole->print(MSG_X, msg_y, message->text.c_str());
		msg_y++;
		if(colorCoef < 1.0f) colorCoef+=0.3f;
	}	

	// renderMouseLook();
	
	// blit the GUI console on the root console
	TCODConsole::blit(messageConsole, 0, 0, 100, PANEL_HEIGHT, 
			  TCODConsole::root, MSG_X, engine.getScreenHeight()-PANEL_HEIGHT);

	TCODConsole::blit(dataConsole, 0, 0, 100, engine.getScreenHeight(), 
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

void Gui::renderMouseLook() {
	// don't render if mouse isn't in FOV
	if(!engine.map->isInFov(engine.getMouse().cx, engine.getMouse().cy)) return;

	// find all the actors in the cell and list them in a string
	char buf[128] = "";
	bool first = true;
	for(Actor* actor : engine.actors) {
		// find actors under the mouse cursor
		if(actor->x == engine.getMouse().cx && actor->y == engine.getMouse().cy) {
			if(!first) {
				strcat(buf, ", ");
			} else {
				first = false;
			}
			strcat(buf, actor->getName().c_str());
		}
	}

	// display the list of actors under the mouse cursor
        messageConsole->setDefaultForeground(TCODColor::lightGrey);
	messageConsole->print(1, 0, buf);
}

void Gui::message(MessageType mType, std::string text) {
	TCODColor color;	
	// make the message a certain color based on its type
	switch(mType) {
	case OBSERVE:
		color = TCODColor::lightGrey;
		break;
	case ACTION:
		color = TCODColor::darkAzure;
		break;
	case ATTACK:
		color = TCODColor::darkRed;
		break;
	}

	std::vector<std::string> strings;
	std::string::size_type pos = 0;
	std::string::size_type prev = 0;
	while((pos = text.find('\n', prev)) != std::string::npos) {
		strings.push_back(text.substr(prev, pos-prev));
		prev = pos+1;
	}
	strings.push_back(text.substr(prev));

	for(std::string str : strings) {
		// NOTE: if speed becomes a concern, use a faster strategy for this
		if(log.size() > MSG_HEIGHT) log.erase(log.begin());
		Message* msg = new Message(str, color);
		log.push_back(msg);
	}
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
        static TCODImage img("C:/Users/Paul/Documents/Benjamin/C++ Stuff/Roguelikes/Gaunt/assets/images/background.png");
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
		   
			TCODConsole::root->print(10, 10+currentItem*3, item->label.c_str());
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
