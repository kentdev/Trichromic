#ifndef _PATHFINDING_H
#define _PATHFINDING_H

#include "empire.h"

const int BUILDING  = 0;
const int UNIT      = 1;

vector<_aipath> best_aipath(int plyr, int mtype, int ox, int oy, int dx, int dy);

bool check_ranged_attack_here(int plyr, int n, int x, int y);
bool friendly_unit_maker_here(int tx, int ty, int plyr);
bool enemy_building(int plyr, int x, int y, bool cap);
bool enemy_building(int plyr, int x, int y, bool cap, int non_city);
int friendly_building(int plyr, int x, int y);
_loc loc_to_attack(_unit *u, _aitarget t);
_loc closest_free_building(int plyr, int u, int mtype, int moves, int basetype, int ox, int oy);
_loc closest_move_to_enemy_building(int plyr, int u, int mtype, int moves, int ox, int oy, int capping_unit, _loc *dest);
_loc closest_move_to_enemy_unit(int plyr, int u, int mtype, int moves, int ox, int oy, _loc *dest);
_loc closest_move_to_empty_copter(int plyr, int u, int mtype, int moves, int ox, int oy, _loc *dest);
_loc closest_move_to_available_lander(int plyr, int u, int mtype, int moves, int ox, int oy, _loc *dest);
_loc closest_move_to_destination(int plyr, int u, int mtype, int moves, int ox, int oy, _loc dest, int offset);
_loc a_free_space(_unit *u, int tx, int ty);
_loc empty_APC_nearby(_unit *u, int move);
_loc best_copter_adjacent_space(int tx, int ty);
_loc empty_adjacent_space(int tx, int ty);
_loc nearest_enemy_building(int plyr, int tx, int ty);
_loc unhealthy_unit_nearby(_unit *u, int tx, int ty);
_loc closest_beach_for_lander(int plyr, int tx, int ty);
_loc closest_beach_for_lander(int plyr, int tx, int ty, _unit *thelander);
_loc closest_enemy_beach(int plyr, int tx, int ty, bool mountain_climber);
_loc reachable_friendly_beach(int plyr, int u, int ox, int oy, int moves);
_loc reachable_enemy_building(int plyr, int mtype, int ox, int oy, bool cap);
_loc reachable_enemy_building(int plyr, int mtype, int ox, int oy, bool cap, int non_city);
_loc reachable_enemy_building_recursive(int plyr, int mtype, int ox, int oy, bool cap, int non_city, int moves);
_loc reachable_friendly_building(int plyr, int mtype, int ox, int oy);
_loc reachable_friendly_building_recursive(int plyr, int mtype, int ox, int oy, int moves);
_loc unloadable_point(int x, int y, int mountains);
_loc unloadable_point(int x, int y, int mountains, _loc ignore);
_loc loc_for_ranged_attack(_unit *u, int ox, int oy, int rmin, int rmax);
_loc closest_move(int plyr, int u, int mtype, int ox, int oy, int dx, int dy, int ignore);
_loc best_pathmap_loc(int tx, int ty);
_loc closest_teleport_to_destination(int plyr, int mtype, int destx, int desty, int maxmoves);

#endif
