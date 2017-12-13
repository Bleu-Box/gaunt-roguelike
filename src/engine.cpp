#include "main.h"

Engine::Engine(int screenWidth, int screenHeight): 
	gameStatus(STARTUP), player(NULL), map(NULL), fovRadius(10),
	screenWidth(screenWidth), screenHeight(screenHeight), level(1), renderMap(true) {
	// set font 
	TCODConsole::setCustomFont("C:/Users/Paul/Documents/Benjamin/C++ Stuff/Roguelikes/Gaunt/assets/fonts/dejavu16x16_gs_tc.png", TCOD_FONT_LAYOUT_TCOD|TCOD_FONT_TYPE_GREYSCALE);
	TCODConsole::initRoot(screenWidth, screenHeight, "Gaunt", false);
	gui = new Gui();
}
 
Engine::~Engine() {
	terminate();
	delete gui;
}
 
void Engine::init() {
	// init player and related things for it
	player = new Actor(100, 100, '@', "Player", TCODColor::white);
	player->destructible = new PlayerDestructible(30, 2, "your cadaver");
	player->attacker = new Attacker(5, 50, "whacks");
	player->ai = new PlayerAi();
	player->container = new Container(26); // create 26 inventory slots for player - 1 for each letter of the alphabet
	actors.push(player);

	// the stairs -- even though they begin w/ a location at (0, 0), Map::createRoom will put them somewhere else
	stairs = new Actor(0, 0, '>', "Stairs", TCODColor::yellow);
	stairs->blocks = false;
	actors.push(stairs);
	
	TCODRandom* rand = TCODRandom::getInstance();
	map = new Map(rand->getInt(screenWidth, screenWidth*level), rand->getInt(screenHeight, screenHeight*level));
	
	gui->message(Gui::ACTION, "You enter the mouth of the cave, finding yourself in a\n claustrophobic mess of tunnels.");
	gameStatus = STARTUP;
}

void Engine::load() {      
	// set up the menu
        gui->menu.clear();
	gui->menu.addItem(Menu::NEW_GAME, "New game");
        gui->menu.addItem(Menu::EXIT, "Exit");
	
	Menu::MenuItemCode menuItem = gui->menu.pick();
	if(menuItem == Menu::EXIT || menuItem == Menu::NONE) {
		exit(0);
	} else if(menuItem == Menu::NEW_GAME) {
		// New game
	        terminate();
	        init();
	}
}
  
void Engine::terminate() {
	actors.clearAndDelete();
	if(map) delete map;
	gui->clear();
}

void Engine::update() {
	if(gameStatus == STARTUP) map->computeFov();
	gameStatus = IDLE;
	// get key and update player
	TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS, &lastKey, NULL);
	player->update();
		
	TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE, &lastKey, &mouse);
	// update actors (except for player)
	if(gameStatus == NEW_TURN) {
		for(Actor** iterator = actors.begin(); iterator != actors.end(); iterator++) {
			Actor* actor = *iterator;
			if (actor != player) actor->update();
		}
	}
}

void Engine::render() {
	// Calculate shift amounts so player remains centered.
	int xshift = screenWidth/2-player->x;
	int yshift = screenHeight/2-player->y;
	
	TCODConsole::root->clear();
	if(renderMap) map->render(xshift, yshift);

	for(Actor** iterator = actors.begin(); iterator != actors.end(); iterator++) {
		Actor* actor = *iterator;
		if(map->isInFov(actor->x, actor->y)) {
			if(renderMap || actor == player) actor->render(xshift, yshift);
		        gui->render();
		}
	}
}

// make an actor be displayed in the background (so others appear to walk on top of it) - useful for corpses
void Engine::sendToBack(Actor* actor) {
	actors.remove(actor); // remove from list
	actors.insertBefore(actor, 0); // push to front of list
}

Actor* Engine::getClosestMonster(int x, int y, float range) const {
	Actor* closest = NULL;
	float bestDist = 1E6f; // starts at impossibly high value
	
	for(Actor** iterator = actors.begin(); iterator != actors.end(); iterator++) {
		Actor* actor = *iterator;
		if(actor != player && actor->destructible && !actor->destructible->isDead()) {
			float dist = actor->getDistance(x, y);
			if(dist < bestDist && (dist <= range || range == 0.0f)) {
				bestDist = dist;
				closest = actor;
			}
		}
	}
	
	return closest;
}

bool Engine::pickTile(int* x, int* y, float maxRange) {
	while(!TCODConsole::isWindowClosed()) {
		render();

		for(int cx = 0; cx < map->width; cx++) {
			for(int cy = 0; cy < map->height; cy++) {
				// highlight tiles around player
				if(map->isInFov(cx, cy) && (maxRange == 0 || player->getDistance(cx, cy) <= maxRange)) {
					TCODColor c = TCODConsole::root->getCharBackground(cx, cy);
					c = c * 1.2f;
					TCODConsole::root->setCharBackground(cx, cy, c);
				}
			}
		}
		
		// highlight selected tile under mouse and set coordinates if left button is clicked
		TCODSystem::checkForEvent(TCOD_EVENT_KEY_PRESS|TCOD_EVENT_MOUSE, &lastKey, &mouse);
		if(map->isInFov(mouse.cx, mouse.cy) && (maxRange == 0 || player->getDistance(mouse.cx, mouse.cy) <= maxRange)) {
			TCODConsole::root->setCharBackground(mouse.cx, mouse.cy, TCODColor::white);

			if(mouse.lbutton_pressed) {
				*x = mouse.cx;
				*y = mouse.cy;
				return true;
			}
		}
		
		// exit selection mode if user right-clicks or presses a key
		if(mouse.rbutton_pressed || lastKey.vk != TCODK_NONE) return false;
		TCODConsole::flush();
	}

	return false;
}

void Engine::nextLevel() {	
	level++;
	gui->message(Gui::ACTION, "You descend deeper into the dungeons...");
	// get rid of all the things from previous level
	delete map;
	// don't delete player or stairs though
	for(Actor** iterator = actors.begin(); iterator != actors.end(); iterator++) {
		if(*iterator != player && *iterator != stairs) {
			delete *iterator;
			// update `iterator' so the loop doesn't go crazy 
			iterator = actors.remove(iterator);
		}
	}

	TCODRandom* rand = TCODRandom::getInstance();
	map = new Map(rand->getInt(screenWidth, screenWidth*level), rand->getInt(screenHeight, screenHeight*level));
	
	gameStatus = STARTUP;
	// before control yields to Engine::update(), we have to wait for the player to move
	// this means that the map won't be visible until the player moves, so we need to
	// just make it visible now
	map->computeFov();
}
