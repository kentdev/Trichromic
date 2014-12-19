#include "empire.h"
#include "logic.h"

int moves_needed(int mtype, int tx, int ty, int plyr)
{        //Infantry, Tires, Tread, Air, Mech, Sea, Lander
  int movegrid[70] = {1, 1, 1, 1, 1,99,99,  //Buildings        0
                      1, 1, 1, 1, 1,99,99,  //Plains           1
                      1, 1, 1, 1, 1,99, 1,  //Shoal            2
                      1, 3, 2, 1, 1,99,99,  //Forest           3
                      2,99,99, 1, 2,99,99,  //Mountain         4
                      2,99,99, 1, 1,99,99,  //River            5
                      1, 1, 1, 1, 1,99,99,  //Road             6
                     99,99,99, 1,99, 1, 1,  //Sea              7
                      1, 1, 1, 1, 1,99,99,  //Special 1        8
                     99,99,99,99,99,99,99}; //Special 2        9
  int ltype = get_ltype(tx, ty);
  if (ltype == 0)
  {
    if ((mtype == M_SEA) || (mtype == M_LANDER))  //sea units can go to ports
    {
      if (worldmap.tile[tx][ty].is_building(PORT))
      {
        return 1;
      }
    }
  }
	else if ((ltype == 3) && (player[plyr].number == 2))
	{  //pine army movement is uninhibited by forests
		return 1;
	}
  return movegrid[mtype + (ltype * 7)];
}

int get_ltype(int tx, int ty)
{
  int s;
  int z;
  s = worldmap.tile[tx][ty].get_set();
  z = worldmap.tile[tx][ty].get_number();
  if (s == 4)  //buildings
  {
    return 0;
  }
  else if (s == 0)  //land
  {
    if ((z == 2) || (z == 20))  //forest or burnt forest
    {
      return 3;
    }
    else if (z == 4)  //mountain
    {
      return 4;
    }
    else if (z == 3)  //plains
    {
      return 1;
    }
    else  //road
    {
      return 6;
    }
  }
  else if (s == 3)  //shoal
  {
    if (z < 25)
    {
      return 2;
    }
    else  //tiles 25 through 28 are sea tiles, but are transitions from shoals
    {
      return 7;
    }
  }
  else if (s == 1)  //river
  {
    return 5;
  }
  else if (s == 6)  //special
  {
    if (z == 0)  //teleport tile
    {
      return 8;
    }
    else if (z == 1)  //barrier tile
    {
      return 9;
    }
  }
  return 7;  //sea
}

int do_moveselect(int plyr, int u, int tx, int ty)
{
  static int first = 1;
  static int lastx;
  static int lasty;
  int i;
  int dir;
  int findpath;
  int q;
  
  if ((tx == -1) && (ty == -1))  //if -1 for tx and ty, reset the static ints
  {
    movelist.resize(0);
    u = -1;
    first = 1;
    return 0;
  }
  _unit *theunit = &player[plyr].unit[u];
  if (first == 1)
  {
    lastx = theunit->tilex;
    lasty = theunit->tiley;
    findpath = 1;
    first = 0;
  }
  else if ((tx != lastx) || (ty != lasty))
  {
    findpath = 0;
    if (tile_distance(lastx, lasty, tx, ty) == 1)
    {  //if the mouse hasn't skipped a tile or anything like that
      if (list_distance(theunit) + moves_needed(unitstats[theunit->type].movetype, tx, ty, plyr) <= theunit->move)
      {  //if the movement is still within the unit's range
        if (worldmap.tile[tx][ty].get_step() <= theunit->move)
        {
          if (ty == lasty + 1)
          {
            dir = DOWN;
          }
          if (ty == lasty - 1)
          {
            dir = UP;
          }
          if (tx == lastx + 1)
          {
            dir = RIGHT;
          }
          if (tx == lastx - 1)
          {
            dir = LEFT;
          }
          movelist.push_back(dir);
          if (move_overlap(theunit))
          {
            findpath = 1;
          }
        }
      }
      else
      {
        findpath = 1;
      }
    }
    else
    {  //if the path is not connected
      findpath = 1;
    }
    if ((tx == theunit->tilex) && (ty == theunit->tiley))
    {  //if the mouse is over the unit's original position, reset the movelist
      findpath = 0;  //also, don't bother getting a best path
      movelist.resize(0);
    }
    if ((findpath == 1) && (worldmap.tile[tx][ty].get_step() <= theunit->move))
    {  //find the best path from (tx, ty) to the unit's location
      vector<_aipath> t = best_aipath(plyr, unitstats[theunit->type].movetype, theunit->tilex, theunit->tiley, tx, ty);
      q = t.size();
      i = 0;
      movelist.resize(0);
      while (i < q)
      {
        movelist.push_back(t[i].dir);
        i++;
      }
    }
    if (worldmap.tile[tx][ty].get_step() <= theunit->move)
    {
      lastx = tx;
      lasty = ty;
    }
  }
  show_movable_tiles(theunit);
  draw_path(theunit);
  theunit->draw(false);
  return u;
}

void show_movable_tiles(_unit *u, float alpha)
{
	int i, j;
	int ymin = u->tiley - u->move;
	int ymax = u->tiley + u->move;
	int xmin = u->tilex - u->move;
	int xmax = u->tilex + u->move;
	int x, y, z;
	j = ymin;
	if (j < 0) j = 0;
	while ((j <= ymax) && (j < worldmap.h))
	{
		i = xmin;
		while ((i <= xmax) && (i < worldmap.l))
		{
			if (worldmap.tile[i][j].get_step() <= u->move)
			{
				//this unit can be moved through, but only highlight the tile if the unit can actually stop on it
				z = any_unit_here(i, j);
				if (z == -1 ||
						((z / 100 == u->color) &&
							((can_merge(u, &player[z / 100].unit[z % 100])) || (player[z / 100].unit[z % 100].can_load_unit(u) == 1)) ) )
				{
					x = (i - worldmap.scroll_x) * MAP_TILE_SIZE + worldmap.offset_x;
					y = (j - worldmap.scroll_y) * MAP_TILE_SIZE + worldmap.offset_y;
					buffer_rectfill(x, y, x + MAP_TILE_SIZE, y + MAP_TILE_SIZE, WHITE, alpha);
				}
			}
			i++;
		}
		j++;
	}
}

void draw_path(_unit *u)
{
  unsigned int i = 0;
  int tx = u->tilex;
  int ty = u->tiley;
  int x = (u->tilex - worldmap.scroll_x) * MAP_TILE_SIZE + worldmap.offset_x;
  int y = (u->tiley - worldmap.scroll_y) * MAP_TILE_SIZE + worldmap.offset_y;
  buffer_rectfill(x, y, x + MAP_TILE_SIZE, y + MAP_TILE_SIZE, YELLOW, 0.66);
  while (i < movelist.size())
  {
    switch(movelist[i])
    {
      case LEFT:
        tx -= 1;
        break;
      case RIGHT:
        tx += 1;
        break;
      case UP:
        ty -= 1;
        break;
      case DOWN:
        ty += 1;
        break;
    }
    x = (tx - worldmap.scroll_x) * MAP_TILE_SIZE + worldmap.offset_x;
    y = (ty - worldmap.scroll_y) * MAP_TILE_SIZE + worldmap.offset_y;
    buffer_rectfill(x, y, x + MAP_TILE_SIZE, y + MAP_TILE_SIZE, YELLOW, 0.47);
    i++;
  }
}

int list_distance(_unit *u)
{
  unsigned int i = 0;
  int moves = 0;
  int x = u->tilex;
  int y = u->tiley;
  while (i < movelist.size())
  {
    switch(movelist[i])
    {
      case UP:
        y -= 1;
        break;
      case DOWN:
        y += 1;
        break;
      case LEFT:
        x -= 1;
        break;
      case RIGHT:
        x += 1;
        break;
    }
    moves += moves_needed(unitstats[u->type].movetype, x, y, u->color);
    i++;
  }
  return moves;
}

bool move_overlap(_unit *u)
{
  unsigned int i = 0, j = 0;
  int tx, ty;
  int x = u->tilex;
  int y = u->tiley;
  vector<_loc> a;
  _loc l;
  tx = x;
  ty = y;
  while (i < movelist.size())
  {
    switch(movelist[i])
    {
      case UP:
        y -= 1;
        break;
      case DOWN:
        y += 1;
        break;
      case LEFT:
        x -= 1;
        break;
      case RIGHT:
        x += 1;
        break;
    }
    j = 0;
    while (j < a.size())
    {
      if ((x == a[j].x) && (y == a[j].y))
      {
        return true;
      }
      j++;
    }
    l.x = x;
    l.y = y;
    a.push_back(l);
    i++;
  }
  return false;
}
