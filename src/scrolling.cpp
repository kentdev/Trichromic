#include "empire.h"
#include "logic.h"

void check_scrolling()
{
  check_scrolling(-1, -1);
}

void check_scrolling(int forcex, int forcey)
{
  static int increment = MAP_TILE_SIZE / global.scroll_divider;
  if (!(mouse_b & 1))
  {  //the screen shouldn't scroll while the mouse button is down
    if (key[KEY_LEFT])
    {
      if (worldmap.scroll_x > 0)
      {
        worldmap.offset_x += increment;
        if (worldmap.offset_x >= MAP_TILE_SIZE)
        {
          worldmap.scroll_x--;
          worldmap.offset_x -= MAP_TILE_SIZE;
        }
      }
    }
    if (key[KEY_RIGHT])
    {
      if (!((worldmap.scroll_x + XTILES + 1 > worldmap.l) && (worldmap.offset_x < increment)))
      {
        worldmap.offset_x -= increment;
        if (worldmap.offset_x < 0)
        {
          worldmap.scroll_x++;
          worldmap.offset_x += MAP_TILE_SIZE;
        }
      }
    }
    if (key[KEY_DOWN])
    {
      if (!((worldmap.scroll_y + YTILES + 1 > worldmap.h) && (worldmap.offset_y < increment)))
      {
        worldmap.offset_y -= increment;
        if (worldmap.offset_y < 0)
        {
          worldmap.scroll_y++;
          worldmap.offset_y += MAP_TILE_SIZE;
        }
      }
    }
    if (key[KEY_UP])
    {
      if (worldmap.scroll_y > 0)
      {
        worldmap.offset_y += increment;
        if (worldmap.offset_y >= MAP_TILE_SIZE)
        {
          worldmap.scroll_y--;
          worldmap.offset_y -= MAP_TILE_SIZE;
        }
      }
    }
  }
  if ((forcex != -1) && (forcey != -1))
  {  //refresh increment because scroll_divider may have changed since last time
    increment = MAP_TILE_SIZE / global.scroll_divider;
    worldmap.scroll_x = forcex;
    worldmap.scroll_y = forcey;
    worldmap.offset_x = 0;
    worldmap.offset_y = 0;
  }  
}

void scroll_to_hq(int plyr)
{
  static bool first = true;
  static int hqx = -1, hqy = -1;
  int x, y;
  if (first)
  {
    y = 0;
    while (y < worldmap.h)
    {
      x = 0;
      while (x < worldmap.l)
      {
        if (worldmap.tile[x][y].is_HQ())
        {
          if (worldmap.tile[x][y].owned_by(plyr))
          {
            hqx = x;
            hqy = y;
            x = worldmap.l;
            y = worldmap.h;
          }
        }
        x++;
      }
      y++;
    }
    first = false;
  }
  if (scroll_to_location(hqx, hqy) == 1)
  {
    var.logicstate = NOTHING_HAPPENING;
    first = true;
  }
}

int scroll_to_location(int x, int y)
{
  static bool first = true;
  static int targetx, targety, increment;
  if (first)
  {
    increment = MAP_TILE_SIZE / global.AI_scroll_divider;
    targetx = x - 8;  //center on chosen tile
    targety = y - 6;
    if (targetx < 0) targetx = 0;  //make sure it doesn't go past the map ends
    if (targetx + XTILES > worldmap.l) targetx = worldmap.l - XTILES;
    if (targety < 0) targety = 0;
    if (targety + YTILES > worldmap.h) targety = worldmap.h - YTILES;
    first = false;
  }
  if (worldmap.scroll_x > targetx)
  {
    worldmap.offset_x += increment;
    if (worldmap.offset_x >= MAP_TILE_SIZE)
    {
      worldmap.scroll_x--;
      worldmap.offset_x -= MAP_TILE_SIZE;
    }
  }
  else if (worldmap.scroll_x < targetx)
  {
    worldmap.offset_x -= increment;
    if (worldmap.offset_x < 0)
    {
      worldmap.scroll_x++;
      worldmap.offset_x += MAP_TILE_SIZE;
    }
  }
  else if ((worldmap.scroll_x == targetx) && (worldmap.offset_x != 0))
  {
    if (worldmap.offset_x < increment)
    {
      worldmap.offset_x = 0;
    }
    else
    {
      worldmap.offset_x -= increment;
    }
  }
  if (worldmap.scroll_y > targety)
  {
    worldmap.offset_y += increment;
    if (worldmap.offset_y >= MAP_TILE_SIZE)
    {
      worldmap.scroll_y--;
      worldmap.offset_y -= MAP_TILE_SIZE;
    }
  }
  else if (worldmap.scroll_y < targety)
  {
    worldmap.offset_y -= increment;
    if (worldmap.offset_y < 0)
    {
      worldmap.scroll_y++;
      worldmap.offset_y += MAP_TILE_SIZE;
    }
  }
  else if ((worldmap.scroll_y == targety) && (worldmap.offset_y != 0))
  {
    if (worldmap.offset_y < increment)
    {
      worldmap.offset_y = 0;
    }
    else
    {
      worldmap.offset_y -= increment;
    }
  }
	
	if ((worldmap.scroll_x == 0) && (worldmap.offset_x > 0))
	{
		worldmap.offset_x = 0;
	}
	if ((worldmap.scroll_y == 0) && (worldmap.offset_y > 0))
	{
		worldmap.offset_y = 0;
	}
  
  if ((worldmap.scroll_x == targetx) && (worldmap.scroll_y == targety) && (worldmap.offset_x == 0) && (worldmap.offset_y == 0))
  {  //if it's reached the target
    first = true;
    return 1;
  }
  return 0;
}
