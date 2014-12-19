#include "empire.h"
#include "logic.h"

vector<_loc> find_unload_locations(_unit *u, int type)
{
  int x, y, i;
  vector<_loc> a;
  _loc t;
  a.reserve(0);
  a.resize(0);
  i = 0;
  while (i < 4)
  {
    switch(i)
    {
      case 0:
        x = u->tilex - 1;
        y = u->tiley;
        break;
      case 1:
        x = u->tilex + 1;
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
    if (worldmap.in_bounds(x, y))
    {
      if (unit_unloadable_here(u->tilex, u->tiley, x, y, type) == true)
      {
        t.x = x;
        t.y = y;
        a.push_back(t);
      }
    }
    i++;
  }
  return a;
}

void show_unload_locations(vector<_loc> locations)
{
  int i = 0;
  int m = locations.size();
  int x, y;
  while (i < m)
  {  //highlight each good unload location
    x = (locations[i].x - worldmap.scroll_x) * MAP_TILE_SIZE + worldmap.offset_x;
    y = (locations[i].y - worldmap.scroll_y) * MAP_TILE_SIZE + worldmap.offset_y;
    buffer_rectfill(x, y, x + MAP_TILE_SIZE, y + MAP_TILE_SIZE, WHITE, 0.66);
    i++;
  }
}

bool unit_unloadable_here(int ox, int oy, int tx, int ty, int type)
{
  bool ok = false;
  
	if (tx < 0) return false;
	if (tx >= worldmap.l) return false;
	if (ty < 0) return false;
	if (ty >= worldmap.h) return false;
  
  if (moves_needed(unitstats[type].movetype, tx, ty, 0) < 99)
  {  //if the unit can walk on this tile
		ok = true;
  }
  if (worldmap.tile[tx][ty].unit_here() != -1)
  {  //can't unload if a unit is on the tile already
    ok = false;
  }
  if (tile_distance(ox, oy, tx, ty) != 1)
  {  //make sure the distance between unloader and unloadee is 1
    ok = false;
  }
  return ok;
}
