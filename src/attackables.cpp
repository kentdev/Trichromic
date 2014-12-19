#include "empire.h"
#include "logic.h"

vector<_loc> find_attackables(_unit *u)
{
  vector<_loc> a;
  _loc t;
  int x, y, i, max, min;
  a.resize(0);
  a.reserve(4);
  if (unitstats[u->type].attacktype == DIRECT)
  {  //search for attackable units on adjacent spaces
    i = 0;
    while (i < 4)
    {
      switch(i)
      {
        case 0:
          x = u->tilex + 1;
          y = u->tiley;
          break;
        case 1:
          x = u->tilex - 1;
          y = u->tiley;
          break;
        case 2:
          x = u->tilex;
          y = u->tiley - 1;
          break;
        case 3:
          x = u->tilex;
          y = u->tiley + 1;
          break;
      }
      if ((x >= 0) && (y >= 0) && (x < worldmap.l) && (y < worldmap.h) && (attackable_unit_here(u, x, y, true) != -1))
      {  //finds all attackables, including teammates
        t.x = x;
        t.y = y;
        a.push_back(t);
      }
      i++;
    }
  }
  else if (unitstats[u->type].attacktype == RANGED)
  {  //search for attackable units anywhere within range
    max = u->rangemax;
    min = u->rangemin;
    y = u->tiley - max;
    if (y < 0) y = 0;
    while ((y <= u->tiley + max) && (y < worldmap.h))
    {
      x = u->tilex - max;
      if (x < 0) x = 0;
      while ((x <= u->tilex + max) && (x < worldmap.l))
      {
        i = tile_distance(x, y, u->tilex, u->tiley);
        if ((i >= min) && (i <= max))
        {
          if (attackable_unit_here(u, x, y, true) != -1)
          {
            t.x = x;
            t.y = y;
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

void draw_attackables(vector<_loc> locations)
{
  static int r = 0;
  int i, m, t1, t2, u;
  int d = MAP_TILE_SIZE;
  r += 3;
  if (r > 256)
  {
    r -= 256;
  }
  i = 0;
  m = locations.size();
  while (i < m)
  {  //rotates a targeting thingy over each attackable enemy
    t1 = (locations[i].x - worldmap.scroll_x) * MAP_TILE_SIZE + worldmap.offset_x;
    t2 = (locations[i].y - worldmap.scroll_y) * MAP_TILE_SIZE + worldmap.offset_y;
    u = worldmap.tile[locations[i].x][locations[i].y].unit_here();
    if (player[u / 100].team == player[pturn].team)
    {  //if this attackable unit is on an allied team
      buffer_hline(t1, t2, t1 + 10, GREEN, 0.9);
      buffer_hline(t1, t2 + 1, t1 + 10, GREEN, 0.9);
      buffer_hline(t1 + (d - 11), t2, t1 + (d - 1), GREEN, 0.9);
      buffer_hline(t1 + (d - 11), t2 + 1, t1 + (d - 1), GREEN, 0.9);
      buffer_hline(t1, t2 + (d - 2), t1 + 10, GREEN, 0.9);
      buffer_hline(t1, t2 + (d - 1), t1 + 10, GREEN, 0.9);
      buffer_hline(t1 + (d - 11), t2 + (d - 2), t1 + (d - 1), GREEN, 0.9);
      buffer_hline(t1 + (d - 11), t2 + (d - 1), t1 + (d - 1), GREEN, 0.9);
      buffer_vline(t1, t2, t2 + 10, GREEN, 0.9);
      buffer_vline(t1 + 1, t2, t2 + 10, GREEN, 0.9);
      buffer_vline(t1 + (d - 2), t2, t2 + 10, GREEN, 0.9);
      buffer_vline(t1 + (d - 1), t2, t2 + 10, GREEN, 0.9);
      buffer_vline(t1, t2 + (d - 11), t2 + (d - 1), GREEN, 0.9);
      buffer_vline(t1 + 1, t2 + (d - 11), t2 + (d - 1), GREEN, 0.9);
      buffer_vline(t1 + (d - 2), t2 + (d - 11), t2 + (d - 1), GREEN, 0.9);
      buffer_vline(t1 + (d - 1), t2 + (d - 11), t2 + (d - 1), GREEN, 0.9);
    }  //(draws a green square-like target thingy)
    else
    {
      buffer_rotate_sprite(glb.bmp(12), t1 + MAP_TILE_SIZE / 2, t2 + MAP_TILE_SIZE / 2, r);
    }
    i++;
  }
}

int attackable_unit_here(_unit *attacker, int tx, int ty)
{
  return attackable_unit_here(attacker, tx, ty, false);
}

int attackable_unit_here(_unit *attacker, int tx, int ty, bool ignore_teams)
{
  int u = worldmap.tile[tx][ty].unit_here();
  _unit *defender;
  int d;
  
  if (attacker->attackdelay > 0) return -1;
  
  if ((u != -1) && (u / 100 != attacker->color))
  {
    defender = &player[u / 100].unit[u % 100];
    if (player[attacker->color].team == player[defender->color].team)
    {  //if the players are on the same team
      if (!ignore_teams)
      {  //if teams are being recognized
        return -1;
      }
    }
    if (type_can_attack(attacker, defender))
    {  //if the attacker could, in theory, attack the defender
      d = tile_distance(attacker->tilex, attacker->tiley, tx, ty);
      if (unitstats[attacker->type].attacktype == DIRECT)
      {  //if the attacker is direct and next to the defender, it can attack
        if (d == 1)
        {
          return u;
        }
      }
      else if (unitstats[attacker->type].attacktype == RANGED)
      {
        if ((d >= attacker->rangemin) && (d <= attacker->rangemax))
        {  //if the attacker has the defender within its firing range, it can attack
          return u;
        }
      }
    }
  }
  return -1;
}

bool type_can_attack(_unit *a, _unit *d)
{
	const short land = 1;
	const short air  = 2;
	const short sea  = 4;
	
	if (a->attackdelay > 0) return false;
	
	short c = unitstats[a->type].canhit;
	
	switch(unitstats[d->type].basetype)
	{
		case LAND:
			if (c & land) return true;
			break;
		case AIR:
			if (c & air) return true;
			break;
		case SEA:
			if (c & sea) return true;
			break;
	}
  return false;
}
