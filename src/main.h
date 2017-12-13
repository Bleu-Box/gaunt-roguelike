// this has all the #include headers for the .cpp files to use
#include "lib/libtcod.hpp"
// forward declaration of `Actor': takes care of circular dependencies btw Destructible, Attacker, and Ai and Actor
// this allows classes to use pointers/refs to Actors before the class is actually defined
class Actor;
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
#include "engine.h"
