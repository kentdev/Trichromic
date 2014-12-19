#include "empire.h"

void check_APC_options(_unit *u)
{
  _loc d;
	_loc dest;
	unitprops *p = &unitstats[u->type];
  int x, y;

  if (u->load[0].loaded == 1)
  {  //if it's carrying a unit
    d = closest_move_to_enemy_building(u->color, u->number, p->movetype, u->move, u->tilex, u->tiley, 1, &dest);
    if (d.x != -1)
    {  //if it can get to an enemy building
      d = closest_move_to_destination(u->color, u->number, p->movetype, u->move, u->tilex, u->tiley, d, 1);
      if (d.x != -1)
      {
        action[0].type = action_move;
        action[0].loc = d;
				action[0].destination = dest;
        if (tile_distance(d.x, d.y, x, y) == 1)
        {  //if the final move brings it one move away from the building
          action[1].type = action_unload1;
          action[1].loc.x = x;
          action[1].loc.y = y;
       }
      }
    }
    else
    {  //it can't get to a building, so go for an enemy unit instead
      d = closest_move_to_enemy_unit(u->color, u->number, p->movetype, u->move, u->tilex, u->tiley, &dest);
      if (d.x != -1)
      {
        d = closest_move_to_destination(u->color, u->number, p->movetype, u->move, u->tilex, u->tiley, d, 2);
        if (d.x != -1)
        {
          action[0].type = action_move;
          action[0].loc = d;
					action[0].destination = dest;
        }
      }
    }
    if (action[1].type == action_none)
    {  //try to unload whenever it has something loaded
      if (action[0].type == action_move)
      {
        d = unloadable_point(action[0].loc.x, action[0].loc.y, 1);
      }
      else
      {
        d = unloadable_point(u->tilex, u->tiley, 1);
      }
      if (d.x != -1)
      {
        if (action[0].type == action_none)
        {
          action[0].type = action_unload1;
          action[0].loc = d;
        }
        else
        {
          action[1].type = action_unload1;
          action[1].loc = d;
        }
      }
    }
  }
}

void check_tcopter_options(_unit *u)
{
  _loc d, t;
  unsigned int i;
  int x, y;
  bool repeat = true;
  bool repeated = false;
	unitprops *p = NULL;
  d.x = -1;
  d.y = -1;

  if (u->load[0].loaded != 0)
  {  //if the copter has a passenger
    d = nearest_enemy_building(u->color, u->tilex, u->tiley);
  }
  else
  {
    i = 0;
    x = 999;
    while (i < need_transport.size())
    {
			p = &unitstats[need_transport[i]->type];
      if ((p->movetype == M_INFANTRY) || (p->movetype == M_MECH))
      {
        t = best_copter_adjacent_space(need_transport[i]->tilex, need_transport[i]->tiley);
        if (t.x != -1)
        {
          if (worldmap.tile[t.x][t.y].get_step() < x)
          {
            x = worldmap.tile[t.x][t.y].get_step();
            d = t;
          }
        }
      }
      i++;
    }
  }
	p = &unitstats[u->type];
  while (repeat == true)
  {
    x = d.x;
    y = d.y;
    if (any_unit_here(x, y) == -1)
    {  //if the target space is empty
      repeat = false;
      if (u->load[0].loaded == 1)
      {
        d = closest_move_to_destination(u->color, u->number, p->movetype, u->move, u->tilex, u->tiley, d, 1);
      }
      else
      {
        d = closest_move_to_destination(u->color, u->number, p->movetype, u->move, u->tilex, u->tiley, d, 0);
      }
      if (d.x != -1)
      {
        action[0].type = action_move;
        action[0].loc = d;
				action[0].destination.x = x;
				action[0].destination.y = y;
        if ((tile_distance(x, y, d.x, d.y) == 1) && (u->load[0].loaded == 1))
        {  //if the move takes it to one away from the carried unit's target
          action[1].type = action_unload1;
          action[1].loc.x = x;
          action[1].loc.y = y;
        }
      }
      else
      {
        if ((tile_distance(x, y, u->tilex, u->tiley) == 1) && (u->load[0].loaded == 1))
        {  //if it is already one away from the carried unit's target
          action[0].type = action_unload1;
          action[0].loc.x = x;
          action[0].loc.y = y;
        }
      }
    }
    else
    {  //if there is a unit on that space
      if (repeated == true)
      {
        repeat = false;
      }
      repeated = true;
      d = a_free_space(u, x, y);
    }
  }
}

void check_lander_options(_unit *u)
{
  bool to_enemy = true;
	bool empty = true;
	bool capper = false;
  int dist = 999, m;
  unsigned int i;
	unitprops *p = &unitstats[u->type];
  _loc d, targetbeach;
  targetbeach.x = -1;
  targetbeach.y = -1;
	
	i = 0;
	while (signed(i) < p->loadcapacity)
	{
		if (u->load[i].loaded == 0)
		{  //only head to where the enemy is if the lander's full
			to_enemy = false;
		}
		else
		{
			empty = false;
		}
		i++;
	}

	if (to_enemy)
	{
		i = 0;
		while (signed(i) < p->loadcapacity)
		{
			if ((u->load[i].loaded == 1) && (unitstats[u->load[i].type].cancapture))
			{
				capper = true;
			}
			i++;
		}
		d = closest_enemy_beach(u->color, u->tilex, u->tiley, capper);
		if (d.x != -1)
		{
			targetbeach = d;
		}
	}
	else if (!empty)  //if it doesn't have both slots filled
	{  //check to see if it's on enemy territory
		m = M_TREAD;
		i = 0;
		while (signed(i) < p->loadcapacity)
		{
			if ((u->load[i].loaded == 1) && (unitstats[u->load[i].type].cancapture))
			{
				capper = 1;
				m = M_MECH;
			}
			i++;
		}
    d = reachable_enemy_building(u->color, m, u->tilex, u->tiley, capper);
    if (d.x != -1)
    {  //if it is (so it maybe could only unload one unit last turn), unload now
      d.x = u->tilex;
      d.y = u->tiley;
      check_unload(u, d);
    }
  }
  else
  {  //no passengers, find the closest unit in need of transportation
    i = 0;
    while (i < need_transport.size())
    {
      if (unitstats[need_transport[i]->type].basetype == LAND)
      {
        d = closest_beach_for_lander(u->color, need_transport[i]->tilex, need_transport[i]->tiley, u);
        if (d.x != -1)
        {
          if (worldmap.tile[d.x][d.y].get_step() < dist)
          {
            targetbeach = d;
            dist = worldmap.tile[d.x][d.y].get_step();
          }
        }
      }
      i++;
    }
  }
  if (targetbeach.x != -1)
  {
    d = closest_move_to_destination(u->color, u->number, M_LANDER, u->move, u->tilex, u->tiley, targetbeach, 0);
    if (d.x != -1)
    {
      action[0].type = action_move;
      action[0].loc = d;
			action[0].destination = targetbeach;
      if ((d.x == targetbeach.x) && (d.y == targetbeach.y))
      {  //if it reaches its destination at the end of the move
        check_unload(u, targetbeach);
      }
    }
  }
}

void check_unload(_unit *u, _loc targetbeach)
{
  int actnum = 1;
  _loc d;
	unitprops *p;
	d.x = -1; d.y = -1;
	if (u->load[0].loaded == 1)
  {
		p = &unitstats[u->load[0].type];
    if ((p->movetype == M_INFANTRY) || (p->movetype == M_MECH))
    {
      d = unloadable_point(targetbeach.x, targetbeach.y, 1);
    }
    else
    {
      d = unloadable_point(targetbeach.x, targetbeach.y, 0);
    }
    if (d.x != -1)
    {
      action[actnum].type = action_unload1;
      action[actnum].loc = d;
      actnum++;
    }
  }
  if (u->load[1].loaded == 1)
  {
    if (actnum == 1)
    {  //if there wasn't a unit in load[0]
      d.x = -1;
      d.y = -1;
    }
		p = &unitstats[u->load[1].type];
    if ((p->movetype == M_INFANTRY) || (p->movetype == M_MECH))
    {
      d = unloadable_point(targetbeach.x, targetbeach.y, 1, d);
    }
    else
    {
      d = unloadable_point(targetbeach.x, targetbeach.y, 0, d);
    }
    if (d.x != -1)
    {
      action[actnum].type = action_unload2;
      action[actnum].loc = d;
    }
  }
}

bool reachable_within_one_tile(_loc d)
{
	if (worldmap.tile[d.x][d.y].get_step() < 99) return true;
	if (d.y - 1 > -1)
	{
		if (worldmap.tile[d.x][d.y - 1].get_step() < 99) return true;
	}
	if (d.y + 1 < worldmap.h)
	{
		if (worldmap.tile[d.x][d.y + 1].get_step() < 99) return true;
	}
	if (d.x - 1 > -1)
	{
		if (worldmap.tile[d.x - 1][d.y].get_step() < 99) return true;
	}
	if (d.x + 1 < worldmap.l)
	{
		if (worldmap.tile[d.x + 1][d.y].get_step() < 99) return true;
	}
	return false;
}

void check_convenient_APC(_unit *u)
{
  int i, deststep;
  _loc d;
	_unit *apc;
	
  //if (((action[0].type == action_move) && (action[1].type == action_none)) || (action[0].type == action_none))
	//if it isn't moving, or moving somewhere but not doing anything when at the destination
	deststep = worldmap.tile[action[0].destination.x][action[0].destination.y].get_step();
	if ((action[0].type == action_none) || 
		 ((action[0].type == action_move) && (deststep > u->move)))
  {  //if it isn't moving, or its destination is far away
    d = empty_APC_nearby(u, u->move);
    if (d.x != -1)
    {  //if there is an empty APC reachable this turn
      if (action[0].type == action_move)
			{  //make sure the APC can actually unload the unit at its destination
				i = worldmap.tile[d.x][d.y].unit_here();
				apc = &(player[u->color].unit[i % 100]);
				
				worldmap.backup_pathmap();
				worldmap.create_limited_pathmap(u->color, unitstats[apc->type].movetype, d.x, d.y, (deststep / u->move) * apc->move);
				// (on "(deststep / u->move) * apc_move": the APC is useless if it
				// takes more turns for it to reach the destination than for the unit
				if (!reachable_within_one_tile(action[0].destination))
				{  // if the APC can't get within one tile of the destination
					worldmap.restore_pathmap();
					return;
				}
				worldmap.restore_pathmap();
			}
				
			action[0].type = action_move;
      action[0].loc = d;
			action[0].destination = d;
      action[1].type = action_load;
      action[1].loc = d;
    }
  }
}

void check_available_copter(_unit *u)
{
  _loc d, dest;
  int z;

  d = closest_move_to_empty_copter(u->color, u->number, unitstats[u->type].movetype, u->move, u->tilex, u->tiley, &dest);
  if (d.x != -1)
  {  //if there is an empty T-Copter available
    action[0].type = action_move;
    action[0].loc = d;
		action[0].destination = dest;
    z = unit_here_thorough(u->color, d.x, d.y, u->number);
    if (z != -1)
    {  //if there is a unit at the final move point, it must be the copter
      action[1].type = action_load;
      action[1].loc = d;
    }
  }
}

void check_available_lander(_unit *u)
{
  int z;
  _loc d, dest;
	
	if (unitstats[u->type].basetype != LAND) return;

  if ((unitstats[u->type].loadcapacity == 0) || (u->load[0].loaded == 1) || (u->load[1].loaded == 1))
  {  //if it isn't an APC, or is an APC with a unit loaded into it
    d = closest_move_to_available_lander(u->color, u->number, unitstats[u->type].movetype, u->move, u->tilex, u->tiley, &dest);
    if (d.x != -1)
    {  //if there is an empty lander available
      action[0].type = action_move;
      action[0].loc = d;
			action[0].destination = dest;
      z = unit_here_thorough(u->color, d.x, d.y, u->number);
      if (z != -1)
      {  //if there is a unit at the final move point, it must be the lander
        action[1].type = action_load;
        action[1].loc = d;
      }
    }
  }
}

void check_teleports(_unit *u)
{
	bool f;
	int maxmoves = 0;
	unsigned int i = 0;
	_loc l, c;
	vector<_loc> sep;
	vector<_loc> good;
	
	if (teleports.size() < 2)
	{  //if there are no teleports on the map (or just one, which would be useless)
		return;
	}
	l = closest_teleport(u);
	if (l.x == -1)
	{
		return;
	}
	
	if ((action[0].type == action_move) && (action[0].loc.x != -1))
	{
		maxmoves = worldmap.tile[action[0].destination.x][action[0].destination.y].get_step() - 1;
		if (maxmoves <= u->move)
		{  //if the destination is within reach, no need to teleport
			return;
		}
		c = closest_teleport_to_destination(u->color, unitstats[u->type].movetype, action[0].destination.x, action[0].destination.y, maxmoves);
		if (c.x != -1)
		{  //if there is a teleport that the unit can take to the target
			if ((c.x == l.x) && (c.y == l.y))
			{  //if the closest teleport to the unit and the closest teleport to the destination are the same
				return;  //then there's no benefit in moving to the teleport
			}
			action[0].loc = closest_move(u->color, u->number, unitstats[u->type].movetype, u->tilex, u->tiley, l.x, l.y, -1);
			if ((action[0].loc.x == l.x) && (action[0].loc.y == l.y))
			{  //if it can reach the teleport this turn
				action[1].type = action_teleport;
				action[1].loc.x = c.x;
				action[1].loc.y = c.y;
				return;
			}
		}
	}
	else if (action[0].type == action_none)
	{
		f = true;
		i = 0;
		
		while (i < teleports.size())
		{  //create a vector of empty teleport locations which the unit can't reach on its own
			if ((worldmap.tile[teleports[i].x][teleports[i].y].get_step() >= 999) && (any_unit_here(teleports[i].x, teleports[i].y) == -1))
			{  //if this teleport tile isn't reachable from the unit's current position
				sep.push_back(teleports[i]);
			}
			i++;
		}
		i = 0;
		while (i < sep.size())
		{
			c = reachable_enemy_building(u->color, unitstats[u->type].movetype, sep[i].x,
			                             sep[i].y, unitstats[u->type].cancapture, 0);
			if (c.x != -1)
			{  //if there is a reachable enemy building (or neutral and the unit is infantry or mech) from this teleport
				if (f)
				{  //change the unit actions to movement when the first good teleport is found
					action[0].type = action_move;
					action[0].loc = closest_move(u->color, u->number, unitstats[u->type].movetype, u->tilex, u->tiley, l.x, l.y, -1);
					if ((action[0].loc.x != l.x) || (action[0].loc.y != l.y))
					{  //if the unit can't get to the teleport this turn
						return;  //don't bother finding any other good teleports
					}
					f = false;
				}
				good.push_back(sep[i]);  //add it to the list of good teleport locations
			}  //NOTE: THIS MEANS THE AI CURRENTLY DOESN'T TELEPORT TO GET AT ENEMY UNITS
			i++;
		}
		//if it gets to this point, either the unit can reach the teleport this turn or there's no good teleports
		if (good.size() == 0)
		{
			return;
		}
		//if it gets to this point, it's ready to teleport after moving
		i = rand()%good.size();  //just pick a random good teleport
		action[1].type = action_teleport;
		action[1].loc = good[i];
	}
}

_loc closest_teleport(_unit *u)
{
	unsigned int i = 0;
	int tempstep;
	int beststep = 999;
	_loc best;
	_loc temp;
	best.x = -1;
	best.y = -1;
	while (i < teleports.size())
	{
		temp.x = teleports[i].x;
		temp.y = teleports[i].y;
		tempstep = worldmap.tile[temp.x][temp.y].get_step();
		if (tempstep < beststep)
		{
			beststep = tempstep;
			best.x = temp.x;
			best.y = temp.y;
		}
		i++;
	}
	return best;
}

int can_unload_from_here(int x, int y, int mountains)
{
  _loc d = unloadable_point(x, y, mountains);
  if (d.x != -1)
  {
    return 1;
  }
  return 0;
}
