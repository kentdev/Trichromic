#include "empire.h"

#include "logic.h"  //needed for the special case where a unit is teleporting
extern bool ai_unit_animating;
extern bool translucent_units;

extern void save_pathmap_info();
extern int moves_needed(int mtype, int tx, int ty, int plyr);
extern int any_unit_here(int tx, int ty);
extern bitmapdat *set_datafile(int set);

int HQglow = 0;
int HQglowdir = 4;

vector<animtiles> animated_tiles;

bool _tile::is_building()
{
  if (set == 4)
  {  // set 4 is the building set
    return true;
  }
  return false;
}

bool _tile::is_building(int type)
{
  if (set == 4)
  {
    if (type == number) return true;
  }
  return false;
}

int _tile::building_type()
{
  if (set == 4)
  {
    return number;
  }
  return -1;
}

bool _tile::is_unit_producing()
{
  if (set == 4)
  {  //only buildings could possibly build units
    if (number > CITY)
    {
      return true;
    }
  }
  return false;
}

bool _tile::is_HQ()
{
  if (set == 4)
  {
    if (number == HQ)
    {
      return true;
    }
  }
  return false;
}

bool _tile::owned_by(int plyr)
{
	if (set == 4)
	{  //only buildings can be owned
		return (owner == plyr);
	}
	return false;
}

bool _tile::same_type(int t)
{
  if (set == t / 100)
  {
    if (number == t % 100)
    {
      return true;
    }
  }
  return false;
}

bool _tile::same_type(int tset, int tnum)
{
  if (set == tset)
  {
    if (number == tnum)
    {
      return true;
    }
  }
  return false;
}

void _tile::reset()
{
  unit = -1;
  set = 0;
  animated = -1;
  number = 3;  //set the tile to grass
  oldset = -1;
  oldnum = -1;
  step = 999;
  temp = 999;
  animframe = 0;
	burnframe = 0;
	burning = false;
	burnedfor = 0;
	owner = -1;
	turnsused = 0;
}

void _tile::change_type(int new_set, int new_number)
{
	set = new_set;
	number = new_number;
}

void _tile::change_owner(int plyr)
{
	if (set == 4)
	{
		if ((plyr >= -1) && (plyr < 4)) owner = plyr;  //plyr can only be 0-3 inclusive or -1
		else owner = -1;
	}
}

void _tile::change_to_city()
{
	set = 4;
	number = CITY;
}

void _tile::draw(int x, int y)
{
  int i, s;
  ol::Bitmap *bmp = set_datafile(set)->bmp(number);
  
  if ((set != oldset) || (number != oldnum))
  {  //if this tile has changed type
    animframe = 0;
    oldset = set;
    oldnum = number;
    i = 0;
    s = animated_tiles.size();
    animated = -1;
    while (i < s)
    {
      if ((animated_tiles[i].set == set) && (animated_tiles[i].num == number))
      {
        animated = i;
        i = s;
      }
      i++;
    }
  }
  
  if (animated == -1)
  {
		if (set == 4)  //buildings set
		{
			buffer_draw_sprite(land.bmp(3), x, y - (land.bmp(2)->Height() - MAP_TILE_SIZE));
			if (owner == -1)
			{
				buffer_draw_sprite(bmp, x, y - (bmp->Height() - MAP_TILE_SIZE));
			}
			else
			{
				if (is_HQ()) buffer_draw_tinted_sprite(bmp, x, y, ol::Rgba(player[owner].tintcolor, 150).MixWith(ol::Rgba(WHITE, 255), float(HQglow) / 255.0));
				else buffer_draw_tinted_sprite(bmp, x, y, ol::Rgba(player[owner].tintcolor, 150));
			}
		}
    else
    {
			buffer_draw_sprite(bmp, x, y - (bmp->Height() - MAP_TILE_SIZE));
    }
  }
  else
  {
    animframe += animated_tiles[animated].frameinc;
    if (int(animframe) >= animated_tiles[animated].framenum)
    {
      animframe = 0;
    }
    buffer_blit_clipped(bmp, x, y - (MAP_TILE_SIZE - bmp->Height()), int(animframe) * MAP_TILE_SIZE, 0, MAP_TILE_SIZE, bmp->Height());
  }
	
	if (burning)
	{
		burnframe += 0.2;
		if (burnframe >= 3) burnframe = 0;
		buffer_draw_sprite(glb.bmp(29 + int(burnframe)), x, y, 0.7);
		if (rand()%2 == 0)
		{
			create_particle(x + rand()%12 - 6 + int(MAP_TILE_SIZE * (worldmap.scroll_x + 0.5)) - worldmap.offset_x,
											y + rand()%12 - 6 + int(MAP_TILE_SIZE * (worldmap.scroll_y + 0.5)) - worldmap.offset_y, P_FIRE);
		}
	}
}

void _tile::draw_unit(int x, int y, _unit *other_unit_here)
{
	float alpha = 1.0;
	bool ellipse = false;
	int i, s, tx = -1, ty = -1, t = -1, mx = mouse_x, my = mouse_y;

  if (mx >= worldmap.offset_x)
  {
    tx = worldmap.scroll_x + (mx - worldmap.offset_x) / MAP_TILE_SIZE;
  }
  else
  {
    tx = worldmap.scroll_x - 1 + (mx - worldmap.offset_x) / MAP_TILE_SIZE;
  }
  if (my >= worldmap.offset_y)
  {
    ty = worldmap.scroll_y + (my - worldmap.offset_y) / MAP_TILE_SIZE;
  }
  else
  {
    ty = worldmap.scroll_y - 1 + (my - worldmap.offset_y) / MAP_TILE_SIZE;
  }
	if ((tx == this->x) && (ty == this->y))
	{  //if the mouse is on this tile
  	t = unit;
  	if (t != -1)
  	{
  		if (t / 100 == pturn)
  		{
  			if ((player[t / 100].unit[t % 100].ready == 1) && (player[t / 100].unit[t % 100].canmove))
  			{
  				ellipse = true;
  			}
  		}
  	}
	}
	
	if (translucent_units) alpha = 0.3;
	
	if (other_unit_here != NULL)
  {
		if (ellipse) glb.draw(43, x, y, alpha);
  	else if ((other_unit_here->color == pturn) && (player[i].controller == HUMAN) && (other_unit_here->canmove) && (other_unit_here->ready == 1)) glb.draw(42, x, y, alpha);
    other_unit_here->draw();
  }
  
  if (unit != -1)
  {
    i = unit / 100;
    s = unit % 100;
    if (player[i].unit[s].exists == 1)
    {
      if (!player[i].unit[s].is_moving())
      {
				if (ellipse) glb.draw(43, x, y, alpha);
				else if ((i == pturn) && (player[i].controller == HUMAN) && (player[i].unit[s].canmove) && (player[i].unit[s].ready == 1)) glb.draw(42, x, y, alpha);
        player[i].unit[s].draw();
      }
    }
	  else  //if the unit doesn't exist but still is in the tile's unit thing
	  {  //it must be about to explode (as in, the map is scrolling to the unit so it can blow up)
		  if ((var.logicstate != TELEPORT_ANIMATION) && (!ai_unit_animating))
			{  //UNLESS a unit is currently teleporting, in which case the unit shouldn't appear if it doesn't exist
			  player[i].unit[s].draw();
			}
  	}
  }
}

void _tile::set_unit(int plyr, int number)
{
  if ((plyr == -1) || (number == -1))
  {
    unit = -1;
  }
  else
  {
    unit = (plyr * 100) + number;
  }
}

void _tile::set_temp(short t)
{
  temp = t;
}

void _tile::set_step(short s)
{
  step = s;
}

short _tile::get_temp()
{
  return temp;
}

short _tile::get_step()
{
  return step;
}

int _tile::unit_here()
{
  return unit;
}

int _tile::get_owner()
{
	return owner;
}

int _tile::get_set()
{
  return set;
}

int _tile::get_number()
{
  return number;
}




//==========================================================
//==========================================================
//==========================================================




void _map::reset()
{
  int i, j;
  j = 0;
  while (j < MAP_MAX_H)
  {
    i = 0;
    while (i < MAP_MAX_L)
    {
      tile[i][j].reset();
      tile[i][j].x = i;
      tile[i][j].y = j;
      i++;
    }
    j++;
  }
  name = "";
  pathmap_limit = 12;  //the AI needs the pathmap to cover everything
  l = XTILES;          //but humans only need it for unit movement
  h = YTILES;
  offset_x = 0;
  offset_y = 0;
  scroll_x = 0;
  scroll_y = 0;
}

string _map::get_name()
{
  return name;
}

void _map::set_name(string new_name)
{
  name = new_name;
}

void _map::draw_tiles(bool showunits)
{  //starts at -1 to account for scroll offset

	//UGLY HACK TO PREVENT BLACK HORIZONTAL LINES ON THE TRANSLUCENT BOUNDARY OF THE MOUNTAIN TILES
	buffer_rectfill(0, 0, XTILES * MAP_TILE_SIZE, YTILES * MAP_TILE_SIZE, makecol(0, 50, 0));
	//END OF UGLY HACK
	
  draw_tiles(-1, -1, XTILES  + 1, YTILES + 1, NULL, showunits);
}

void _map::draw_tiles(int gridx, int gridy, int numx, int numy, bool showunits)
{
  draw_tiles(gridx, gridy, numx, numy, NULL, showunits);
}

void _map::draw_tiles(int gridx, int gridy, int numx, int numy, _unit *other_unit_here, bool showunits)
{
  int i;
  int j = gridy;
  int tilex, tiley;
  
	HQglow += HQglowdir;
	if (HQglow > 150)
	{
		HQglow = 150;
		HQglowdir = -HQglowdir;
	}
	else if (HQglow < 0)
	{
		HQglow = 0;
		HQglowdir = -HQglowdir;
	}
  
  while (j < gridy + numy)
  {
    i = gridx;
    while (i < gridx + numx)
    {
			tilex = i + scroll_x;
      tiley = j + scroll_y;
      if ((tilex > -1) && (tilex < MAP_MAX_L) && (tiley > -1) && (tiley < MAP_MAX_H))
			{
      	if ((tilex > -1) && (tilex < l) && (tiley > -1) && (tiley < h))
      	{
	        tile[tilex][tiley].draw(i * MAP_TILE_SIZE + offset_x, j * MAP_TILE_SIZE + offset_y);
      	}
			}
      i++;
    }
    j++;
  }
	
	if (showunits)
	{
	  j = gridy;
	  while (j < gridy + numy)
    {
      i = gridx;
      while (i < gridx + numx)
      {
        tilex = i + scroll_x;
        tiley = j + scroll_y;
				if ((tilex > -1) && (tilex < MAP_MAX_L) && (tiley > -1) && (tiley < MAP_MAX_H))
				{
        	if ((tilex > -1) && (tilex < l) && (tiley > -1) && (tiley < h))
        	{
	          tile[tilex][tiley].draw_unit(i * MAP_TILE_SIZE + offset_x, j * MAP_TILE_SIZE + offset_y, other_unit_here);
        	}
				}
        i++;
      }
      j++;
		}
  }
}

void _map::create_pathmap(int plyr, int mtype, int startx, int starty)
{
  create_pathmap(plyr, mtype, startx, starty, false);
}

void _map::create_limited_pathmap(int plyr, int mtype, int startx, int starty, int limit)
{
  pathmap_limit = limit;
  create_pathmap(plyr, mtype, startx, starty, true);
}

void _map::create_pathmap(int plyr, int mtype, int startx, int starty, bool limited)
{
  int x, y, z;
  y = 0;
  while (y < h)
  {
    x = 0;
    while (x < l)
    {
      tile[x][y].set_step(999);
      x++;
    }
    y++;
  }
	
  z = 0;
  while (z < 4)
  {
    switch(z)
    {
      case 0:
        x = startx + 1;
        y = starty;
        break;
      case 1:
        x = startx - 1;
        y = starty;
        break;
      case 2:
        x = startx;
        y = starty - 1;
        break;
      case 3:
        x = startx;
        y = starty + 1;
        break;
    }
    if ((x >= 0) && (x < l) && (y >= 0) && (y < h))
    {
      pathmap_recursive(plyr, mtype, x, y, 0, limited);
    }
    z++;
  }
  tile[startx][starty].set_step(0);
}

void _map::pathmap_recursive(int plyr, int mtype, int ox, int oy, int moves, bool limited)
{
  int m = moves_needed(mtype, ox, oy, plyr);
  int x, y, z;
	
  z = any_unit_here(ox, oy);
  if ((z != -1) && (z / 100 != plyr) && (player[z / 100].team != player[plyr].team))
  {
    m = 99;
  }
  moves += m;
  if ((moves >= tile[ox][oy].get_step()) || (m == 99))
  {
    return;
  }
  if (limited == true)
  {
    if (moves > pathmap_limit)
    {
      return;
    }
  }

  tile[ox][oy].set_step(moves);
  z = 0;
  while (z < 4)
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
    if ((x >= 0) && (x < l) && (y >= 0) && (y < h))
    {
      pathmap_recursive(plyr, mtype, x, y, moves, limited);
    }
    z++;
  }
}

void _map::backup_pathmap()
{
  int i, j;
  j = 0;
  while (j < h)
  {
    i = 0;
    while (i < l)
    {
      tile[i][j].set_temp(tile[i][j].get_step());
      i++;
    }
    j++;
  }
}

void _map::restore_pathmap()
{
  int i, j;
  j = 0;
  while (j < h)
  {
    i = 0;
    while (i < l)
    {
      tile[i][j].set_step(tile[i][j].get_temp());
      i++;
    }
    j++;
  }
}

bool _map::in_bounds(int x, int y)
{
	if ((x < 0) || (y < 0) || (x >= l) || (y >= h))
	{
		return false;
	}
	return true;
}

void load_map(string mapname, const char *path, bool reset_players)
{
	int a, b, c, d, i, j, temp, temp2;
	char dbg[50];
	ifstream m(path);
	
	worldmap.reset();
	if (reset_players)
	{
		player[0].reset();  //get rid of any previous settings or units
		player[1].reset();
		player[2].reset();
		player[3].reset();
	}
	else
	{
		player[0].clear_units();  //just get rid of old units
		player[0].stats.reset();
		player[1].clear_units();
		player[1].stats.reset();
		player[2].clear_units();
		player[2].stats.reset();
		player[3].clear_units();
		player[3].stats.reset();
	}
	worldmap.set_name(mapname);

	if (m.bad())
	{
		sprintf(dbg, "Couldn't load map file: %s", path);
		empdebug(dbg);
		return;
	}

	m >> worldmap.l >> worldmap.h >> numplayers;
	
	i = 0;
	while (i < numplayers)
	{  //initialize all players participating in this game
		player[i].playing = 1;
		player[i].everplaying = 1;
		i++;
	}
	while (i < 4)
	{  //make sure other players are set to "not playing"
		player[i].playing = 0;
		player[i].everplaying = 0;
		i++;
	}
	
	i = 0;
	while (i < worldmap.h)
	{
		j = 0;
		while (j < worldmap.l)
		{
			m >> temp >> temp2;
			worldmap.tile[j][i].change_type(temp / 100, temp % 100);
			worldmap.tile[j][i].change_owner(temp2);
			j++;
		}
		i++;
	}
	i = 0;
	while (i < 50 * numplayers)  //loading any placed units
	{
		m >> a;
		if (a != -1)  //if there is a saved unit for this i
		{
			m >> b >> c >> d;
			j = player[d].create_unit(c, a, b, d);
		}
		else  //if a reads -1, there are no more saved units
		{
			i = 50 * numplayers;
		}
		i++;
	}
	m.close();
}
