// contains most of the #includes the files needed
// TODO: don't use this to put all your dependencies in
#ifndef MAIN_H
#define MAIN_H
// debug mode changes things like player health so playing through is easier
#define DEBUG_MODE 0

#include "lib/libtcod.hpp"
#include "tiles.h"
class Actor;
class Map;
class Gui;
#include "engine.h"
#include "effect.h"
#include "destructible.h"
#include "attacker.h"
#include "ai.h"
#include "pickable.h"
#include "container.h"
#include "spreadable.h"
#include "actor.h"
#include "map.h"
#include "gui.h"

#endif
