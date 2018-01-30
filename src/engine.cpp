#include <algorithm>
#include "main.h"
#include "engine.h"
#include "actor.h"
#include "pickable.h"
#include "container.h"
#include "destructible.h"
#include "ai.h"
#include "attacker.h"
#include "map.h"
#include "gui.h"

Engine::Engine(int screenWidth, int screenHeight): 
	gameStatus(STARTUP), player(NULL), map(NULL), renderMap(true), fovRadius(5), turnCount(0),
	screenWidth(screenWidth), screenHeight(screenHeight), level(1) {
	// set font
	TCODConsole::setCustomFont("./assets/fonts/consolas18x18_gs_tc.png",
				   TCOD_FONT_LAYOUT_TCOD|TCOD_FONT_TYPE_GREYSCALE);
	TCODConsole::initRoot(screenWidth, screenHeight, "Gaunt", false);
	gui = new Gui();
}
 
Engine::~Engine() {
	terminate();
	delete gui;
}
 
void Engine::init() {
	// initialize random colors for potions
	Potion::assignColors();
	// init player and related things for it
	player = new Actor(100, 100, '@', "Player", TCODColor::white);
	player->destructible = new PlayerDestructible(20, 5, 0.5);
        #if DEBUG_MODE == 1
		player->destructible->invincible = true;
        #endif
	player->attacker = new Attacker(5, 50, "whacks");
	player->ai = new PlayerAi(2);
	player->container = new Container(26); // create 26 inventory slots for player - 1 for each letter of the alphabet
	actors.push_back(player);

	// the stairs -- even though they begin w/ a location at (0, 0), the map will put them somewhere else
	stairs = new Actor(0, 0, '>', "Stairs", TCODColor::yellow);
	stairs->blocks = false;
	stairs->resistsMagic = true;
	actors.push_back(stairs);
	// TODO: find a better way to make map size based on Gui panel sizes
	//map = new Map(screenWidth-18, screenHeight-7);
	map = new Map(screenWidth-gui->getDataConsoleWidth(), screenHeight-gui->getMessageConsoleHeight());
	
	gui->message("Welcome to the Dungeons of Doom!");
	gui->message("You should try getting the Amulet of Yendor while you're down here.");
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
	actors.clear();
	if(map) delete map;
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
		gui->clearMessages();
		turnCount++;
		
		for(Actor* actor : actors) {
			if(actor != player) actor->update();
		}

		actors.erase(std::remove_if(actors.begin(), actors.end(),
					    [](Actor* a) {
						    return a->destructible && a->destructible->isDead();
					    }), actors.end());
		actors.insert(actors.end(), spawnQueue.begin(), spawnQueue.end());
		spawnQueue.clear();
	}
}

void Engine::render() {	
	TCODConsole::root->clear();
	if(renderMap) map->render();

	for(Actor* actor : actors) {
		if(map->isInFov(actor->x, actor->y)) {
			if(renderMap || actor == player) actor->render();
		        gui->render();
		}
	}
}

// make an actor be displayed in the background (so others appear to walk on top of it) - useful for corpses
void Engine::sendToBack(Actor* actor) {
        auto i = std::find(actors.begin(), actors.end(), actor);
	actors.erase(i);
	actors.insert(actors.begin(), actor);
}

/*
Actor* Engine::getClosestMonster(int x, int y, float range) const {
	Actor* closest = NULL;
	float bestDist = 1E6f; // starts at impossibly high value
	
	for(Actor* actor : actors) {
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
*/

Actor* Engine::getActorAt(int x, int y) {
	auto it = std::find_if(actors.begin(), actors.end(), [x, y](Actor* a) {
			return a->x == x && a->y == y;
		});

	return *it;
}

bool Engine::pickTile(int* x, int* y, float maxRange) {
	while(!TCODConsole::isWindowClosed()) {
		render();

		for(int cx = 0; cx < map->getWidth(); cx++) {
			for(int cy = 0; cy < map->getHeight(); cy++) {
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
	gui->message("You descend deeper...");
	// get rid of all the things from previous level
	delete map;
	// make sure to keep player and stairs though
	actors.clear();
	actors.push_back(player);
	actors.push_back(stairs);
	
	map = new Map(screenWidth-18, screenHeight-7);
	
	gameStatus = STARTUP;
	map->computeFov();
}
