#include "ranges.h"

extern int moves_needed(int mtype, int tx, int ty);
extern int any_unit_here(int tx, int ty);
extern int attackable_unit_here(_unit *attacker, int tx, int ty, bool ignore_teams);

int tile_distance(int ax, int ay, int bx, int by)
{
  int dx, dy;
  dx = abs(bx - ax);
  dy = abs(by - ay);
  return dx + dy;
}

int enemy_in_range(_unit *u)
{
  int i, j, z;
  int min = u->rangemin;
  int max = u->rangemax;
  j = u->tiley - max;
  if (j < 0) j = 0;
  while ((j <= u->tiley + max) && (j < worldmap.h))
  {
    i = u->tilex - max;
    if (i < 0) i = 0;
    while ((i <= u->tilex + max) && (i < worldmap.l))
    {
      if (tile_distance(u->tilex, u->tiley, i, j) >= min)
      {
        z = attackable_unit_here(u, i, j, true);
        if (z != -1)
        {
          return z;
        }
      }
      i++;
    }
    j++;
  }
  return -1;
}      

void draw_attackrange(_unit *u)
{
	static float alpha = 0.3;
	static float alphachange = 0.01;
	int rangemin, rangemax, d, lx, ly, x, y, max_x, max_y, m;
	bool arange[MAP_MAX_L][MAP_MAX_H];
	
	if (unitstats[u->type].attacktype == RANGED)
	{
  	rangemin = u->rangemin;
  	rangemax = u->rangemax;
  	x = u->tilex - rangemax;
  	y = u->tiley - rangemax;
  	max_x = u->tilex + rangemax + 1;
  	max_y = u->tiley + rangemax + 1;
  	if (max_x > worldmap.scroll_x + XTILES) max_x = worldmap.scroll_x + XTILES;
  	if (max_y > worldmap.scroll_y + YTILES) max_y = worldmap.scroll_y + YTILES;
  	if (x < 0) x = 0;
  	while (x < max_x)
  	{
	    y = u->tiley - rangemax;
    	if (y < 0) y = 0;
    	while (y < max_y)
    	{
	      d = tile_distance(x, y, u->tilex, u->tiley);
      	if ((d >= rangemin) && (d <= rangemax))
      	{
	        lx = (x - worldmap.scroll_x) * MAP_TILE_SIZE + worldmap.offset_x;
        	ly = (y - worldmap.scroll_y) * MAP_TILE_SIZE + worldmap.offset_y;
        	buffer_rectfill(lx, ly, lx + MAP_TILE_SIZE, ly + MAP_TILE_SIZE, RED, alpha);
      	}
      	y++;
    	}
    	x++;
  	}
	}
	else if ((unitstats[u->type].attacktype == DIRECT) || (unitstats[u->type].name == "Mechanic"))
	{
		m = u->move;
		if (!u->canmove) m = 0;  //if it can't move, it can only attack adjacent tiles
		//worldmap.backup_pathmap();
		//worldmap.create_limited_pathmap(u->color, unitstats[u->type].movetype, u->tilex, u->tiley, m);
		y = 0;
		while (y < worldmap.h)
		{
			x = 0;
			while (x < worldmap.l)
			{
				arange[x][y] = false;
				x++;
			}
			y++;
		}
		
		y = 0;
		while (y < worldmap.h)
		{
			x = 0;
			while (x < worldmap.l)
			{
				if (worldmap.tile[x][y].get_step() < 99)
				{
					if ((any_unit_here(x, y) == -1) || (any_unit_here(x, y) == u->color * 100 + u->number))
					{
						if (x > 0)
						{
							arange[x - 1][y] = true;
						}
						if (x < worldmap.l - 1)
						{
							arange[x + 1][y] = true;
						}
						if (y > 0)
						{
							arange[x][y - 1] = true;
						}
						if (y < worldmap.h - 1)
						{
							arange[x][y + 1] = true;
						}
					}
				}
				x++;
			}
			y++;
		}
		
		y = 0;
		while (y < worldmap.h)
		{
			x = 0;
			while (x < worldmap.l)
			{
				if (arange[x][y])
				{
					lx = (x - worldmap.scroll_x) * MAP_TILE_SIZE + worldmap.offset_x;
        	ly = (y - worldmap.scroll_y) * MAP_TILE_SIZE + worldmap.offset_y;
					buffer_rectfill(lx, ly, lx + MAP_TILE_SIZE, ly + MAP_TILE_SIZE, RED, alpha);
				}
				x++;
			}
			y++;
		}
		
		//worldmap.restore_pathmap();
	}
	alpha += alphachange;
	if ((alpha >= 0.66) || (alpha <= 0.2)) alphachange *= -1;
}
