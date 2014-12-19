#include "empire.h"

extern _loc capital;
bool is_good_parking_unit(_unit *u);
bool no_better_parking_alternatives(_unit *u);

void check_blocking_base(_unit *u)
{
  _loc d;
  if (friendly_unit_maker_here(u->tilex, u->tiley, u->color) == true)
  {
		if (!((unitstats[u->type].movetype == M_LANDER) && (unitstats[u->type].loadcapacity > 0)))
    {  //unless the unit's a lander, move off the unit-producing building
			d = a_free_space(u, u->tilex, u->tiley);
      if (d.x != -1)
      {
        action[0].type = action_move;
        action[0].loc = d;
				action[0].destination = d;
      }
    }
    else
    {  //if it's a lander, move to a reachable friendly beach or stay at port
      d = reachable_friendly_beach(u->color, u->number, u->tilex, u->tiley, u->move);
      if (d.x != -1)
      {
        action[0].type = action_move;
        action[0].loc = d;
				action[0].destination = d;
      }
    }
  }
}

void check_low_health(_unit *u)
{
  _loc d;
  if (action[0].type != action_none)
  {  //if it's doing something, never mind low health
    return;
  }
  if (u->health < 6.5)
  {  //if the unit has taken serious damage
    d = closest_free_building(u->color, u->number, unitstats[u->type].movetype, u->move, unitstats[u->type].basetype, u->tilex, u->tiley);
    if (d.x != -1)  //if there is a free friendly building, head for it
    {
      if ((d.x != u->tilex) || (d.y != u->tiley))
      {  //if it isn't already sitting on the best place
        action[0].type = action_move;
        action[0].loc = d;
      }
    }
    if (u->health < 5)
    {  //if the unit is very low on health, look for a merge instead
      d = unhealthy_unit_nearby(u, u->tilex, u->tiley);
      if (d.x != -1)
      {
        action[0].type = action_move;
        action[0].loc = d;
        action[1].type = action_merge;
        action[1].loc = d;
      }
    }
  }
}

void check_HQ_park(_unit *u)
{  //check if the HQ is open, if the unit is close enough to park on it, and whether it's strong enough to park on it
	if (action[0].type != action_none) return;
	
	if ((u->tilex == capital.x) && (u->tiley == capital.y))
	{  //if this unit is already parked on the HQ
		if (is_good_parking_unit(u))
		{
			action[0].type = action_standby;
			action[0].loc.x = u->tilex;
			action[0].loc.y = u->tiley;
		}
		return;
	}
	
	if (worldmap.tile[capital.x][capital.y].get_step() > u->move * 2)
	{  //unit's too far away, not worth it
		return;
	}
	
	if (any_unit_here(capital.x, capital.y) == -1)
	{  //if the AI's HQ is wide open
		if ((unitstats[u->type].basetype != SEA) && is_good_parking_unit(u))
		{  //if this unit is suitable for parking on the HQ
			action[0].type = action_move;
			action[0].destination.x = capital.x;
			action[0].destination.y = capital.y;
			action[0].loc = closest_move_to_destination(u->color, u->number, unitstats[u->type].movetype, u->move, u->tilex, u->tiley, action[0].destination, 0);
			action[1].type = action_standby;
			action[1].loc.x = capital.x;
			action[1].loc.y = capital.y;
		}
		else if (unitstats[u->type].basetype != SEA)
		{  //if it isn't a good parking unit
			if (no_better_parking_alternatives(u))
			{
				action[0].type = action_move;
				action[0].destination.x = capital.x;
				action[0].destination.y = capital.y;
				action[0].loc = closest_move_to_destination(u->color, u->number, unitstats[u->type].movetype, u->move, u->tilex, u->tiley, action[0].destination, 0);
				action[1].type = action_standby;
				action[1].loc.x = capital.x;
				action[1].loc.y = capital.y;
			}
		}
	}
}

bool is_good_parking_unit(_unit *u)
{
	if (u->resist[0] + u->resist[1] + u->resist[2] > 80) return true;
	return false;
}

bool no_better_parking_alternatives(_unit *u)
{
	int beat_this = u->resist[0] + u->resist[1] + u->resist[2];
	int i = 0;
	_unit *v;
	
	worldmap.backup_pathmap();
	while (i < 50)
	{
		if (i != u->number)
		{
			v = &player[u->color].unit[i];
			if ((v->exists == 1) && v->canmove && (v->resist[0] + v->resist[1] + v->resist[2] > beat_this))
			{
				worldmap.create_limited_pathmap(v->color, unitstats[v->type].movetype, v->tilex, v->tiley, v->move * 2);
				if (worldmap.tile[capital.x][capital.y].get_step() < v->move * 2)
				{
					worldmap.restore_pathmap();
					return false;
				}
			}
		}
		i++;
	}
	worldmap.restore_pathmap();
	return true;
}
