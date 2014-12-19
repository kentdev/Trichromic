#include "empire.h"

extern bool translucent_units;

extern void draw_tile(int set, int pic, int x, int y);
extern void init_explode(int x, int y);
extern int get_set(int mapdat);
extern int get_tilenum(int mapdat);
extern int moves_needed(int mtype, int tx, int ty);
extern bool unit_unloadable_here(int ox, int oy, int tx, int ty, int type);
extern int rounded_health(float h);
extern int any_unit_here(int tx, int ty);

unitprops unitstats[MAX_UNITS];

bool shieldgen_nearby(int tx, int ty, int team, bool ignore_shieldstatus = false);
int angle(int dir);
int angle_direction(int rot);

int unit_price(int type, int army);

_unit::_unit()
{
  type = -1;
  color = -1;
  number = -1;
  ready = 0;
  health = -1;
  exists = 0;
  tilex = -1;
  tiley = -1;
  move = -1;
}

void _unit::create(int utype, int tx, int ty, int tcolor, int tnum)
{
  int i;
	unitprops *tmpr = &unitstats[utype];
  move = tmpr->_move;
	rangemin = tmpr->_rangemin;
	rangemax = tmpr->_rangemax;
	price = unit_price(utype, player[tcolor].number);
	
	i = 0;
	while (i < 3)
	{
		attack[i] = tmpr->_attack[i];
		resist[i] = tmpr->_resist[i];
		i++;
	}
	
	resist[0] += armystats[player[tcolor].number].resistfraction[0] - 100;
	resist[1] += armystats[player[tcolor].number].resistfraction[1] - 100;
	resist[2] += armystats[player[tcolor].number].resistfraction[2] - 100;
	if (tmpr->attacktype != -1)
	{
		attack[0] += armystats[player[tcolor].number].attackfraction[0] - 100;
		attack[1] += armystats[player[tcolor].number].attackfraction[1] - 100;
		attack[2] += armystats[player[tcolor].number].attackfraction[2] - 100;
	}
	move += armystats[player[tcolor].number].movemod;
	if (move < 2) move = 2;
	rangemax += armystats[player[tcolor].number].rangemaxmod;
	
  attackdelay = 0;
  shielding = false;
  health = 10;
  x = tx * MAP_TILE_SIZE;
  y = ty * MAP_TILE_SIZE;
	rotation = 0;
  destx = 0;
  desty = 0;
  tilex = tx;
  tiley = ty;
  worldmap.tile[tilex][tiley].set_unit(tcolor, tnum);
  type = utype;
  number = tnum;
  exists = 1;
  color = tcolor;
  direction = RIGHT;
  frame = 0;
  ready = 0;
  moving = 0;
  otherunit = -1;
  load[0].loaded = 0;
  load[0].type = -1;
  load[0].subload[0].loaded = 0;
  load[0].subload[1].loaded = 0;
  load[0].health = -1;
  load[1].loaded = 0;
  load[1].type = -1;
  load[1].health = -1;
  load[1].subload[0].loaded = 0;
  load[1].subload[1].loaded = 0;
  mlist.resize(0);
}

void _unit::do_move()
{
  switch(direction)
  {
    case UP:
      y -= 4;
      break;
    case DOWN:
      y += 4;
      break;
    case LEFT:
      x -= 4;
      break;
    case RIGHT:
      x += 4;
      break;
  }
  tilex = x / MAP_TILE_SIZE;
  tiley = y / MAP_TILE_SIZE;
}

void _unit::do_movement()
{
  if (moving == 1)
  {
    rotation = angle(direction);  //if it's moving, reset its rotation
		if (otherunit != -1)
    {  //this keeps any units passed over from being erased
      worldmap.tile[tilex][tiley].set_unit(otherunit / 100, otherunit % 100);
      worldmap.draw_tiles(tilex - worldmap.scroll_x, tiley - worldmap.scroll_y, 1, 1);
    }
    else
    {
      worldmap.tile[tilex][tiley].set_unit(-1, -1);
    }
    do_move();
    otherunit = worldmap.tile[tilex][tiley].unit_here();
    if (otherunit != -1)
    {
      player[otherunit / 100].unit[otherunit % 100].draw();
    }
    worldmap.tile[tilex][tiley].set_unit(color, number);
    if ((x == destx * MAP_TILE_SIZE) && (y == desty * MAP_TILE_SIZE))
    {  //if it arrived at this move's destination
      mlist.erase(mlist.begin());
      moving = 0;
      if (mlist.size() > 0)
      {
        direction = mlist[0];
      }
    }
    frame += .08;  //if moving, the frames change more quickly
  }
  if (moving == 0)
  {
    if (mlist.size() > 0)
    {
      switch(mlist[0])
      {
        case UP:
          destx = tilex;
          desty = tiley - 1;
          break;
        case DOWN:
          destx = tilex;
          desty = tiley + 1;
          break;
        case LEFT:
          destx = tilex - 1;
          desty = tiley;
          break;
        case RIGHT:
          destx = tilex + 1;
          desty = tiley;
          break;
      }
      direction = mlist[0];
      moving = 1;
    }
    if (moving == 0)
    {
			rotation = angle(direction) + rand()%10 - 5;  //rotate a little, for variety
      if (otherunit != -1)
      {  //if it stops moving when on top of another unit, it is merging
        otherunit = -1;  //which means the covered unit should be erased
      }
    }
  }
}

void _unit::draw()
{
  draw(true);
}

void _unit::draw(bool increment_frame)
{
  int z = (direction - 1) * 2;
  int lx, ly;
  char tempchar[5];
  float alpha = 1.0;
  _tile *t;
  
	if (translucent_units) alpha = 0.3;
	
	if (unitstats[type].cancapture)
	{
		t = &worldmap.tile[tilex][tiley];
		if ((t->is_building()) && (!(t->is_unit_producing())) && (!(t->is_HQ())))
		{  //if it can take control of the city, change it to its "holding city" frame
			if (units[type].size > 8)  //but make sure it actually HAS a "holding city" frame first
				z = 8;
		}
	}
	if (unitstats[type].healthdrawhack)
	{  //different images depending on health
		z += 10 * intmax((rounded_health(health) / 2) - 1, 0);
	}
	if (attackdelay > 0)
	{  //if there's an attack delay, switch to "unarmed" images
		z += 8;
	}
	
  lx = x - (worldmap.scroll_x * MAP_TILE_SIZE) + worldmap.offset_x;
  ly = y - (worldmap.scroll_y * MAP_TILE_SIZE) + worldmap.offset_y;
  if (moving == 1)
  {
    if (unitstats[type].moveparticle == P_DUST)
    {
      create_particle(x + MAP_TILE_SIZE / 2, y + MAP_TILE_SIZE - 5, P_DUST);
    }
    else if (unitstats[type].moveparticle == P_AIRTRAIL)
    {
      create_particle(x + MAP_TILE_SIZE / 2, y + 11, P_AIRTRAIL);
      create_particle(x + MAP_TILE_SIZE / 2, y + 11, P_AIRTRAIL);
    }
    else if (unitstats[type].moveparticle == P_SEATRAIL)
    {
      create_particle(x + 15 + rand()%10, y + 23 + rand()%10, P_SEATRAIL);
      create_particle(x + 15 + rand()%10, y + 23 + rand()%10, P_SEATRAIL);
    }
  }
	else
	{  //if it isn't moving, the unit's direction depends on its rotation
		while (rotation > 255) rotation -= 256;  //make sure rotation is between 0 and 255
  	while (rotation < 0) rotation += 256;
		direction = angle_direction(rotation);
	}
	
  if (increment_frame == true)
  {
    frame += .04;
  }
  if (frame >= 2)
  {
    frame -= 2;
  }
  if (health < 5)
  {  //start smoking if low on health
    if ((unitstats[type].movetype != M_INFANTRY) && (unitstats[type].movetype != M_MECH))
    {  //except if it's an infantry or mech
      if (int(frame / 0.04) % 2 == 0)
      {
        create_particle(x + 18, y + 15, P_SMOKE);
        if (health < 3)
        {
          create_particle(x + 18, y + 15, P_SMOKE);
        }
      }
  	}
  }
	
  if (ready == 1)
  {
  	if (canmove)
  	{
			buffer_rotate_tinted_sprite(units[type].bmp(z + int(frame)), lx + MAP_TILE_SIZE / 2, ly + MAP_TILE_SIZE / 2,
																	rotation - angle(direction), ol::Rgba(player[color].tintcolor, 100), alpha);
		}
		else
		{
			buffer_rotate_tinted_sprite(units[type].bmp(z), lx + MAP_TILE_SIZE / 2, ly + MAP_TILE_SIZE / 2,
																	rotation - angle(direction), ol::Rgba(player[color].tintcolor, 100), alpha);
		}
  }
  else
  {
		buffer_rotate_tinted_sprite(units[type].bmp(z), lx + MAP_TILE_SIZE / 2, ly + MAP_TILE_SIZE / 2, rotation - angle(direction), ol::Rgba(BLACK, 100).MixWith(ol::Rgba(player[color].tintcolor, 100), 0.5), alpha);
  }
  
  if (unitstats[type].name == "Shield Generator")
	{
		if (shieldgen_nearby(tilex, tiley, player[color].team, true))
		{
			shielding = false;
		}
		else
		{
			shielding = true;
		}
	}
	else
	{
		if (shieldgen_nearby(tilex, tiley, player[color].team))
		{
			buffer_circlefill_gradient((tilex - worldmap.scroll_x) * MAP_TILE_SIZE + MAP_TILE_SIZE / 2 + worldmap.offset_x,
																 (tiley - worldmap.scroll_y) * MAP_TILE_SIZE + MAP_TILE_SIZE / 2 + worldmap.offset_y,
																 MAP_TILE_SIZE / 2.0,
																 ol::Rgba(0, 0, 0, 0), ol::Rgba(1.0, 1.0, 1.0, (0.5 + float(rand()%5) / 20.0) * alpha));
		}
	}
  
  if (rounded_health(health) < 10)
  {  //if less than maximum health, show it at the bottom-right
    sprintf(tempchar, "%-1d", rounded_health(health));
    buffer_textout_ex(normal, tempchar, lx + MAP_TILE_SIZE - 7, ly + MAP_TILE_SIZE - 10, WHITE);
  }
}

void _unit::set_moves(vector<int> list)
{
  mlist = list;
}

vector<int> _unit::get_movelist()
{
	return mlist;
}

bool _unit::is_moving()
{
  if (moving == 0)
  {
    return false;
  }
  return true;
}

int _unit::load_unit(_unit *u)
{
  int i = 0;
  int loadok = 0;
	char c[100];
	
  loadok = can_load_unit(u);
  if (load[i].loaded == 1)
  {
    i++;
  }
  if (loadok == 1)
  {
		
		sprintf(c, "Player %d's unit %d (%s) has loaded unit %d (%s)", color, number, unitstats[type].name.c_str(), u->number, unitstats[u->type].name.c_str());
		empdebug(c);
		
    load[i].loaded = 1;
    load[i].type = u->type;
    load[i].health = u->health;
    if (u->load[0].loaded == 1)
    {  //if this is a lander and the loaded unit is an APC with a passenger
      load[i].subload[0].loaded = 1;
      load[i].subload[0].type = u->load[0].type;
      load[i].subload[0].health = u->load[0].health;
    }
    if (u->load[1].loaded == 1)
    {
    	load[i].subload[1].loaded = 1;
    	load[i].subload[1].type = u->load[1].type;
    	load[i].subload[1].health = u->load[1].health;
    }
    return 0;
  }
  return -1;  //return -1 if unable to load unit for whatever reason
}

bool _unit::can_load_unit(_unit *u)
{
  int i = 0;
  if (u->color != color) return false;
  while (i < unitstats[type].loadcapacity)
  {
    if (load[i].loaded == 0)
    {
			if (unitstats[type].loadtype == 1)
			{  //can load infantry, mech
				if ((unitstats[u->type].movetype == M_INFANTRY) || (unitstats[u->type].movetype == M_MECH))
				{
					return true;
				}
			}
			else if (unitstats[type].loadtype == 2)
			{  //can load any land unit
				if (unitstats[u->type].basetype == LAND)
				{
					return true;
				}
			}
    }
    i++;
  }
  return false;
}

bool _unit::can_unload_unit(int n)
{
  int tx, ty, type;
  if ((load[n].loaded == 1) && (worldmap.tile[tilex][tiley].get_set() != 2))
  {  //if the transport unit is over sea, it can't unload no matter what
    type = load[n].type;
    tx = tilex + 1;
    ty = tiley;
    if (unit_unloadable_here(tilex, tiley, tx, ty, type))
    {
      return true;
    }
    tx = tilex - 1;
    if (unit_unloadable_here(tilex, tiley, tx, ty, type))
    {
      return true;
    }
    tx = tilex;
    ty = tiley + 1;
    if (unit_unloadable_here(tilex, tiley, tx, ty, type))
    {
      return true;
    }
    ty = tiley - 1;
    if (unit_unloadable_here(tilex, tiley, tx, ty, type))
    {
      return true;
    }
  }
  return false;
}

void _unit::kill()
{
  health = 0;
  exists = 0;
  init_explode(tilex, tiley);  //the explosion animation takes care of clearing the tile's unit variable
}

void _unit::set_tiles(int tx, int ty)
{
  worldmap.tile[tilex][tiley].set_unit(-1, -1);
  tilex = tx;
  tiley = ty;
  x = tilex * MAP_TILE_SIZE;
  y = tiley * MAP_TILE_SIZE;
  worldmap.tile[tilex][tiley].set_unit(color, number);
}

void _unit::reposition(int offset_x, int offset_y)
{
	x += offset_x;
	y += offset_y;
}

int _unit::units_loaded()
{
	int i = 0;
	if (load[0].loaded == 1) i++;
	if (load[1].loaded == 1) i++;
	return i;
}

int angle(int dir)
{
	switch (dir)
	{
	  case RIGHT:
  		return 0;
	    break;
	  case DOWN:
  		return 64;
	    break;
	  case LEFT:
  		return 128;
	    break;
	  case UP:
  		return 192;
	    break;
		default:
			return 0;
		  break;
  }
}

int angle_direction(int rot)
{
	rot += 32;  //an eighth of a circle
	while (rot > 256) rot -= 256;
	
	if (rot < 64) return RIGHT;
	if (rot < 128) return DOWN;
	if (rot < 192) return LEFT;
	return UP;
}

bool shieldgen_nearby(int tx, int ty, int team, bool ignore_shieldstatus)
{
	int i, j, k;
	
	j = ty - SHIELD_RADIUS;
	if (j < 0) j = 0;
	while ((j <= ty + SHIELD_RADIUS) && (j < worldmap.h))
	{
		i = tx - SHIELD_RADIUS;
		if (i < 0) i = 0;
		while ((i <= tx + SHIELD_RADIUS) && (i < worldmap.l))
		{
			if (tile_distance(i, j, tx, ty) <= SHIELD_RADIUS)
			{
				if ((i != tx) || (j != ty))
				{
					k = any_unit_here(i, j);
					if (k != -1)
					{
						if ((player[k / 100].team == team) && (unitstats[player[k / 100].unit[k % 100].type].name == "Shield Generator"))
						{
							if ((player[k / 100].unit[k % 100].shielding) || (ignore_shieldstatus))
							{
								return true;
							}
						}
					}
				}
			}
			i++;
		}
		j++;
	}
	return false;
}

int unit_price(int type, int army)
{
	int p = unitstats[type]._price;
	return p + ((p * (armystats[army].pricefraction - 100)) / 100);
}
