#include <stdio.h>
#include <stdarg.h>
#include "main.h"

static const int PANEL_HEIGHT = 7;
static const int BAR_WIDTH = 20;
static const int MSG_X = BAR_WIDTH+2;
static const int MSG_HEIGHT = PANEL_HEIGHT-1;

Gui::Gui() {
	console = new TCODConsole(engine.screenWidth, PANEL_HEIGHT);
}

Gui::~Gui() {
	delete console;
        clear();
}

void Gui::clear() {
	log.clearAndDelete();
}

Gui::Message::Message(const char* text, const TCODColor& color):
	text(strdup(text)), color(color) {}

Gui::Message::~Message() {
	free(text);
}

void Gui::render() {
	console->setDefaultBackground(TCODColor::black);
	console->clear();

	// player's health bar
	renderBar(1, 1, BAR_WIDTH, "Health", engine.player->destructible->hp, 
		  engine.player->destructible->maxHp, 
		  TCODColor::desaturatedHan, TCODColor::darkestHan);

	// the message log
	int y = 1;
	float colorCoef = 0.4f; // modifier to make older lines appear to fade
	for(Message** iterator = log.begin(); iterator != log.end(); iterator++) {
		Message* message = *iterator;
		console->setDefaultForeground(message->color*colorCoef);
		console->print(MSG_X, y, message->text);
		y++;
		if(colorCoef < 1.0f) colorCoef+=0.3f;
	}	

	renderMouseLook();

	// show dungeon level
	console->setDefaultForeground(TCODColor::white);
	console->print(3, 3, "Level %d", engine.level);
	
	// blit the GUI console on the root console
	TCODConsole::blit(console, 0, 0, engine.screenWidth, PANEL_HEIGHT, 
			  TCODConsole::root, 0, engine.screenHeight-PANEL_HEIGHT);
}

void Gui::renderBar(int x, int y, int width, const char* name, float value,
		    float maxValue, const TCODColor& barColor, const TCODColor& backColor) {
	console->setDefaultBackground(backColor);
	console->rect(x, y, width, 1, false, TCOD_BKGND_SET);

	int barWidth = (int) (value/maxValue*width);
	if(barWidth > 0) {
		// draw the bar
		console->setDefaultBackground(barColor);
		console->rect(x, y, barWidth, 1, false, TCOD_BKGND_SET);
	}

	// print text on top of the bar
	console->setDefaultForeground(TCODColor::white);
	console->printEx(x+width/2, y, TCOD_BKGND_NONE, TCOD_CENTER, "%s", name);
}

void Gui::renderMouseLook() {
	// don't render if mouse isn't in FOV
	if(!engine.map->isInFov(engine.mouse.cx, engine.mouse.cy)) return;

	// find all the actors in the cell and list them in a string
	char buf[128] = "";
	bool first = true;
	for(Actor** iter = engine.actors.begin(); iter != engine.actors.end(); iter++) {
		Actor* actor = *iter;
		// find actors under the mouse cursor
		if(actor->x == engine.mouse.cx && actor->y == engine.mouse.cy) {
			if(!first) {
				strcat(buf, ", ");
			} else {
				first = false;
			}
			strcat(buf, actor->name);
		}
	}

	// display the list of actors under the mouse cursor
	console->setDefaultForeground(TCODColor::lightGrey);
	console->print(1, 0, buf);
}

// this is really confusing
void Gui::message(MessageType mType, const char* text, ...) {
	// this is all the weird stuff we need to do to get text from the varargs
	// it uses stuff from the <stdarg.h> header
	va_list ap;
	char buf[128];
	va_start(ap, text);
	vsprintf(buf, text, ap);
	va_end(ap);

	char* lineBegin = buf;
	char* lineEnd;

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
	
	do {
		// remove oldest message if there's not enough room for new one yet		
		if(log.size() == MSG_HEIGHT) {
			Message* toRemove =  log.get(0);
			log.remove(toRemove);
			delete toRemove;
		}
		
		// detect end of the line
		lineEnd = strchr(lineBegin,'\n');
		if(lineEnd) *lineEnd = '\0';
		// add a new message to the log
		Message* msg = new Message(lineBegin, color);
		log.push(msg);

		lineBegin = lineEnd+1;
	} while(lineEnd);
}

Menu::~Menu() {
	clear();
}

void Menu::clear() {
	items.clearAndDelete();
}

void Menu::addItem(MenuItemCode code, const char* label) {
   MenuItem* item = new MenuItem();
   item->code = code;
   item->label = label;
   items.push(item);
}

Menu::MenuItemCode Menu::pick() {
        static TCODImage img("C:/Users/Paul/Documents/Benjamin/C++ Stuff/Roguelikes/Gaunt/assets/images/background.png");
	static const int BKGND_IMG_SIZE = engine.screenWidth <= engine.screenHeight? engine.screenWidth : engine.screenHeight;
	
	int selectedItem = 0;
	while(!TCODConsole::isWindowClosed()) {
		img.blitRect(TCODConsole::root, 0, 0, BKGND_IMG_SIZE, BKGND_IMG_SIZE);

		int currentItem = 0;
		for (MenuItem** iterator = items.begin(); iterator != items.end(); iterator++) {
			if(currentItem == selectedItem) {
				TCODConsole::root->setDefaultForeground(TCODColor::white);
			} else {
				TCODConsole::root->setDefaultForeground(TCODColor::lightGrey);
			}
		   
			TCODConsole::root->print(10, 10+currentItem*3, (*iterator)->label);
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
		case TCODK_ENTER: return items.get(selectedItem)->code;
		default: break;
		}
	}

	return NONE;
}
