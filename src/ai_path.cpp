#include "empire.h"

extern bool type_can_attack(_unit *a, _unit *d);
extern int any_unit_here(int tx, int ty);
extern int moves_needed(int mtype, int tx, int ty, int plyr);
extern int get_set(int mapdat);
extern int can_unload_from_here(int x, int y, int mountains);
extern int intmax(int a, int b);
extern int get_damage_cost(_unit *a, _unit *d);
extern int get_return_damage_cost(_unit *a, _unit *d);
extern _aitarget aitarget_here(_unit *u, int tx, int ty);

extern vector<_unit*> lander;
extern vector<_connectedlocs> beaches;


extern void save_pathmap_info();
vector<_aipath> best_aipath(int plyr, int mtype, int ox, int oy, int dx, int dy)
{  //assumes fill_pathmap_for_unit() has been called
  vector<_aipath> best, orig;
  _aipath t;
  int x, y, z, b, d, i, s;
  best.clear();
  best.reserve(25);
  orig.clear();
  orig.reserve(25);
  x = dx;
  y = dy;  //going from target to destination is much easier than vice-versa
  if (!worldmap.in_bounds(dx, dy))
  {
    best.push_back(t);
    best[0].dir = -1;
    best[0].total_moves = 0;
    return best;
  }
  if (worldmap.tile[x][y].get_step() == 999)
  {  //if the destination is unreachable
    best.push_back(t);
    best[0].dir = -1;
    best[0].total_moves = 0;
    return best;
  }
  while ((dx != ox) || (dy != oy))
  {
    d = -1;
    b = 999;
    i = 1;
    while (i < 5)
    {
      switch(i)
      {
        case UP:
          x = dx;
          y = dy - 1;
          break;
        case DOWN:
          x = dx;
          y = dy + 1;
          break;
        case LEFT:
          x = dx - 1;
          y = dy;
          break;
        case RIGHT:
          x = dx + 1;
          y = dy;
          break;
      }
      if (worldmap.in_bounds(x, y))
      {
        z = worldmap.tile[x][y].get_step();
        s = worldmap.tile[x][y].unit_here();
        if ((s != -1) && (player[s / 100].team != player[plyr].team))
        {
          z = 999;
        }
        if (z < b)
        {
          d = i;
          b = z;
        }
        if (z == b)
        {  //if both directions are the same
          z = rand()%2;  //the direction taken is random
          if (z == 0)
          {
            d = i;
	        }
        }
      }
      i++;
    }
    orig.push_back(t);
    s = orig.size();
    orig[s - 1].dir = d;
    orig[s - 1].total_moves = worldmap.tile[x][y].get_step();
    switch(d)
    {
      case UP:
        dy--;
        break;
      case DOWN:
        dy++;
        break;
      case LEFT:
        dx--;
        break;
      case RIGHT:
        dx++;
        break;
    }
  }
  i = orig.size() - 1;
  while (i >= 0)
  {  //reverse the list, so it goes from unit to destination
    best.push_back(orig[i]);
    i--;
  }
  s = best.size();
  i = 0;
  while (i < s)
  {  //the directions have to be reversed also
    switch(best[i].dir)
    {
      case UP:
        best[i].dir = DOWN;
        break;
      case DOWN:
        best[i].dir = UP;
        break;
      case LEFT:
        best[i].dir = RIGHT;
        break;
      case RIGHT:
        best[i].dir = LEFT;
        break;
    }
    i++;
  }
  return best;
}

_loc loc_to_attack(_unit *u, _aitarget t)
{
  _loc l;
  int i, x, y;

  i = 0;
  while (i < 4)
  {
    switch(i)
    {
      case 0:
        x = t.loc.x + 1;
        y = t.loc.y;
        break;
      case 1:
        x = t.loc.x - 1;
        y = t.loc.y;
        break;
      case 2:
        x = t.loc.x;
        y = t.loc.y + 1;
        break;
      case 3:
        x = t.loc.x;
        y = t.loc.y - 1;
        break;
    }
    if (worldmap.in_bounds(x, y))
    {
      if (worldmap.tile[x][y].get_step() <= u->move)
      {  //if this adjacent location to the target is in range
        if ((worldmap.tile[x][y].unit_here() == -1) || ((u->tilex == x) && (u->tiley == y)))
        {  //if there is no unit here, besides maybe the attacker
          l.x = x;
          l.y = y;
          return l;
        }
      }
    }
    i++;
  }
  l.x = -1;
  l.y = -1;
  return l;
}


_loc closest_free_building(int plyr, int u, int mtype, int moves, int basetype, int ox, int oy)
{
  _loc best;
  _loc closest;
  int d = 999;
  int y = 0;
  int x;
  int ok;
  int t;
  best.x = -1;
  best.y = -1;
  closest.x = -1;
  closest.y = -1;
  _tile* tile;
  while (y < worldmap.h)
  {
    x = 0;
    while (x < worldmap.l)
    {
      tile = &worldmap.tile[x][y];
      if ((tile->is_building()) && (tile->unit_here() == -1))
      {  //if there is a building here without a unit on it
        ok = 0;
        if (tile->is_HQ())
        {
          if (tile->owned_by(plyr))
          {
            if (basetype == LAND)
            {  //only land units can be repaired at the capital
              ok = 1;
            }
          }
        }
        else
        {
          if (tile->owned_by(plyr))
          {
            switch(basetype)
            {
              case LAND:
                if ((tile->is_building(CITY)) || (tile->is_building(BASE)))
                {
                  ok = 1;
                }
                break;
              case AIR:
                if (tile->is_building(AIRPORT))
                {
                  ok = 1;
                }
                break;
              case SEA:
                if (tile->is_building(PORT))
                {
                  ok = 1;
                }
                break;
            }
          }
        }
        if (ok == 1)  //if the unit can be repaired at this building
        {
          t = tile->get_step();
          if (t < d)
          {
            d = t;
            best.x = x;
            best.y = y;
          }
        }
      }
      x++;
    }
    y++;
  }
  if (best.x != -1)
  {  //if there is a path to the targeted tile
    closest = closest_move(plyr, u, mtype, ox, oy, best.x, best.y, -1);
  }
  return closest;
}


_loc closest_move_to_enemy_building(int plyr, int u, int mtype, int moves, int ox, int oy, int capping_unit, _loc *dest)
{
  _loc best;
  _loc closest;
  _loc temp;
  _tile *tile;
  vector<_aipath> q;
  vector<_loc> viable;
  best.x = -1;
  best.y = -1;
  closest.x = -1;
  closest.y = -1;
  viable.clear();
  int x, y, z, d, t, a;
  d = 999;
  y = 0;
  while (y < worldmap.h)
  {
    x = 0;
    while (x < worldmap.l)
    {
      tile = &worldmap.tile[x][y];
      if (tile->is_building())
      {  //if there is a building here
        a = 0;
        if (tile->get_owner() == -1)
        {  //if it is neutral
          a = 1;
        }
        else if (player[tile->get_owner()].team != player[plyr].team)
        {  //if it is owned by the enemy
          a = 1;
        }
        if ((a == 1) && (capping_unit == 1))
        {  //if this unit intends to capture the building
          t = any_unit_here(x, y);
          if (t != -1)
          {
            if (t / 100 == plyr)
            {  //if there is a friendly unit on it
              if (t % 100 != u)
              {  //if it isn't this unit
                if (unitstats[player[plyr].unit[t % 100].type].cancapture)
                {  //if it is already taking care of that building
                  a = 0;
                }
              }
            }
          }
        }
        else if (a == 1)
        {  //if it can't capture, then it shouldn't bother going after cities
          if (tile->is_building(CITY))
          {
            a = 0;
          }
          if (tile->get_owner() == -1)
          {  //don't bother with non-enemy bulidings at all if it can't capture
            a = 0;
          }
        }
        if (a == 1)
        {
          temp.x = x;
          temp.y = y;
          viable.push_back(temp);
        }
      }
      x++;
    }
    y++;
  }
  x = 0;
  y = 999;
  z = viable.size();
  while (x < z)
  {
    if (worldmap.tile[viable[x].x][viable[x].y].get_step() < y)
    {
      best.x = viable[x].x;
      best.y = viable[x].y;
      y = worldmap.tile[best.x][best.y].get_step();
			*dest = best;
    }
    x++;
  }
  if (best.x != -1)
  {  //if there is a path to the targeted tile
    closest = closest_move(plyr, u, mtype, ox, oy, best.x, best.y, -1);
  }
  return closest;
}


_loc closest_move_to_enemy_unit(int plyr, int u, int mtype, int moves, int ox, int oy, _loc *dest)
{
  _loc best;
  _loc closest;
  vector<_aipath> q;
  best.x = -1;
  best.y = -1;
  closest.x = -1;
  closest.y = -1;
  int i = 0;
  int x, y, d;
  d = 999;
  i = 0;
  _unit *temp;
  _unit *attacker = &player[plyr].unit[u];
  _loc temploc;
  while (i < 4)
  {
    if ((player[i].playing == 1) && (player[i].team != player[plyr].team))
    {
      y = 0;
      while (y < 50)
      {
        temp = &player[i].unit[y];
        if (temp->exists == 1)
        {
          if (type_can_attack(attacker, temp))
          {
            if (get_damage_cost(attacker, temp) > get_return_damage_cost(attacker, temp))
            {
              temploc = best_pathmap_loc(temp->tilex, temp->tiley);
              if ((temploc.x != -1) && (temploc.y != -1))
              {
                x = worldmap.tile[temploc.x][temploc.y].get_step();
                if (x < d)
                {
                  d = x;
                  best.x = temploc.x;
                  best.y = temploc.y;
                  *dest = best;
                }
              }
            }
          }
        }
        y++;
      }
    }
    i++;
  }
  if (best.x != -1)
  {  //if there is a path to the targeted tile
    closest = closest_move(plyr, u, mtype, ox, oy, best.x, best.y, -1);
  }
  return closest;
}

_loc best_copter_adjacent_space(int tx, int ty)
{ //assumes the pathmap has been filled correctly
  int i = 0;
  int x, y, dist;
  _loc d;
  d.x = -1;
  d.y = -1;
  dist = 999;
  while (i < 4)
  {
    switch(i)
    {
      case 0:
        x = tx - 1;
        y = ty;
        break;
      case 1:
        x = tx + 1;
        y = ty;
        break;
      case 2:
        x = tx;
        y = ty - 1;
        break;
      case 3:
        x = tx;
        y = ty + 1;
        break;
    }
    if (worldmap.in_bounds(x, y))
    {
      if (worldmap.tile[x][y].get_step() < dist)
      {
        if (worldmap.tile[x][y].unit_here() == -1)
        {
          if (moves_needed(M_INFANTRY, x, y, 0) < 99)
          {  //if the unit to be loaded can move here
            d.x = x;
            d.y = y;
            dist = worldmap.tile[x][y].get_step();
          }
        }
      }
    }
    i++;
  }
  return d;
}

_loc a_free_space(_unit *u, int tx, int ty)
{
	int i, j;
	_loc d;
	_tile *tile;
	d.x = -1;
	d.y = -1;
	
	j = ty - u->move;
	if (j < 0) j = 0;
	while (j <= ty + u->move)
	{
		i = tx - u->move;
		if (i < 0) i = 0;
		while (i <= tx + u->move)
		{
			if (worldmap.in_bounds(i, j))
			{
				tile = &worldmap.tile[i][j];
				if (tile->get_step() <= u->move)
				{  //  if this tile can be reached this turn
					if ((tile->get_step() < 99) && (tile->unit_here() == -1))
					{  //  if the tile is reachable and there isn't a unit there already
						if (!((tile->owned_by(u->color)) && (tile->is_unit_producing())))
						{  //  if it isn't a building, or isn't owned by this unit's player
							d.x = i;
							d.y = j;
							return d;
						}
					}
				}
			}
			i++;
		}
		j++;
	}
  return d;
}


_loc empty_APC_nearby(_unit *u, int move)
{
  _loc best;
  vector<_aipath> q;
  best.x = -1;
  best.y = -1;
	unitprops *p;
  int x, y, z, d, r, w;
  d = 999;
  y = u->tiley - move;
  while (y < u->tiley + move)
  {
    x = u->tilex - move;
    while (x < u->tilex + move)
    {
      w = -1;
      r = any_unit_here(x, y);
      if ((r != -1) && (r / 100 == u->color))
      {
        w = r % 100;
      }
      if (w != -1)
      {
				p = &unitstats[player[u->color].unit[w].type];
        if ((p->basetype == LAND) && (p->loadcapacity - player[u->color].unit[w].units_loaded() > 0))
        {
          z = worldmap.tile[x][y].get_step();
          if ((z <= move) && (z < d))
          {
            d = z;
            best.x = x;
            best.y = y;
          }
        }
      }
      x++;
    }
    y++;
  }
  return best;
}


_loc closest_move_to_destination(int plyr, int u, int mtype, int moves, int ox, int oy, _loc dest, int offset)
{
  _loc best;
  _loc closest;
  _loc goodloc;
  vector<_aipath> q;
  best.x = -1;
  best.y = -1;
  closest.x = -1;
  closest.y = -1;
  goodloc.x = -1;
  goodloc.y = -1;
  int x, y, z, d;
  d = 999;
  y = dest.y - offset;
  while (y <= dest.y + offset)
  {
    x = dest.x - offset;
    while (x <= dest.x + offset)
    {
      z = any_unit_here(x, y);
      if (z == -1)
      {
        if (tile_distance(x, y, dest.x, dest.y) == offset)
        {  //if the distance from the current tile to the desired one equals the specified offset
          if (worldmap.tile[x][y].get_step() < d)
          {
            goodloc.x = x;
            goodloc.y = y;
            d = worldmap.tile[x][y].get_step();
          }
        }
      }
      x++;
    }
    y++;
  }
  if (goodloc.x != -1)
  {
    q = best_aipath(plyr, mtype, ox, oy, goodloc.x, goodloc.y);
    if (q[0].dir != -1)
    {
      best.x = goodloc.x;
      best.y = goodloc.y;
    }
  }
  if (best.x != -1)
  {  //if there is a path to the targeted tile
    closest = closest_move(plyr, u, mtype, ox, oy, best.x, best.y, -1);
  }
  return closest;
}


_loc empty_adjacent_space(int tx, int ty)
{
  _loc d;
  d.x = -1;
  d.y = -1;
  if (any_unit_here(tx - 1, ty) == -1)
  {
    d.x = tx - 1;
    d.y = ty;
  }
  else if (any_unit_here(tx + 1, ty) == -1)
  {
    d.x = tx + 1;
    d.y = ty;
  }
  else if (any_unit_here(tx, ty - 1) == -1)
  {
    d.x = tx;
    d.y = ty - 1;
  }
  else if (any_unit_here(tx, ty + 1) == -1)
  {
    d.x = tx;
    d.y = ty + 1;
  }
  return d;
}


_loc closest_move_to_empty_copter(int plyr, int u, int mtype, int moves, int ox, int oy, _loc *dest)
{
  _loc best;
  _loc closest;
  vector<_aipath> q;
  best.x = -1;
  best.y = -1;
  closest.x = -1;
  closest.y = -1;
  int x, y, d, dest_id;
	unitprops *p;
  d = 999;
  dest_id = -1;
  y = 0;
  while (y < 50)
  {
    if (player[plyr].unit[y].exists == 1)
    {
			p = &unitstats[player[plyr].unit[y].type];
      if ((p->basetype == AIR) && (p->loadcapacity - player[plyr].unit[y].units_loaded() > 0))
      {
        x = tile_distance(player[plyr].unit[y].tilex, player[plyr].unit[y].tiley, ox, oy);
        if (x < d)
        {
          q = best_aipath(plyr, mtype, ox, oy, player[plyr].unit[y].tilex, player[plyr].unit[y].tiley);
          if (q[0].dir != -1)
          {
            d = x;
            best.x = player[plyr].unit[y].tilex;
            best.y = player[plyr].unit[y].tiley;
						*dest = best;						
            dest_id = (100 * plyr) + y;
          }
        }
      }
    }
    y++;
  }
  if (best.x != -1)
  {  //if there is a path to the targeted tile
    closest = closest_move(plyr, u, mtype, ox, oy, best.x, best.y, dest_id);
  }
  return closest;
}


_loc closest_move_to_available_lander(int plyr, int u, int mtype, int moves, int ox, int oy, _loc *dest)
{
  _loc best;
  _loc closest;
  vector<_aipath> q;
  _tile *tile;
  best.x = -1;
  best.y = -1;
  closest.x = -1;
  closest.y = -1;
  int z, d, m;
  unsigned int y;
  d = 999;
  y = 0;
  z = -1;
  m = -1;
  while (y < lander.size())
  {
    if ((lander[y]->load[0].loaded == 0) || (lander[y]->load[1].loaded == 0))
    {
      tile = &worldmap.tile[lander[y]->tilex][lander[y]->tiley];
      if (tile->get_step() < d)
      {
        z = y;
        d = tile->get_step();
      }
    }
    y++;
  }
  if (z != -1)
  {
    q = best_aipath(plyr, mtype, ox, oy, lander[z]->tilex, lander[z]->tiley);
    if (q[0].dir != -1)
    {
      best.x = lander[z]->tilex;
      best.y = lander[z]->tiley;
      m = (100 * plyr) + lander[z]->number;
      *dest = best;
    }
  }
  if (best.x != -1)
  {  //if there is a path to the targeted tile
    closest = closest_move(plyr, u, mtype, ox, oy, best.x, best.y, m);
  }
  return closest;
}


_loc nearest_enemy_building(int plyr, int tx, int ty)
{
  int x, y, d, a, t;
  _loc theloc;
  _tile *tile;
  theloc.x = -1;
  theloc.y = -1;
  d = 999;
  y = 0;
  while (y < worldmap.h)
  {
    x = 0;
    while (x < worldmap.l)
    {
      tile = &worldmap.tile[x][y];
      if (tile->is_building())
      {  //if there is a building here
        a = 0;
        if (tile->get_owner() == -1)
        {  //if it is an enemy building
          a = 1;
        }
        else if (player[tile->get_owner()].team != player[plyr].team)
        {
          a = 1;
        }
        t = any_unit_here(x, y);
        if (t != -1)
        {
          if (t / 100 == plyr)
          {  //if there is a friendly unit on it
            if ((x != tx) || (y != ty))
            {  //if it isn't this unit
							if (unitstats[player[plyr].unit[t % 100].type].cancapture)
              {  //if that unit is already taking care of that building
                a = 0;
              }
            }
          }
        }
        if (a == 1)
        {
          t = tile_distance(x, y, tx, ty);
          if (t < d)
          {
            theloc.x = x;
            theloc.y = y;
            d = t;
          }
        }
      }
      x++;
    }
    y++;
  }
  return theloc;
}


_loc unhealthy_unit_nearby(_unit *u, int tx, int ty)
{
  int i, j, q;
  _loc d;
  d.x = -1;
  d.y = -1;
  j = ty - 9;
  while (j <= ty + 9)
  {
    i = tx - 9;
    while (i <= tx + 9)
    {
      if ((i != tx) || (j != ty))
      {
        q = any_unit_here(i, j);
        if ((q != -1) && (worldmap.tile[i][j].get_step() <= u->move))
        {  //if there is a unit here and it is in range
          if (q / 100 == u->color)
          {
            if (player[u->color].unit[q % 100].type == u->type)
            {  //if it is the same unit type
              if (player[u->color].unit[q % 100].health < 7)
              {  //if it could benefit from a merge
                d.x = i;
                d.y = j;
                return d;
              }
            }
          }
        }
      }
      i++;
    }
    j++;
  }
  return d;   
}


_loc closest_beach_for_lander(int plyr, int tx, int ty)
{
  unsigned int y;
  _loc d;
  vector<_unit*> goodlander;
  d.x = -1;
  d.y = -1;
  y = 0;
  while (y < lander.size())
  {
    if ((lander[y]->load[0].loaded == 0) || (lander[y]->load[1].loaded == 0))
    {  //create a vector of pointers to good (can load) landers
      goodlander.push_back(lander[y]);
    }
    y++;
  }
  if (goodlander.size() == 0)
  {  //if there aren't any landers, don't bother looking for a beach for them
    return d;
  }
  worldmap.backup_pathmap();
  y = 0;
  while (y < goodlander.size())
  {
    worldmap.create_pathmap(plyr, unitstats[goodlander[y]->type].movetype, goodlander[y]->tilex, goodlander[y]->tiley);
    d = closest_beach_for_lander(plyr, tx, ty, goodlander[y]);
    if (d.x != -1)
    {
      worldmap.restore_pathmap();
      return d;
    }
    y++;
  }
  worldmap.restore_pathmap();
  return d;
}

_loc closest_beach_for_lander(int plyr, int tx, int ty, _unit *thelander)
{  //assumes the pathmap has already been filled for this lander
  int x, y, t;
  int r = 0;
  _loc d;
  vector<_aipath> q;
  d.x = -1;
  d.y = -1;
  while ((r < intmax(worldmap.l, worldmap.h)) && (d.x == -1))
  {  //start looking for a good beach at 1 distance away and move outward
    y = ty - r;
    while (y <= ty + r)
    {
      x = tx - r;
      while (x <= tx + r)
      {
        if ((worldmap.in_bounds(x, y)) && (tile_distance(x, y, tx, ty) == r))
        {
          t = worldmap.tile[x][y].get_set();
          if (t == 3)  //3 is the set for shoal (beach) tiles
          {
            if (worldmap.tile[x][y].get_step() < 999)
            {  //if the lander can get to this beach
              d.x = x;
              d.y = y;
              return d;
            }
          }
        }
        x++;
      }
      y++;
    }
    r++;
  }
  return d;
}


_loc closest_enemy_beach(int plyr, int ox, int oy, bool mountain_climber)
{  //if mountain_climber, then the lander is carrying all infantries or mechs
  vector<_loc> closest_ones;
  vector<_loc> viable;
  vector<_aipath> q;
  _loc d;
  _loc best;
  _tile *tile;
  unsigned int i = 0, j = 0;
  int b;
  int t;
  int wtype;
  closest_ones.reserve(10);
  viable.reserve(5);
  if (mountain_climber == 1)
  {
    wtype = M_INFANTRY;
  }
  else
  {
    wtype = M_TREAD;
  }
  while (i < beaches.size())
  {
    tile = &worldmap.tile[beaches[i].loc[0].x][beaches[i].loc[0].y];
    if (tile->get_step() < 999)
    {  //if the lander can reach this beach (and, therefore, all connected beaches)
      b = 999;
      j = 0;
      d.x = -1;
      d.y = -1;
      while (j < beaches[i].loc.size())
      {  //put the closest beach to the lander in this set into the closest_ones vector
        tile = &worldmap.tile[beaches[i].loc[j].x][beaches[i].loc[j].y];
        t = tile->get_step();
        if ((t < b) && (can_unload_from_here(beaches[i].loc[j].x, beaches[i].loc[j].y, mountain_climber) == 1))
        {
          if (tile->unit_here() == -1)
          {
            b = t;
            d.x = beaches[i].loc[j].x;
            d.y = beaches[i].loc[j].y;
          }
        }
        j++;
      }
      if (d.x != -1)
      {
        closest_ones.push_back(d);
      }
    }
    i++;
  }
  i = 0;
  while (i < closest_ones.size())
  {  //from these _locs, find ones that lead to an enemy building or unit
    d = reachable_enemy_building(plyr, wtype, closest_ones[i].x, closest_ones[i].y, mountain_climber);
    if (d.x != -1)
    {
      viable.push_back(closest_ones[i]);
    }
    i++;
  }
  best.x = -1;
  best.y = -1;
  b = 999;
  i = 0;
  while (i < viable.size())
  {
    tile = &worldmap.tile[viable[i].x][viable[i].y];
    if (tile->get_step() < b)
    {  //find the viable beach location closest to the lander
      best = viable[i];
      b = tile->get_step();
    }
    i++;
  }
  return best;
}

_loc reachable_friendly_beach(int plyr, int u, int ox, int oy, int moves)
{
  _loc d;
  _loc temp;
  _tile *tile;
  d.x = -1;
  d.y = -1;
  unsigned int i = 0, j = 0;
  int b;
  int t;

  b = moves + 1;
  while (i < beaches.size())
  {
    j = 0;
    while (j < beaches[i].loc.size())
    {
      tile = &worldmap.tile[beaches[i].loc[j].x][beaches[i].loc[j].y];
      if ((tile->is_building() == false) && (any_unit_here(beaches[i].loc[j].x, beaches[i].loc[j].y) == -1))
      {  //if there is not a building here (which must be a port) or a unit here
        t = tile->get_step();
        if (t < b)
        {
          temp = reachable_friendly_building(plyr, M_MECH, beaches[i].loc[j].x, beaches[i].loc[j].y);
          if (temp.x != -1)
          {
            b = t;
            d.x = beaches[i].loc[j].x;
            d.y = beaches[i].loc[j].y;
          }
        }
      }
      j++;
    }
    i++;
  }
  return d;
}

_loc reachable_enemy_building(int plyr, int mtype, int ox, int oy, bool cap)
{
  return reachable_enemy_building(plyr, mtype, ox, oy, cap, 0);
}

_loc reachable_enemy_building(int plyr, int mtype, int ox, int oy, bool cap, int non_city)
{  //if cap == 1 it counts neutral buildings as enemies
  int x, y, z;
  _loc d;
  d.x = -1;
  d.y = -1;
  y = 0;
  while (y < worldmap.h)
  {
    x = 0;
    while (x < worldmap.l)
    {
      worldmap.tile[x][y].set_temp(999);
      x++;
    }
    y++;
  }
  if (enemy_building(plyr, ox, oy, cap, non_city) == 1)
  {
    d.x = ox;
    d.y = oy;
    return d;
  }
  z = 0;
  while ((d.x == -1) && (z < 4))
  {
    switch(z)
    {
      case 0:
        x = ox + 1;
        y = oy;
        break;
      case 1:
        x = ox - 1;
        y = oy;
        break;
      case 2:
        x = ox;
        y = oy - 1;
        break;
      case 3:
        x = ox;
        y = oy + 1;
        break;
    }
    if (worldmap.in_bounds(x, y))
    {
      d = reachable_enemy_building_recursive(plyr, mtype, x, y, cap, non_city, 0);
    }
    z++;
  }
  return d;
}

_loc reachable_enemy_building_recursive(int plyr, int mtype, int ox, int oy, bool cap, int non_city, int moves)
{
  _loc d;
  int m = moves_needed(mtype, ox, oy, plyr);
  int x, y, z;
  d.x = -1;
  d.y = -1;
  moves += m;
  if ((moves >= worldmap.tile[ox][oy].get_temp()) || (m == 99))
  {
    return d;
  }
  worldmap.tile[ox][oy].set_temp(moves);
  if (enemy_building(plyr, ox, oy, cap, non_city) == 1)
  {
    d.x = ox;
    d.y = oy;
    return d;
  }
  z = 0;
  while ((d.x == -1) && (z < 4))
  {
    switch(z)
    {
      case 0:
        x = ox + 1;
        y = oy;
        break;
      case 1:
        x = ox - 1;
        y = oy;
        break;
      case 2:
        x = ox;
        y = oy - 1;
        break;
      case 3:
        x = ox;
        y = oy + 1;
        break;
    }
    if (worldmap.in_bounds(x, y))
    {
      d = reachable_enemy_building_recursive(plyr, mtype, x, y, cap, non_city, moves);
    }
    z++;
  }
  return d;
}

_loc reachable_friendly_building(int plyr, int mtype, int ox, int oy)
{
  int x, y, z;
  _loc d;
  d.x = -1;
  d.y = -1;
  y = 0;
  while (y < worldmap.h)
  {
    x = 0;
    while (x < worldmap.l)
    {
      worldmap.tile[x][y].set_temp(999);
      x++;
    }
    y++;
  }
  if (friendly_building(plyr, ox, oy) == 1)
  {
    d.x = ox;
    d.y = oy;
    return d;
  }
  z = 0;
  while ((d.x == -1) && (z < 4))
  {
    switch(z)
    {
      case 0:
        x = ox + 1;
        y = oy;
        break;
      case 1:
        x = ox - 1;
        y = oy;
        break;
      case 2:
        x = ox;
        y = oy - 1;
        break;
      case 3:
        x = ox;
        y = oy + 1;
        break;
    }
    if (worldmap.in_bounds(x, y))
    {
      d = reachable_friendly_building_recursive(plyr, mtype, x, y, 0);
    }
    z++;
  }
  return d;
}

_loc reachable_friendly_building_recursive(int plyr, int mtype, int ox, int oy, int moves)
{
  _loc d;
  int m = moves_needed(mtype, ox, oy, plyr);
  int x, y, z;
  d.x = -1;
  d.y = -1;
  moves += m;
  if ((moves >= worldmap.tile[ox][oy].get_temp()) || (m == 99))
  {
    return d;
  }
  worldmap.tile[ox][oy].set_temp(moves);
  if (friendly_building(plyr, ox, oy) == 1)
  {
    d.x = ox;
    d.y = oy;
    return d;
  }
  z = 0;
  while ((d.x == -1) && (z < 4))
  {
    switch(z)
    {
      case 0:
        x = ox + 1;
        y = oy;
        break;
      case 1:
        x = ox - 1;
        y = oy;
        break;
      case 2:
        x = ox;
        y = oy - 1;
        break;
      case 3:
        x = ox;
        y = oy + 1;
        break;
    }
    if (worldmap.in_bounds(x, y))
    {
      d = reachable_friendly_building_recursive(plyr, mtype, x, y, moves);
    }
    z++;
  }
  return d;
}

bool enemy_building(int plyr, int x, int y, bool cap)
{
  return enemy_building(plyr, x, y, cap, 0);
}

bool enemy_building(int plyr, int x, int y, bool cap, int non_city)
{
  _tile *tile = &worldmap.tile[x][y];
  if (tile->is_building() == false)
  {
    return false;
  }
  if (tile->owned_by(plyr))
  {
    return false;
  }
  if ((!cap) && (tile->owned_by(-1)))
  {
    return false;
  }
  else if (tile->owned_by(-1))
  {
    return true;
  }
  if (player[tile->get_owner()].team == player[plyr].team)
  {
    return false;
  }
  if ((non_city == 1) && (tile->is_building(CITY)))
  {
    return false;
  }
  return true;
}

int friendly_building(int plyr, int x, int y)
{
  if (worldmap.tile[x][y].owned_by(plyr) == true)
  {
    return 1;
  }
  return 0;
}

_loc unloadable_point(int x, int y, int mountains)
{
  _loc i;
  i.x = -1;
  i.y = -1;
  return unloadable_point(x, y, mountains, i);
}

_loc unloadable_point(int x, int y, int mountains, _loc ignore)
{
  int a, b, i, m;
  _loc d;
  d.x = -1;
  d.y = -1;
  if (mountains == 1)
  {
    m = M_MECH;
  }
  else
  {
    m = M_TREAD;
  }
  i = 0;
  while (i < 4)
  {
    switch(i)
    {
      case 0:
        a = x - 1;
        b = y;
        break;
      case 1:
        a = x + 1;
        b = y;
        break;
      case 2:
        a = x;
        b = y - 1;
        break;
      case 3:
        a = x;
        b = y + 1;
        break;
    }
    if ((ignore.x != a) || (ignore.y != b))
    {  //if it is not the ignored location
      if (any_unit_here(a, b) == -1)
      {  //if there is no unit here to obstruct unloading
        if (worldmap.in_bounds(a, b))
        {
          if (moves_needed(m, a, b, 0) < 99)
          {  //if the unit to be unloaded can actually go on this tile
            d.x = a;
            d.y = b;
            return d;
          }
        }
      }
    }
    i++;
  }
  return d;
}

_loc loc_for_ranged_attack(_unit *u, int ox, int oy, int rmin, int rmax)
{
  int x, y, rx, ry;
  int dist = 999;
  _loc t;
  _aitarget target;

  y = oy - u->move;
  if (y < 0) y = 0;
  while ((y <= oy + u->move) && (y < worldmap.h))
  {
    x = ox - u->move;
    while ((x <= ox + u->move) && (x < worldmap.l))
    {
      if (worldmap.tile[x][y].get_step() <= u->move)
      {  //if the unit can move here
        ry = y - rmax;
        if (ry < 0) ry = 0;  //see if it can hit something from here
        while ((ry <= y + rmax) && (ry < worldmap.h))
        {
          rx = x - rmax;
          if (rx < 0) rx = 0;
          while ((rx <= x + rmax) && (rx < worldmap.l))
          {
            dist = tile_distance(x, y, rx, ry);
            if ((dist >= rmin) && (dist <= rmax))
            {  //make sure the location to be checked is in firing range
              target = aitarget_here(u, rx, ry);
              if (target.type != -1)
              {
                t.x = x;  //if it can hit something from here, move here
                t.y = y;
                return t;
              }
            }
            rx++;
          }
          ry++;
        }
      }
      x++;
    }
    y++;
  }
  t.x = -1;
  t.y = -1;
  return t;
}

bool check_ranged_attack_here(int plyr, int n, int x, int y)
{
  _loc d, l;
  _unit* u = &player[plyr].unit[n];
  int z = any_unit_here(x, y);

	l.x = x;
	l.y = y;
  if ((z != -1) && (z / 100 != plyr))
  {
    if (type_can_attack(u, &player[z / 100].unit[z % 100]))
    {
      d = closest_move_to_destination(u->color, u->number, unitstats[u->type].movetype, u->move, u->tilex, u->tiley, l, u->rangemax);
      if (d.x == -1)
      {
        d = closest_move_to_destination(u->color, u->number, unitstats[u->type].movetype, u->move, u->tilex, u->tiley, l, u->rangemin);
        if (d.x == -1)
        {
          return false;
        }
      }
      return true;
    }
  }
  return false;
}

_loc closest_move(int plyr, int u, int mtype, int ox, int oy, int dx, int dy, int ignore)
{
  _loc closest;
  _loc t;
  vector<_loc> loclist;
  vector<_aipath> b;
  int x, y, moves, tx, ty, r;
  if ((ox == dx) && (oy == dy))
  {
    closest.x = ox;
    closest.y = oy;
    return closest;
  }
  b = best_aipath(plyr, mtype, ox, oy, dx, dy);
  loclist.push_back(t);
  loclist[0].x = ox;
  loclist[0].y = oy;
  x = b.size();
  tx = ox;
  ty = oy;
  moves = player[plyr].unit[u].move;

  y = 0;
  while ((y < x) && (moves > 0))
  {  //put all move locations in one vector of _loc's
    switch(b[y].dir)
    {
      case UP:
        ty--;
        break;
      case DOWN:
        ty++;
        break;
      case LEFT:
        tx--;
        break;
      case RIGHT:
        tx++;
        break;
    }
    moves -= moves_needed(mtype, tx, ty, plyr);
    if (moves >= 0)  //make sure the final 'best' location is reachable this turn
    {  //because if it isn't, the game will crash quite horribly
      loclist.push_back(t);
      loclist[y + 1].x = tx;
      loclist[y + 1].y = ty;
      y++;
    }
  }
  y = loclist.size() - 1;
  t.x = loclist[y].x;
  t.y = loclist[y].y;
  if (any_unit_here(t.x, t.y) == -1)
  {
    closest.x = t.x;
    closest.y = t.y;
    return closest;
  }  //if it gets past this point, the best location had a unit on it
  moves = player[plyr].unit[u].move;
  r = 1;
  while (r <= moves)
  {
    y = t.y - r;
    if (y < 0) y = 0;
    while ((y <= t.y + r) && (y < worldmap.h))
    {
      x = t.x - r;
      if (x < 0) x = 0;
      while ((x <= t.x + r) && (x < worldmap.l))
      {
        if (tile_distance(x, y, t.x, t.y) == r)
        {  //if this tile is r away from the center
          if (worldmap.tile[x][y].get_step() <= moves)
          {  //if this tile is reachable by the unit
            if (any_unit_here(x, y) == -1)
            {  //if there is no other unit here
              closest.x = x;  //then this is the best tile to go to
              closest.y = y;
              return closest;
            }
          }
        }
        x++;
      }
      y++;
    }
    r++;
  }
  closest.x = ox;
  closest.y = oy;
  return closest;
}

_loc best_pathmap_loc(int tx, int ty)
{
  _loc a, b, c, d, best;
  int ia, ib, ic, id, ibest;
  ibest = ia = ib = ic = id = 999;
  d.x = -1;
  d.y = -1;
  best = a = b = c = d;
  if (tx > 0)
  {
    a.x = tx - 1;
    a.y = ty;
    ia = worldmap.tile[a.x][a.y].get_step();
  }
  if (tx < worldmap.l)
  {
    b.x = tx + 1;
    b.y = ty;
    ib = worldmap.tile[b.x][b.y].get_step();
  }
  if (ty > 0)
  {
    c.x = tx;
    c.y = ty - 1;
    ic = worldmap.tile[c.x][c.y].get_step();
  }
  if (ty < worldmap.h)
  {
    d.x = tx;
    d.y = ty + 1;
    id = worldmap.tile[d.x][d.y].get_step();
  }
  if (ia < ibest)
  {
    best = a;
    ibest = ia;
  }
  if (ib < ibest)
  {
    best = b;
    ibest = ib;
  }
  if (ic < ibest)
  {
    best = c;
    ibest = ic;
  }
  if (id < ibest)
  {
    best = d;
    ibest = id;
  }
  return best;
}

bool friendly_unit_maker_here(int tx, int ty, int plyr)
{
  if (worldmap.tile[tx][ty].is_unit_producing())
  {
    if (worldmap.tile[tx][ty].owned_by(plyr))
    {
      return true;
    }
  }
  return false;
}

_loc closest_teleport_to_destination(int plyr, int mtype, int destx, int desty, int maxmoves)
{
	int best = 99;
	unsigned int i = 0;
	_loc b;
	b.x = -1;
	b.y = -1;
	worldmap.backup_pathmap();
	worldmap.create_limited_pathmap(plyr, mtype, destx, desty, maxmoves);
	
	while (i < teleports.size())
	{
		if (worldmap.tile[teleports[i].x][teleports[i].y].get_step() < best)
		{
			if (any_unit_here(teleports[i].x, teleports[i].y) == -1)
			{  //make sure AI units don't teleport on top of each other
				best = worldmap.tile[teleports[i].x][teleports[i].y].get_step();
				b = teleports[i];
			}
		}
		i++;
	}
	
	worldmap.restore_pathmap();
	return b;
}
