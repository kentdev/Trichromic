#include "empire.h"

void check_capturing(_unit *u)
{
  _loc d;
	_tile *t = &worldmap.tile[u->tilex][u->tiley];
  if (!unitstats[u->type].cancapture)
  {
    return;
  }
	
	if (action[0].type != action_none) return;
	
  if (t->building_type() == CITY)
  {
  	action[0].type = action_standby;
  	action[0].loc.x = u->tilex;
  	action[0].loc.y = u->tiley;
  }
  else
  {
    d = capturable_building(u);
    if (d.x != -1)  //if there is a capturable building in range
    {
      if ((d.x == u->tilex) && (d.y == u->tiley))
      {  //if it is already on the capturable building
        action[0].type = action_standby;
        action[0].loc = d;
      }
      else
      {  //if not, move there
        action[0].type = action_move;
        action[0].loc = d;
      }
    }
  }
	action[0].destination = action[0].loc;  //check_capturing only deals with 
	action[1].destination = action[1].loc;  //immediately reachable buildings
}

void check_attackable_units(_unit *u)
{
  _aitarget t;
  _loc d;
	unitprops *p = &unitstats[u->type];
  if (p->attacktype == -1)
  {  //if the unit can't attack at all
    return;
  }
  if (u->attackdelay > 0) return;
	if (action[0].type == action_move)
	{  //this is only ever the case if it is moving to capture something this turn
		check_attackable_units(u, action[0].loc);
		return;
	}
	if (action[0].type == action_standby)
	{  //if the unit has not decided it needs to stay on this tile (eg, to control a city)
		check_attackable_units(u, action[0].loc);
		return;
	}
  
  t = best_target();
  if (p->attacktype == RANGED)
  {
    if (t.type != -1)  //if a target has been found, the unit will attack
    {
      action[0].type = action_attack;
      action[0].loc = t.loc;
    }
  }
  else  //a direct attacker
  {
    if (t.type != -1)
    {
      d = loc_to_attack(u, t);
      if ((u->tilex == d.x) && (u->tiley == d.y))
      {  //if the unit is already in the best position to attack
        action[0].type = action_attack;
        action[0].loc = t.loc;
      }
      else
      {
				if (d.x != -1)
				{
					action[0].type = action_move;
					action[0].loc = d;
					action[0].destination = d;
					action[1].type = action_attack;
					action[1].loc = t.loc;
				}
      }
    }
  }
}

void check_attackable_units(_unit *u, _loc l)
{  //only check to see if it can attack a unit from this tile
	int i = 0, x, y, c = 0;
	_aitarget t, best;
	best.type = -1;
	
	if (unitstats[u->type].attacktype == RANGED) return;
	
	while (i < 4)
	{
		x = l.x;
		y = l.y;
		switch (i)
		{
			case 0:
				x++;
				break;
			case 1:
				x--;
				break;
			case 2:
				y++;
				break;
			case 3:
				y--;
				break;
		}
		
		t = aitarget_here(u, x, y);
		if (t.type != -1)
		{
			if (t.damage_cost - t.attacker_cost > c)
			{
				c = t.damage_cost - t.attacker_cost;
				best = t;
			}
		}
		i++;
	}
	
	if (best.type != -1)
	{
		action[1].type = action_attack;
		action[1].loc = best.loc;
	}
}

void check_closest_capturable(_unit *u)
{  //assumes the unit can capture buildings
	_loc d, dest;
	
  d = closest_move_to_enemy_building(u->color, u->number, unitstats[u->type].movetype, u->move, u->tilex, u->tiley, 1, &dest);
  if (d.x != -1)
  {
    if ((d.x != u->tilex) || (d.y != u->tiley))
    { //if the closest move is not where it already is
      action[0].type = action_move;
      action[0].loc = d;
			action[0].destination = dest;
    }
  }
}

void check_closest_attackable(_unit *u)
{  //assumes the unit can attack
  _loc d;
	_loc attackables_loc;
  d.x = -1;
  d.y = -1;
	unitprops *p = &unitstats[u->type];
	
	if (u->attackdelay > 0) return;
	
  if (p->attacktype == DIRECT)
  {  //if it is a direct attacker, head for the nearest enemy unit
    d = closest_move_to_enemy_unit(u->color, u->number, p->movetype, u->move, u->tilex, u->tiley, &attackables_loc);		
    if (d.x != -1)
    {
      if ((d.x != u->tilex) || (d.y != u->tiley))
      { //if the closest move is not where it already is
        action[0].type = action_move;
        action[0].loc = d;
				action[0].destination = attackables_loc;
      }
    }
    else  //no enemy units, head for nearest enemy base
    {
      d = closest_move_to_enemy_building(u->color, u->number, p->movetype, u->move, u->tilex, u->tiley, 0, &attackables_loc);
      if (d.x != -1)
      {
        if ((d.x != u->tilex) || (d.y != u->tiley))
        { //if the closest move is not where it already is
          action[0].type = action_move;
          action[0].loc = d;
					action[0].destination = attackables_loc;
        }
      }
    }
  }
  else
  {  //if it is a ranged attacker
    d = loc_for_ranged_attack(u, u->tilex, u->tiley, u->rangemin, u->rangemax);
    if (d.x != -1)
    {
			attackables_loc = d;
      d = closest_move_to_destination(u->color, u->number, p->movetype, u->move, u->tilex, u->tiley, d, 0);
    }
    else
    {
      d = closest_move_to_enemy_unit(u->color, u->number, p->movetype, u->move, u->tilex, u->tiley, &attackables_loc);
    }
    if (d.x != -1)
    {
      if ((d.x != u->tilex) || (d.y != u->tiley))
      { //if the closest move is not where it already is
        action[0].type = action_move;
        action[0].loc = d;
				action[0].destination = attackables_loc;
      }
    }
  }
}

void check_enemy_bases(_unit *u)
{
  _loc d, dest;

  if (unitstats[u->type].loadcapacity == 0)
  {  //if it isn't an APC
    d = reachable_enemy_building(u->color, unitstats[u->type].movetype, u->tilex, u->tiley, 0, 1);
    if (d.x != -1)
    {  //if there are reachable enemy unit-producing buildings
			dest = d;
      d = closest_move_to_destination(u->color, u->number, unitstats[u->type].movetype, u->move, u->tilex, u->tiley, d, 0);
      if (d.x != -1)
      {
        action[0].type = action_move;
        action[0].loc = d;
				action[0].destination = dest;
      }
    }
  }
}

vector<_aitarget> list_targets(_unit *u)
{
  _aitarget t;
  int x, y, min, max, dist;
  vector<_aitarget> a;
	unitprops *p = &unitstats[u->type];
  a.resize(0);
  
	if (p->attacktype == DIRECT)
  {
    y = u->tiley - u->move - 1;
    if (y < 0) y = 0;
    while ((y <= u->tiley + u->move + 1) && (y < worldmap.h))
    {
      x = u->tilex - u->move - 1;
      if (x < 0) x = 0;
      while ((x <= u->tilex + u->move + 1) && (x < worldmap.l))
      {
        t = aitarget_here(u, x, y);
        if (t.type != -1)
        {  //if there is a viable target here
          if (can_get_to_aitarget(u, t))
          {  //even if the unit is in range, it may not be attackable due to other units
            a.push_back(t);
          }
        }
        x++;
      }
      y++;
    }
  }
  else if (p->attacktype == RANGED)
  {
    min = u->rangemin;
    max = u->rangemax;
    y = u->tiley - max;
    if (y < 0) y = 0;
    while ((y <= u->tiley + max) && (y < worldmap.h))
    {
      x = u->tilex - max;
      if (x < 0) x = 0;
      while ((x <= u->tilex + max) && (x < worldmap.l))
      {
        dist = tile_distance(u->tilex, u->tiley, x, y);
        if ((dist >= min) && (dist <= max))
        {  //make sure the location to be checked is in firing range
          t = aitarget_here(u, x, y);
          if (t.type != -1)  //if there is a viable target here
          {
            a.push_back(t);
          }
        }
        x++;
      }
      y++;
    }
  }
  return a;
}

_aitarget best_target()
{
  _aitarget best;
  best.damage_cost = 0;
  best.type = -1;
  int i = 0;
  int s = targets.size();
  while (i < s)
  {
    if (targets[i].damage_cost > targets[i].attacker_cost)
    {
      if (targets[i].damage_cost > best.damage_cost)
      {
        best = targets[i];
      }
    }
    i++;
  }
  return best;
}

_aitarget aitarget_here(_unit *u, int tx, int ty)
{
  _aitarget p;
  _unit *t;
  int q;
  p.type = -1;
  q = any_unit_here(tx, ty);
  if (q != -1)
  {
    t = &player[q / 100].unit[q % 100];
    if ((t->color != u->color) && (player[t->color].team != player[u->color].team) && (type_can_attack(u, t)))
    {
      p.type = t->type;
      p.loc.x = tx;
      p.loc.y = ty;
      p.health = t->health;
      p.damage_cost = get_damage_cost(u, t);
      p.attacker_cost = get_return_damage_cost(u, t);
    }
  }
  return p;
}

int get_damage_cost(_unit *a, _unit *d)
{
  float basic = float(basic_damage(a, d)) * a->health / 10;
  int def = defbonus(d->tilex, d->tiley);  //most of this code lifted from battle.cpp
  if (unitstats[d->type].basetype == AIR)
  {  //air units have no terrain defense bonus
    def = 0;
  }
  //apply_player_bonuses(basic, a, d);
  float damage = basic - ((basic * float(def)) / 10);
  return int(damage * d->price);
}

int get_return_damage_cost(_unit *a, _unit *d)
{
  float basic, defhealth, damage;
  int def;
  
  if (unitstats[d->type].attacktype != DIRECT)
  {  //if the defender isn't a direct attacker, no return damage
    return 0;
  }
  basic = float(basic_damage(a, d)) * a->health / 10;
  def = defbonus(d->tilex, d->tiley);  //most of this code lifted from battle.cpp
  defhealth = d->health;
  if (unitstats[d->type].basetype == AIR)
  {  //air units have no terrain defense bonus
    def = 0;
  }
  //apply_player_bonuses(basic, a, d);
  damage = basic - ((basic * float(def)) / 10);
  defhealth -= damage;
  if (defhealth < 0.01)
  {  //if the defender is destroyed, no damage to the attacker
    return 0;
  }

  basic = float(basic_damage(d, a)) * defhealth / 10;
  def = defbonus(a->tilex, a->tiley);  //most of this code lifted from battle.cpp
  if (unitstats[a->type].basetype == AIR)
  {  //air units have no terrain defense bonus
    def = 0;
  }
  //apply_player_bonuses(basic, d, a);
  damage = basic - ((basic * float(def)) / 10);
  return int(damage * a->price);
}

bool can_get_to_aitarget(_unit *a, _aitarget d)
{  //checks all 4 tiles adjacent to D to see if one is in range of A
  int x, y, z, i;
  i = 0;
  while (i < 4)
  {
    switch(i)
    {
      case 0:
        x = d.loc.x + 1;
        y = d.loc.y;
        break;
      case 1:
        x = d.loc.x - 1;
        y = d.loc.y;
        break;
      case 2:
        x = d.loc.x;
        y = d.loc.y + 1;
        break;
      case 3:
        x = d.loc.x;
        y = d.loc.y - 1;
        break;
    }
    if (worldmap.in_bounds(x, y))
    {
      if (worldmap.tile[x][y].get_step() <= a->move)
      {
        z = worldmap.tile[x][y].unit_here();
        if ((z == -1) || ((a->tilex == x) && (a->tiley == y)))
        {  //make sure there's no unit here (besides maybe the attacker itself)
          return true;
        }
      }
    }
    i++;
  }
  return false;
}

_loc capturable_building(_unit *u)
{  //this function assumes that u is either infantry or mech (can capture)
  _loc l;
  int x = u->tilex - u->move;
  int y = u->tiley - u->move;
  int priority = 0;
  int p = 0;
  bool unit_ok = false;
  _tile *tile;
  l.x = -1;
  l.y = -1;
  if (y < 0) y = 0;
  while ((y <= u->tiley + u->move) && (y < worldmap.h))
  {
    x = u->tilex - u->move;
    if (x < 0) x = 0;
    while ((x <= u->tilex + u->move) && (x < worldmap.l))
    {
      tile = &worldmap.tile[x][y];
      if (tile->get_step() <= u->move)
      {  //if the unit can actually get to this location
        if ((tile->unit_here() == -1) || (tile->unit_here() == (u->color * 100) + u->number))
        {
          unit_ok = true;
        }
        if ((tile->is_building()) && (unit_ok == true))
        {
          p = u->move + 1 - tile_distance(x, y, u->tilex, u->tiley);  //priority based on distance
          if (tile->is_HQ())
          {
            if ((player[tile->get_owner()].team != player[u->color].team))
            {
              p += 7;  //this building has a higher priority than others
            }
            else
            {
              p = 0;  //if it is the unit's team capital, it has 0 capture priority
            }
          }
          else  //not a capital
          {
            if ((tile->get_owner() != -1) && (player[tile->get_owner()].team == player[u->color].team))
            {
              p = 0;  //capture priority for owned buildings is 0
            }
            else
            {
              if (tile->is_unit_producing())
              {
                p += 3;  //higher priority for unit-producing buildings
              }
            }
          }
          if (tile->unit_here() != -1)
          {
            p = 0;
          }
          if (p > priority)
          {  //if this building has greater priority than the last one
            priority = p;
            l.x = x;
            l.y = y;
          }
        }
      }
      x++;
    }
    y++;
  }
  return l;
}
