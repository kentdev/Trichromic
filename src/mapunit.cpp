#include "empire.h"

setunit mapunit[200];

void draw_units(int scrollx, int scrolly);
void draw_unit(int num, int x, int y);
void new_unit(int x, int y, int type, int player);
void new_unit(int x, int y, int type, int player, int sound);
void clear_all_units();
void remove_unit(int n);
bool munit_exists(int playernum);
int find_free_unit();
int check_for_unit(int x, int y);
int units_for_player(int plyr);

void draw_units(int scrollx, int scrolly)
{
  int i = 0;
  while (i < 200)
  {
    if (mapunit[i].exist == 1)
    {
      if ((mapunit[i].x >= scrollx) && (mapunit[i].x < scrollx + XTILES))
      {
        if ((mapunit[i].y >= scrolly) && (mapunit[i].y < scrolly + YTILES))
        {
					
          draw_unit(i, (mapunit[i].x - scrollx) * MAP_TILE_SIZE, (mapunit[i].y - scrolly) * MAP_TILE_SIZE);
        }
      }
    }
    i++;
  }
}

void clear_all_units()
{
  int i = 0;
  while (i < 200)
  {
    mapunit[i].exist = 0;
    i++;
  }
}

void draw_unit(int num, int x, int y)
{
  setunit *t = &mapunit[num];
	switch(t->player)
	{
		case 0:
			buffer_draw_tinted_sprite(units[t->type].bmp(0), x, y, ol::Rgba(RED, 150));
			break;
		case 1:
			buffer_draw_tinted_sprite(units[t->type].bmp(0), x, y, ol::Rgba(BLUE, 150));
			break;
		case 2:
			buffer_draw_tinted_sprite(units[t->type].bmp(0), x, y, ol::Rgba(MGREEN, 150));
			break;
		case 3:
			buffer_draw_tinted_sprite(units[t->type].bmp(0), x, y, ol::Rgba(DYELLOW, 150));
			break;
		default:
			buffer_draw_tinted_sprite(units[t->type].bmp(0), x, y, ol::Rgba(BLACK, 150));
			break;
	}
//  buffer_draw_sprite(mapunits[(t.type * 4) + t.player], x, y);
}

void new_unit(int x, int y, int type, int player)
{
  new_unit(x, y, type, player, 1);
}

void remove_unit(int n)
{
  while (n < 199)
  {
    mapunit[n] = mapunit[n + 1];
    n++;
  }
}

void new_unit(int x, int y, int type, int player, int sound)
{
  int n;
  int z = check_for_unit(x, y);
  if (z == -1)  //if no unit in this spot
  {
    n = find_free_unit();  //initialize a new one
    if (n != -1)
    {
      if (units_for_player(player) < 50)
      {
        if (sound == 1)
        {
          play_sound(S_UNITDOWN);
        }
        mapunit[n].exist = 1;
        mapunit[n].x = x;
        mapunit[n].y = y;
        mapunit[n].type = type;
        mapunit[n].player = player;
      }
    }
  }
  else  //if a unit is already there, redefine its attributes to what was selected
  {
    if (!((mapunit[z].type == type) && (mapunit[z].player == player)))
    {  //if a unit of the exact same type is here already, it won't do anything
      if (sound == 1)
      {
        play_sound(S_UNITDOWN);
      }
      mapunit[z].type = type;
      mapunit[z].player = player;
    }
  }
}

int check_for_unit(int x, int y)
{
  int i = 0;
  while (i < 200)
  {
    if (mapunit[i].exist == 1)
    {
      if ((mapunit[i].x == x) && (mapunit[i].y == y))
      {
        return i;
      }
    }
    i++;
  }
  return -1;
}

int find_free_unit()
{
  int i = 0;
  while (i < 200)
  {
    if (mapunit[i].exist == 1)
    {
      i++;
    }
    else
    {
      return i;
    }
  }
  return -1;
}

bool munit_exists(int playernum)
{
  int i = 0;
  while (i < 200)
  {
    if (mapunit[i].exist == 1)
    {
      if (mapunit[i].player == playernum)
      {
        return true;
      }
    }
    i++;
  }
  return false;
}

int units_for_player(int plyr)
{
  int n = 0;
  int i = 0;
  while (i < 200)
  {
    if (mapunit[i].exist == 1)
    {
      if (mapunit[i].player == plyr)
      {
        n++;
      }
    }
    i++;
  }
  return n;
}
