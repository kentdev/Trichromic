#include "empire.h"

#define MFIRST  6
#define MLOAD   7
#define MNEW    8
#define MEDIT   9
#define MEXIT  10
#define MCHECK 11
#define MSAVE  12

extern setunit mapunit[200];

//extern ol::Bitmap *mapunits[72];

extern volatile int counter;

extern void text_box(int x, int y, int w, int h, char *ret, int leng, int spaces);
extern void text_box(int x, int y, int w, int h, char *ret, int leng, int spaces, string title);
extern void draw_units(int scrollx, int scrolly);
extern void new_unit(int x, int y, int type, int player);
extern void new_unit(int x, int y, int type, int player, int sound);
extern void clear_all_units();
extern void remove_unit(int n);
extern void refresh_maplist();
extern bool munit_exists(int playernum);
extern int any_unit_here(int tx, int ty);
extern int mapcolor(int set, int num);
extern int check_for_unit(int x, int y);
extern int find_free_unit();
extern int units_for_player(int plyr);
extern string unit_name(int i);

void draw_selection(int set, int x);
void delete_map(string mapname);
void draw_minimap(int x, int y, string file);
void remove_previous_HQ(int player, int ignorex, int ignorey);
void map_message(string message);
void draw_trans_tile(int set, int num, int x, int y);
void move_tiles(int x1, int y1, int x2, int y2, int dx, int dy);
bool HQ_exists(int playernum);
bool base_exists(int playernum);
int mfirst();
int mload();
int mnew();
int medit();
int mcheck();
int save_map();
int do_aiselect();
int datafile_size(DATAFILE *data);
int resize_map();
bitmapdat *set_datafile(int set);

float length;
float height;

_button option[34];
_button selection[18];

int mapedit()
{
  static bool first = true;
  static int status = MFIRST;
  if (first)
  {
    worldmap.reset();
		clear_all_units();
		minimap.create();
    first = false;
  }
  switch(status)
  {
    case MFIRST:
      status = mfirst();
      break;
    case MLOAD:
      status = mload();
      break;
    case MNEW:
      status = mnew();
      break;
    case MEDIT:
      status = medit();
      break;
    case MCHECK:
      status = mcheck();
      break;
    case MSAVE:
      status = save_map();
      break;
    case MEXIT:
      status = MFIRST;
      return MENU;
      break;
  }
  return MAPEDIT;
}





int mfirst()
{
  static int drawfirst = 1;
  if (drawfirst == 1)
  {
    option[0].init(230, 575, 100, 20, "New Map");
    option[1].init(340, 575, 100, 20, "Load Map");
    option[2].init(10, 575, 100, 20, "Back");
    option[3].init(450, 575, 100, 20, "Continue Map");
    drawfirst = 0;
    clear_particles();
  }  
  glb.draw(13, 0, 0);
  option[0].check();
  option[1].check();
  option[2].check();
  option[3].check();
  draw_mouse();
  if (option[0].clicked())
  {
    clear_all_units();
    drawfirst = 1;
    return MNEW;
  }
  if (option[1].clicked())
  {
    clear_all_units();
    drawfirst = 1;
    return MLOAD;
  }
  if ((option[2].clicked()) || (key[KEY_ESC]))
  {
    worldmap.reset();
    clear_all_units();
    drawfirst = 1;
    return MEXIT;
  }
  if (option[3].clicked())
  {
    drawfirst = 1;
    return MEDIT;
  }
  return MFIRST;
}





int mload()
{
  static bool first = true;
  static int highlighted = -1;
  static int delay = 0;
  static int lscroll = 0;
  static int mapnum;
  string ainame;
  int i, j;
  int ok;
  int mx;
  int my;
  int sel;
  int pnum;
  int a, b, c, d;
  char p[40] = "";
  string s;
  
  if (first)
  {
    option[0].init(10, 575, 100, 20, "Back");
    refresh_maplist();
    i = maplist.size();
    if (i > 22)  //if the list will go beyond the load box
    {
      option[1].init(195, 355, 85, 20, "Up");
      option[2].init(285, 355, 85, 20, "Down");
    }
    mapnum = i;
    i = lscroll;
    while ((i < lscroll + 22) && (i < mapnum))
    {
      buffer_textout_ex(normal, maplist[i].c_str(), 200, 20 + (15 * (i - lscroll)), WHITE);
      i++;
    }
    first = false;
  }

  glb.draw(13, 0, 0);
  buffer_rectfill(195, 455, 370, 478, BLACK);
  buffer_rect(195, 455, 370, 478, LGREY);
  buffer_textout_centre_ex(normal, "Left mouse: Select", 280, 456, WHITE);
  buffer_textout_centre_ex(normal, "Right mouse: Delete", 280, 466, RED);
  buffer_rect(195, 15, 370, 350, LGREY);
  buffer_rectfill(196, 16, 369, 349, BLACK);
  
  i = lscroll;
  while ((i < lscroll + 22) && (i < mapnum))
  {
    buffer_textout_ex(normal, maplist[i].c_str(), 200, 20 + (15 * (i - lscroll)), WHITE);
    i++;
  }
  
  if (mapnum == 0)
  {
    buffer_textout_ex(normal, "No maps found", 200, 20, RED);
  }
  option[0].check();
  if (mapnum > 22)
  {
    option[1].check();
    option[2].check();
    if (option[1].clicked())
    {
      if (lscroll > 0)
      {
        lscroll--;
      }
    }
    if (option[2].clicked())
    {
      if (lscroll + 22 < mapnum)
      {
        lscroll++;
      }
    }
  }
  if (option[0].clicked())
  {
    first = true;
    highlighted = -1;
    lscroll = 0;
    return MFIRST;
  }
  mx = mouse_x;
  my = mouse_y;
  if ((mx > 200) && (mx < 370))
  {
    if ((my > 15) && (my < 350))
    {
      sel = (my - 20) / 15 + lscroll;
      if (sel < mapnum)
      {
        if (maplist[sel].c_str() != "")
        {
          buffer_rectfill(395, 80, 605, 305, BLACK);
          buffer_rect(395, 80, 605, 305, LGREY);
          sprintf(p, "maps/");
					s = maplist[sel];
					s.erase(0, 5);  //get rid of the number of players
          strcat(p, s.c_str());
          strcat(p, ".map");
          buffer_textout_ex(normal, maplist[sel].c_str(), 400, 85, WHITE);
          draw_minimap(400, 100, p);
        }
        if (mouse_b & 1)
        {
          sprintf(p, "maps/");
					s = maplist[sel];
					s.erase(0, 5);
          strcat(p, s.c_str());
          strcat(p, ".map");
          ok = 1;
          ifstream m(p);
          if (m.bad())
          {
            ok = 0;
          }
          if (ok == 1)
          {
            worldmap.set_name(s);
            m >> worldmap.l >> worldmap.h >> pnum;
            i = 0;
            while (i < worldmap.h)  //loading the map tiles
            {
              j = 0;
              while (j < worldmap.l)
              {
                m >> a;
                m >> b;
                worldmap.tile[j][i].change_type(a / 100, a % 100);
                worldmap.tile[j][i].change_owner(b);
                j++;
              }
              i++;
            }
            clear_all_units();
            i = 0;
            while (i < 200)  //loading any placed units
            {
              m >> a;
              if (a != -1)  //if there is a saved unit for this i
              {
                m >> b >> c >> d;
                new_unit(a, b, c, d, 0);  //no unit placement sound
              }
              else  //if a reads -1, there are no more saved units
              {
                i = 200;
              }
              i++;
            }
            s = maplist[sel];
						s.erase(0, 5); //get rid of the (2P) / (3P) / (4P)
            load_ai(s);
            lscroll = 0;
            first = true;
            highlighted = -1;
            minimap.create();
            return MEDIT;
          }
        }
        if (mouse_b & 2)
        {
          if (delay == 0)
          {
						s = maplist[sel];
						s.erase(0, 5);
            delete_map(s);
            delay = 50;
            lscroll = 0;
            first = true;
          }
        }
      }
    }
  }
  if (delay > 0)
  {
    delay--;
  }
  draw_mouse();
  return MLOAD;
}





int mnew()
{
  static int drawfirst = 1;
  int i;
	char msg[30];
  if (drawfirst == 1)
  {
    length = XTILES;
    height = YTILES;
    option[0].init(270, 160, 40, 15, "+");
    option[1].init(270, 190, 40, 15, "-");
    option[2].init(330, 160, 40, 15, "+");
    option[3].init(330, 190, 40, 15, "-");
    option[4].init(270, 215, 100, 20, "Create New");
    option[5].init(270, 250, 100, 20, "Cancel");
    drawfirst = 0;
  }
  glb.draw(13, 0, 0);
  buffer_rectfill(260, 130, 380, 280, GREY);
  buffer_rect(260, 130, 380, 280, WHITE);
  buffer_textout_centre_ex(normal, "Size:", 320, 140, BLACK);
  sprintf(msg, "%-1d", int(length));
  buffer_textout_centre_ex(normal, msg, 290, 178, BLACK);
  sprintf(msg, "%-1d", int(height));
  buffer_textout_centre_ex(normal, msg, 350, 178, BLACK);
  i = 0;
  while (i < 6)
  {
    option[i].check();
    i++;
  }
  if (option[0].mousedown() == 1)
  {
    if (length < MAP_MAX_L)
    {
      length += .25;
    }
  }
  if (option[1].mousedown() == 1)
  {
    if (length > XTILES)
    {
      length -=.25;
    }
    else
    {
      length = XTILES;
    }
  }
  if (option[2].mousedown() == 1)
  {
    if (height < MAP_MAX_H)
    {
      height += .25;
    }
  }
  if (option[3].mousedown() == 1)
  {
    if (height > YTILES)
    {
      height -=.25;
    }
    else
    {
      height = YTILES;
    }
  }
  if (option[4].clicked())
  {
    worldmap.reset();
    worldmap.l = int(length);
    worldmap.h = int(height);
    drawfirst = 1;
    clear_all_units();
    minimap.create();
    return MEDIT;
  }
  if (option[5].clicked())
  {
    drawfirst = 1;
    load_default_ai();
    return MFIRST;
  }
  draw_mouse();
  return MNEW;
}





int medit()
{
	const int leftx = 800 - MAP_TILE_SIZE * 5;
	const int minimapx = leftx + 5;
	const int minimapy = 447;
	const int messagey = 562;
  string backupname;
  char tempname[20] = "";
	char msg[30];
  static int drawfirst = 1;
  static int moving_tiles;
  static float scrx = 0;
  static float scry = 0;
  //static float tscroll = 0;
  static int selected = -1;
  static int set = 0;
  static int gone_to_mapcheck = 0;
  static int ai_select = 0;
  static int resize_select = 0;
  static int owner = -1;
  static int tx1, ty1, tx2, ty2, t;
  //int tscroll_i = int(tscroll);
  int i, j, z, temp;//, scrolled, mickeyx, mickeyy;
  int mx = mouse_x;
  int my = mouse_y;
	int mm_mousex, mm_mousey;
  if (drawfirst == 1)
  {
    option[0].init(leftx + 10, 256, 60, 20, "<");
    option[1].init(leftx + 90, 256, 60, 20, ">");
    option[20].init(leftx + 5, 300, 150, 15, "Land");
    option[21].init(leftx + 5, 320, 150, 15, "River");
    option[22].init(leftx + 5, 340, 150, 15, "Sea");
    option[29].init(leftx + 5, 360, 150, 15, "Shoal");
    option[26].init(leftx + 5, 380, 150, 15, "Buildings");
    option[33].init(leftx + 5, 400, 150, 15, "Special");
    option[27].init(leftx + 5, 420, 150, 15, "Units");
    option[25].init(5, 582, 70, 15, "Back");
		option[28].init(110, 582, 70, 15, "Check");
		option[30].init(185, 582, 70, 15, "AI");
		option[31].init(290, 582, 70, 15, "Resize");
		option[32].init(365, 582, 70, 15, "Copy Tiles");
		option[24].init(470, 582, 80, 15, "Name");
		option[23].init(555, 582, 80, 15, "Save");
		
		scrx = float(worldmap.scroll_x);
		scry = float(worldmap.scroll_y);
		
    if (gone_to_mapcheck == 0)
    {
      i = 0;
      draw_selection(0, i);
      draw_units(0, 0);
      //scrx = 0;
      //scry = 0;
      set = 0;
      //tscroll = 0;
			t = 0;
      selected = -1;
    }
    else
    {
      //worldmap.scroll_x = int(scrx);
      //worldmap.scroll_y = int(scry);
      draw_selection(set, t);//tscroll_i);
      draw_units(worldmap.scroll_x, worldmap.scroll_y);
      gone_to_mapcheck = 0;
    }
    moving_tiles = -1;
    worldmap.scroll_x = 0;
    worldmap.scroll_y = 0;
    position_mouse(320, 240);
    drawfirst = 0;
    ai_select = 0;
  }

  worldmap.draw_tiles(0, 0, XTILES, YTILES, false);
  draw_units(worldmap.scroll_x, worldmap.scroll_y);
	buffer_rectfill_gradient(0, 576, leftx, 581, WHITE, WHITE, VDGREY, VDGREY);
	buffer_rectfill_gradient(leftx, 0, leftx + 3, 600, WHITE, DGREY, BLACK, WHITE);
	buffer_rectfill_gradient(leftx + 3, 0, 800, 600, GREY, GREY, DGREY, DGREY);
	
	i = 0;
	while (i < XTILES)
	{
		buffer_vline(i * MAP_TILE_SIZE, 0, YTILES * MAP_TILE_SIZE, WHITE, 0.5);
		i++;
	}
	i = 0;
	while (i < YTILES)
	{
		buffer_hline(0, i * MAP_TILE_SIZE, leftx, WHITE, 0.5);
		i++;
	}

  if (ai_select == 1)
  {
    ai_select = do_aiselect();
    /*if (ai_select == 0)  //do_aiselect() just ended
    {
      worldmap.scroll_x = int(scrx);
      worldmap.scroll_y = int(scry);
    }*/
  }
  else if (resize_select == 1)
  {
    resize_select = resize_map();
    if (resize_select == 0)
    {
      worldmap.scroll_x = 0;
      worldmap.scroll_y = 0;
      //scrx = 0;
      //scry = 0;
    }
  }
  else
  {
    if (moving_tiles == -1)
    {
      buffer_textout_ex(normal, "Left mouse: Lay Tile [+Ctrl: Remove Unit]", 5, messagey, WHITE);
      option[20].check();
      option[21].check();
      option[22].check();
      option[23].check();
      option[24].check();
      option[25].check();
      option[26].check();
      option[27].check();
      option[28].check();
      option[29].check();
      option[30].check();
      option[31].check();
      option[32].check();
      option[33].check();
      if (option[28].clicked())
      {
        gone_to_mapcheck = 1;
        drawfirst = 1;
        return MCHECK;
      }
      if (option[25].clicked())
      {
        drawfirst = 1;
        return MFIRST;
      }
      if (option[23].clicked())
      {
        /*i = 0;
        while (i < XTILES - 2)
        {  //copied & pasted from below, drawn here so the selection area 
          selection[i].check();   //doesn't flicker when the map is saved
          i++;
        }*/
        return MSAVE;
      }
      if (option[24].clicked())
      {  //"name" was clicked, go to map name input
        sprintf(tempname, worldmap.get_name().c_str());
        backupname = worldmap.get_name();
        text_box(240, 200, 170, 20, tempname, 18, 1);
        if (strcmp(tempname, "_CANCEL_") == 0)
        {  //if ESC was pressed in text_box
          worldmap.set_name(backupname);
        }
        else
        {
          worldmap.set_name(tempname);
        }
      }
      if (option[30].clicked())
      {  //"AI" was clicked, select AI profile
        ai_select = 1;
      }
      if (option[31].clicked())
      {
        resize_select = 1;
      }
      if (option[32].clicked())
      {
        moving_tiles = 0;
      }
      if (option[20].clicked())
      {  //switch to the land tiles
        set = 0;
				t = 0;
        selected = -1;
        draw_selection(set, 0);//tscroll_i);
      }
      if (option[21].clicked())
      {  //switch to the river tiles
        set = 1;
				t = 0;
        selected = -1;
        draw_selection(set, 0);//tscroll_i);
      }
      if (option[22].clicked())
      {  //switch to the sea tiles
        set = 2;
        t = 0;
				selected = -1;
        draw_selection(set, 0);//tscroll_i);
      }
      if (option[29].clicked())
      {  //switch to the shoal tiles
        set = 3;
				t = 0;
        selected = -1;
        draw_selection(set, 0);//tscroll_i);
      }
      if (option[26].clicked())
      {  //switch to the building tiles
        set = 4;
				t = 0;
        selected = -1;
        draw_selection(set, 0);//tscroll_i);
      }
      if (option[33].clicked())
      {  //switch to the special tiles
        set = 6;
				t = 0;
        selected = -1;
        draw_selection(set, 0);//tscroll_i);
      }
      if (option[27].clicked())
      {  //switch to unit placement
        set = 5;
				t = 0;
        selected = -1;
        draw_selection(set, 0);//tscroll_i);
      }
    }
    else
    {
      if (mouse_b & 2)
      {
        moving_tiles = -1;
      }
      
      switch(moving_tiles)
      {
        case 0:
          buffer_textout_centre_ex(normal, "Select top-left corner to be copied    Right Mouse: Cancel", 320, messagey, WHITE);
          buffer_vline(mx - 5, my - 5, my + 15, BLACK);
          buffer_hline(mx - 5, my - 5, mx + 15, BLACK);
          break;
        case 1:
          buffer_textout_centre_ex(normal, "Select bottom-right corner    Right Mouse: Cancel", 320, messagey, WHITE);
          buffer_vline((tx1 - worldmap.scroll_x) * MAP_TILE_SIZE, (ty1 - worldmap.scroll_y) * MAP_TILE_SIZE, (ty1 - worldmap.scroll_y) * MAP_TILE_SIZE + 40, RED);
          buffer_hline((tx1 - worldmap.scroll_x) * MAP_TILE_SIZE, (ty1 - worldmap.scroll_y) * MAP_TILE_SIZE, (tx1 - worldmap.scroll_x) * MAP_TILE_SIZE + 40, RED);
          buffer_vline(mx + 20, my + 20, my, BLACK);
          buffer_hline(mx + 20, my + 20, mx, BLACK);
          break;
        case 2:
        case 3:
          buffer_textout_centre_ex(normal, "Select top-left corner of new location    Right Mouse: Cancel", 320, messagey, WHITE);
          buffer_vline((tx1 - worldmap.scroll_x) * MAP_TILE_SIZE, (ty1 - worldmap.scroll_y) * MAP_TILE_SIZE, (ty1 - worldmap.scroll_y) * MAP_TILE_SIZE + 40, RED);
          buffer_hline((tx1 - worldmap.scroll_x) * MAP_TILE_SIZE, (ty1 - worldmap.scroll_y) * MAP_TILE_SIZE, (tx1 - worldmap.scroll_x) * MAP_TILE_SIZE + 40, RED);
          buffer_vline((tx2 - worldmap.scroll_x + 1) * MAP_TILE_SIZE - 1, (ty2 - worldmap.scroll_y + 1) * MAP_TILE_SIZE - 1, (ty2 - worldmap.scroll_y) * MAP_TILE_SIZE - 10, RED);
          buffer_hline((tx2 - worldmap.scroll_x + 1) * MAP_TILE_SIZE - 1, (ty2 - worldmap.scroll_y + 1) * MAP_TILE_SIZE - 1, (tx2 - worldmap.scroll_x) * MAP_TILE_SIZE - 10, RED);
          break;
      }
    }
		
    if (!(mouse_b & 1))
    {  //the screen can't scroll while the mouse button is down
      if (key[KEY_LEFT])
      {
        if (scrx > 0)
        {
          scrx -= .25;
        }
      }
      if (key[KEY_RIGHT])
      {
        if (scrx + XTILES < worldmap.l)
        {
          scrx += .25;
        }
      }
      if (key[KEY_DOWN])
      {
        if (scry + YTILES < worldmap.h)
        {
          scry += .25;
        }
      }
      if (key[KEY_UP])
      {
        if (scry > 0)
        {
          scry -= .25;
        }
      }
    }
    if ((worldmap.scroll_x != int(scrx)) || (worldmap.scroll_y != int(scry)))
    {
      worldmap.scroll_x = int(scrx);
      worldmap.scroll_y = int(scry);
    }
    
    if (moving_tiles == -1)
    {  //don't show tile choices when moving tiles
      option[0].check();
      option[1].check();
      if (option[0].clicked())
      {
				if (t > 0)
				{
        	t--;
					draw_selection(set, t * 18);
        }
      }
      if (option[1].clicked())
      {
        if (set != 5)  //if it's not the mapunits set (which isn't a bitmapdat)
        {
        	if (set == 4)
        	{
        		temp = set_datafile(set)->size * 4;
        	}
        	else temp = set_datafile(set)->size;
        }
        else
        {
          temp = NUM_UNITS * 4;
        }
				if ((t + 1) * 18 < temp)
				{
					t++;
					draw_selection(set, t * 18);
				}
      }
      i = 0;
      while (i < 18)
      {
        selection[i].check();
        if ((set == 4) && (i + t * 18 < buildings.size * 5 - 1))  //buildings
        {
					buffer_draw_sprite(land.bmp(3), selection[i].x, selection[i].y);
					if (i + t * 18 < 4)
					{
						temp = i;
					}
					else
					{
						temp = ((i + t * 18) - 4) % 5 - 1;
					}
					z = i + t * 18;
					if (z < 4)
					{
						z = 0;
					}
					else
					{
						z -= 4;
						z /= 5;
						z++;
					}
					
					switch(temp)
					{
						case -1:
							buffer_draw_sprite(buildings.bmp(z), selection[i].x, selection[i].y);
							break;
						case 0:
							buffer_draw_tinted_sprite(buildings.bmp(z), selection[i].x, selection[i].y, ol::Rgba(RED, 150));
							break;
						case 1:
							buffer_draw_tinted_sprite(buildings.bmp(z), selection[i].x, selection[i].y, ol::Rgba(BLUE, 150));
							break;
						case 2:
							buffer_draw_tinted_sprite(buildings.bmp(z), selection[i].x, selection[i].y, ol::Rgba(MGREEN, 150));
							break;
						case 3:
							buffer_draw_tinted_sprite(buildings.bmp(z), selection[i].x, selection[i].y, ol::Rgba(DYELLOW, 150));
							break;
					}
        }
				else if ((set == 5) && (i + t * 18 < NUM_UNITS * 4))
				{
					switch((i + t * 18) % 4)
					{
						case 0:
							buffer_draw_tinted_sprite(units[(i + t * 18) / 4].bmp(0), selection[i].x, selection[i].y, ol::Rgba(RED, 150));
							break;
						case 1:
							buffer_draw_tinted_sprite(units[(i + t * 18) / 4].bmp(0), selection[i].x, selection[i].y, ol::Rgba(BLUE, 150));
							break;
						case 2:
							buffer_draw_tinted_sprite(units[(i + t * 18) / 4].bmp(0), selection[i].x, selection[i].y, ol::Rgba(MGREEN, 150));
							break;
						case 3:
							buffer_draw_tinted_sprite(units[(i + t * 18) / 4].bmp(0), selection[i].x, selection[i].y, ol::Rgba(DYELLOW, 150));
							break;
					}
				}
				if (selection[i].mousedown())  //not clicked() anymore, was a bit annoying
				{
					if (set == 4)
					{
						if (i + t * 18 < 4)
						{
							selected = 0;
							owner = i;
						}
						else
						{
							selected = ((i + t * 18) - 4) / 5 + 1;
							owner = ((i + t * 18) - 4) % 5 - 1;
						}
					}
					else
					{
						selected = i + t * 18;
					}
				}
				i++;
			}

			if ((selected != -1) && (my < 576) && (mx < leftx))
			{
				if (set != 5)  //if it's a tile that's selected, not a unit
				{  //give a translucent preview of the tile under the mouse
					if (set == 4)
					{
						switch(owner)
						{
							case -1:
								buffer_draw_sprite(buildings.bmp(selected), mx - MAP_TILE_SIZE / 2, my - MAP_TILE_SIZE / 2, .75);
								break;
							case 0:
								buffer_draw_tinted_sprite(buildings.bmp(selected), mx - MAP_TILE_SIZE / 2, my - MAP_TILE_SIZE / 2, ol::Rgba(RED, 150), .75);
								break;
							case 1:
								buffer_draw_tinted_sprite(buildings.bmp(selected), mx - MAP_TILE_SIZE / 2, my - MAP_TILE_SIZE / 2, ol::Rgba(BLUE, 150), .75);
								break;
							case 2:
								buffer_draw_tinted_sprite(buildings.bmp(selected), mx - MAP_TILE_SIZE / 2, my - MAP_TILE_SIZE / 2, ol::Rgba(MGREEN, 150), .75);
								break;
							case 3:
								buffer_draw_tinted_sprite(buildings.bmp(selected), mx - MAP_TILE_SIZE / 2, my - MAP_TILE_SIZE / 2, ol::Rgba(DYELLOW, 150), .75);
								break;
						}
					}
					else
					{
						draw_trans_tile(set, selected, mx - MAP_TILE_SIZE / 2, my - MAP_TILE_SIZE / 2);
					}
				}
			}
		}
    
    if (moving_tiles == 2)
    {
      if (!(mouse_b & 1))
      {  //if the mouse button is no longer pressed down
        moving_tiles = 3;
      }
    }
    if ((mouse_b & 1) && (my < MAP_TILE_SIZE * YTILES) && (mx < leftx))
    {  //place a tile/unit on the map if the mouse is pressed or remove a unit
      i = mx / MAP_TILE_SIZE + worldmap.scroll_x;
      j = my / MAP_TILE_SIZE + worldmap.scroll_y;
      if (moving_tiles != -1)
      {
        switch(moving_tiles)
        {
          case 0:  //select top left edge
            tx1 = i;
            ty1 = j;
            moving_tiles = 1;
            break;
          case 1:  //select bottom right edge
            if ((i != tx1) || (j != ty1))
            {
              tx2 = i;
              ty2 = j;
              moving_tiles = 2;
              
              if (tx2 < tx1)
              {  //ensure that (tx1, ty1) is the top-left corner
                temp = tx2;
                tx2 = tx1;
                tx1 = temp;
              }
              if (ty2 < ty1)
              {
                temp = ty2;
                ty2 = ty1;
                ty1 = temp;
              }              
            }
            break;
          case 3:
            move_tiles(tx1, ty1, tx2, ty2, i, j);
            selected = -1;  //keeps a tile from being instantly set
            moving_tiles = -1;
            break;
        }
      }
      else
      {
        if ((key[KEY_LCONTROL]) || (key[KEY_RCONTROL]))
        {  //if the mouse clicks while holding ctrl, remove the unit the mouse is on
          z = check_for_unit(i, j);
          if (z != -1)
          {
            remove_unit(z);
          }
        }
        else if (selected != -1)
        {
          if (set != 5)  //if it's tiles, not units being placed
          {
            if ((!worldmap.tile[i][j].same_type(set, selected)) || ((set == 4) && (worldmap.tile[i][j].get_owner() != owner)))
            {                       //from the tile to be placed
              if (set == 4)
              {
                if (selected == 0)
                {  //if it's an HQ being placed, remove previous HQ for that player
                  remove_previous_HQ(owner, i, j);
                }
              }
              play_sound(S_TILEDOWN);
              worldmap.tile[i][j].change_type(set, selected);
              worldmap.tile[i][j].change_owner(owner);
              minimap.create();  //recreate the minimap when a tile is changed
              z = check_for_unit(i, j);
            }
          }
          else  //if a unit is being placed
          {
            new_unit(i, j, selected / 4, selected % 4);
          }
        }
      }
    }

    minimap.draw(true, leftx + 5, 447);
		if (mouse_b & 1)
		{  //center the map on the area of the minimap that was clicked
			mm_mousex = (mouse_x - minimapx - ((150 - minimap.bmp->Width()) / 2)) / 3;
			mm_mousey = (mouse_y - minimapy - ((150 - minimap.bmp->Height()) / 2)) / 3;
			if ((mm_mousex >= 0) && (mm_mousex < worldmap.l))
			{
				if ((mm_mousey >= 0) && (mm_mousey < worldmap.h))
				{
					worldmap.scroll_x = mm_mousex - (XTILES / 2);
					worldmap.scroll_y = mm_mousey - (YTILES / 2);
					worldmap.offset_x = 0;
					worldmap.offset_y = 0;
					scrx = float(worldmap.scroll_x);
					scry = float(worldmap.scroll_y);
					if (worldmap.scroll_x < 0)
					{
						worldmap.scroll_x = 0;
						scrx = 0;
					}
					if (worldmap.scroll_x + XTILES >= worldmap.l)
					{
						worldmap.scroll_x = worldmap.l - XTILES;
						scrx = worldmap.l - XTILES;
					}
					if (worldmap.scroll_y < 0)
					{
						worldmap.scroll_y = 0;
						scry = 0;
					}
					if (worldmap.scroll_y + YTILES >= worldmap.h)
					{
						worldmap.scroll_y = worldmap.h - YTILES;
						scry = worldmap.h - YTILES;
					}
				}
			}
		}

    if ((mx < MAP_TILE_SIZE * XTILES) && (my < MAP_TILE_SIZE * YTILES))
    {  //show the mouse's tile coordinates
      sprintf(msg, "%-1d, %-1d", mx / MAP_TILE_SIZE + worldmap.scroll_x, my / MAP_TILE_SIZE + worldmap.scroll_y);
      buffer_textout_ex(normal, msg, 5, 5, WHITE);
    }
    do_particles();
    draw_mouse();    
  }
  return MEDIT;
}




int resize_map()
{
  static bool first = true;
  static _button up[2], down[2], ok, cancel;
  static int x, y;
  int i, j;
	char msg[30];
  
  if (first)
  {
    up[0].init(270, 160, 40, 15, "+");
    down[0].init(270, 190, 40, 15, "-");
    up[1].init(330, 160, 40, 15, "+");
    down[1].init(330, 190, 40, 15, "-");
    ok.init(270, 215, 100, 20, "Resize");
    cancel.init(270, 250, 100, 20, "Cancel");
    x = worldmap.l;
    y = worldmap.h;
    first = false;
  }
  
  buffer_rectfill(260, 130, 380, 280, GREY);
  buffer_rect(260, 130, 380, 280, WHITE);  
  buffer_textout_centre_ex(normal, "Size:", 320, 140, BLACK);
  sprintf(msg, "%-1d", x);
  buffer_textout_centre_ex(normal, msg, 290, 178, BLACK);
  sprintf(msg, "%-1d", y);
  buffer_textout_centre_ex(normal, msg, 350, 178, BLACK);
  up[0].check();
  up[1].check();
  down[0].check();
  down[1].check();
  ok.check();
  cancel.check();
  
  if (up[0].clicked())
  {
    if (x < MAP_MAX_L) x++;
  }
  if (up[1].clicked())
  {
    if (y < MAP_MAX_H) y++;
  }
  if (down[0].clicked())
  {
    if (x > XTILES) x--;
  }
  if (down[1].clicked())
  {
    if (y > YTILES) y--;
  }
  if (ok.clicked())
  {
    first = true;
    j = 0;
    while (j < MAP_MAX_H)  //set newly accessible tiles to their default
    {
      i = 0;
      while (i < MAP_MAX_L)
      {
        if ((i >= worldmap.l) || (j >= worldmap.h))
        {
          worldmap.tile[i][j].reset();
        }
        i++;
      }
      j++;
    }
    worldmap.l = x;
    worldmap.h = y;
    minimap.create();
    return 0;
  }
  if (cancel.clicked())
  {
    first = true;
    return 0;
  }
  
  draw_mouse();
  return 1;
}




void move_tiles(int x1, int y1, int x2, int y2, int dx, int dy)
{
  int i, j, x, y, z;
  _tile *HQs[4];
  if ((x2 < x1) || (y2 < y1))
  {  //if the bottom-right corner wasn't lower and right-er than the top-left
    return;
  }
  _tile temp[x2 - x1 + 1][y2 - y1 + 1];
  
  z = 0;
  while (z < 4)
  {
    HQs[z] = NULL;
    z++;
  }

  z = 0;
  y = y1;
  j = 0;
  while (y <= y2)
  {
    x = x1;
    i = 0;
    while (x <= x2)
    {
      temp[i][j] = worldmap.tile[x][y];
      if (worldmap.tile[x][y].is_HQ())
      {
        HQs[z] = &worldmap.tile[x][y];
        z++;
      }
      x++;
      i++;
    }
    y++;
    j++;
  }
  
  y = dy;
  j = 0;
  while ((j <= y2 - y1) && (y + j < MAP_MAX_H))
  {
    x = dx;
    i = 0;
    while ((i <= x2 - x1) && (x + i < MAP_MAX_L))
    {
      worldmap.tile[x][y] = temp[i][j];
      x++;
      i++;
    }
    y++;
    j++;
  }
  
  z = 0;
  while (z < 4)
  {  //if an HQ was moved, replace its previous location with a grass tile
    if (HQs[z] != NULL)
    {
      HQs[z]->change_type(0, 3);
    }
    z++;
  }
  minimap.create();
}




void draw_selection(int set, int x)
{
  int i, j, z;
	const int leftx = 800 - MAP_TILE_SIZE * 5;
  bitmapdat *selected;
	
	if ((set != 5) && (set != 4))
  {
    selected = set_datafile(set);
    z = selected->size;
    i = 0;
		j = 0;
    while (j < 6)
    {
      if (x + i < z)
      {
        selection[i].init(leftx + 25, 20 + j * 40, MAP_TILE_SIZE, MAP_TILE_SIZE, "", selected->bmp(x + i));
      }
      else
      {
        selection[i].init(0, -10, 0, 0);  //basically make it unclickable
      }
      i++;
			if (x + i < z)
      {
        selection[i].init(leftx + 65, 20 + j * 40, MAP_TILE_SIZE, MAP_TILE_SIZE, "", selected->bmp(x + i));
      }
      else
      {
        selection[i].init(0, -10, 0, 0);  //basically make it unclickable
      }
			i++;
			if (x + i < z)
      {
        selection[i].init(leftx + 105, 20 + j * 40, MAP_TILE_SIZE, MAP_TILE_SIZE, "", selected->bmp(x + i));
      }
      else
      {
        selection[i].init(0, -10, 0, 0);  //basically make it unclickable
      }
			i++;
			j++;
    }
  }
  else //buildings or mapunits
  {
  	if (set == 4)
  	{
  		selected = set_datafile(4);
    	z = selected->size * 5 - 1;
    }
    else z = NUM_UNITS * 4;
    j = 0;
		i = 0;
    while (j < 6)
    {
			if (x + i < z)
      {
        selection[i].init(leftx + 25, 20 + j * 40, MAP_TILE_SIZE, MAP_TILE_SIZE, "");
      }
      else
      {
        selection[i].init(0, -10, 0, 0);  //basically make it unclickable
      }
      i++;
			if (x + i < z)
      {
        selection[i].init(leftx + 65, 20 + j * 40, MAP_TILE_SIZE, MAP_TILE_SIZE, "");
      }
      else
      {
        selection[i].init(0, -10, 0, 0);  //basically make it unclickable
      }
			i++;
			if (x + i < z)
      {
        selection[i].init(leftx + 105, 20 + j * 40, MAP_TILE_SIZE, MAP_TILE_SIZE, "");
      }
      else
      {
        selection[i].init(0, -10, 0, 0);  //basically make it unclickable
      }
			i++;
      j++;
    }
  }
}

bitmapdat *set_datafile(int set)
{
  switch(set)
  {
    case 0:
      return &land;
      break;
    case 1:
      return &river;
      break;
    case 2:
      return &sea;
      break;
    case 3:
      return &shoal;
      break;
    case 4:
      return &buildings;
      break;
    case 6:
      return &special;
      break;
  }
  return &shoal;  //it should never get here, but returning shoal should make the error obvious if drawn
}

int datafile_size(DATAFILE *data)
{
  int size = 0;
  while (data[size].type != DAT_END)
      size++;
  return size;
}

int add_aifiles(const char *filename, int attrib, void *param)
{
	char *fname = (char*)malloc(sizeof(char) * (strlen(filename) + 1));
	strcpy(fname, get_filename(filename));
	fname[strlen(fname) - 3] = '\0';  //remove extension
	((_listbox*)param)->additem(fname);
	free(fname);
	return 0;
}
int do_aiselect()
{
	const int rows = NUM_UNITS / 2 + NUM_UNITS % 2;
  static bool first = true;
  static int sel;
  static _button done, save, changeweight[MAX_UNITS * 2], changelimit[MAX_UNITS * 2];
	static _listbox presets;
  char t[50];
  int i, j, k;

  if (first)
  {
    done.init(65, 415, 80, 15, "Done");
    save.init(65, 385, 145, 15, "Save as a Preset");
		
		presets.init(65, 65, 145, 310);
		for_each_file_ex("ai/*.ai", 0, FA_DIREC, add_aifiles, &presets);
		presets.sort();
		
    i = 0;
    while (i < 2)
    {
      j = 0;
      while (j < rows)
      {
        changeweight[2 * (j + (rows * i))].init(280 + (160 * i), 65 + (35 * j), 13, 30, "<");
        changeweight[2 * (j + (rows * i)) + 1].init(315 + (160 * i), 65 + (35 * j), 13, 30, ">");
        changelimit[2 * (j + (rows * i))].init(340 + (160 * i), 65 + (35 * j), 13, 30, "<");
        changelimit[2 * (j + (rows * i)) + 1].init(375 + (160 * i), 65 + (35 * j), 13, 30, ">");
        j++;
      }
      i++;
    }
    //position_mouse(320, 240);
    //set_mouse_range(48, 48, 592, 432);
		sel = -1;
    first = false;
  }

  buffer_rectfill(50, 50, 590, 460, GREY, 0.85);
	buffer_rect(50, 50, 590, 460, DGREY);
  buffer_rect(48, 48, 592, 462, DGREY);
  buffer_rect(280, 65, 328, 445, DGREY);
  buffer_rect(340, 65, 388, 445, DGREY);
  buffer_rect(440, 65, 488, 445, DGREY);
  buffer_rect(500, 65, 548, 445, DGREY);
  done.check();
  save.check();
	
  buffer_textout_ex(normal, "AI profile presets:", 70, 53, BLACK);
	presets.check();
  buffer_textout_ex(normal, "Weight", 285, 52, BLACK);
  buffer_textout_ex(normal, "Limit", 350, 52, BLACK);
	
	i = 0;
	while (i < 2)
	{
		j = 0;
		while (j < rows)
		{
			if (j + (rows * i) < NUM_UNITS)
			{
				k = j + (rows * i);
				buffer_draw_sprite(units[k].bmp(0), 240 + (160 * i), 65 + (35 * j));
				
				changeweight[2 * k].check();
				if (changeweight[2 * k].clicked())
				{
					if (theai.weight[k] > 0)
						theai.weight[k]--;
				}
				
				changeweight[2 * k + 1].check();
				if (changeweight[2 * k + 1].clicked())
				{
					if (theai.weight[k] < 101)
						theai.weight[k]++;
				}
				
				
				changelimit[2 * k].check();
				if (changelimit[2 * k].clicked())
				{
					if (theai.limit[k] > 0)
						theai.limit[k]--;
				}
				
				changelimit[2 * k + 1].check();
				if (changelimit[2 * k + 1].clicked())
				{
					if (theai.limit[k] < 51)
						theai.limit[k]++;
				}
				
				sprintf(t, "%-1d", theai.weight[j + (rows * i)]);
				buffer_textout_centre_ex(normal, t, 304 + (160 * i), 76 + (35 * j), BLACK);
				sprintf(t, "%-1d", theai.limit[j + (rows * i)]);
				buffer_textout_centre_ex(normal, t, 364 + (160 * i), 76 + (35 * j), BLACK);
			}
			j++;
		}
		i++;
	}
  
  /*i = 0;
  while (i < NUM_UNITS * 2)
  {
    if (changeweight[i].clicked())
    {
      if (i % 2 == 0)
      {
        if (theai.weight[i / 2] > 0)
        {
          theai.weight[i / 2]--;
        }
      }
      else
      {
        if (theai.weight[i / 2] < 101)
        {
          theai.weight[i / 2]++;
        }
      }
			presets.unselect();
			sel = -1;
    }
    if (changelimit[i].clicked())
    {
      if (i % 2 == 0)
      {
        if (theai.limit[i / 2] > 0)
        {
          theai.limit[i / 2]--;
        }
      }
      else
      {
        if (theai.limit[i / 2] < 51)
        {
          theai.limit[i / 2]++;
        }
      }
			presets.unselect();
			sel = -1;
    }
    i++;
  }*/

  if ((presets.getsel() != -1) && (presets.getsel() != sel))
	{
		sprintf(t, "ai/%s.ai", presets.selected());
  	load_ai_from_file(t);
		sel = presets.getsel();
  }

  if (save.clicked())
  {
    sprintf(t, "New Profile");
    text_box(200, 200, 240, 40, t, 35, 1, "Name the AI profile:");
    if (strcmp(t, "_CANCEL_") != 0)
    {
    	save_new_aiprofile(t);
    	
    	i = 0;
    	j = 0;
    	while (i < presets.size())
    	{
    		if (strcmp(t, presets.getitem(i)) == 0)
    		{
    			j = 1;
    		}
    		i++;
    	}
    	if (j == 0)
				presets.additem(t);
		}
  }
  if (done.clicked())
  {
    first = true;
    //set_mouse_range(0, 0, 639, 479);
    return 0;
  }
  draw_mouse();
  return 1;
}





int save_map()
{
  int i;
  int j;
  char t[50] = "";
  static bool first = true;
  static bool failure = false;
  static string failstring = "";
	
  if (worldmap.get_name() != "")
  {
		empdebug("Saving map:");
		empdebug(worldmap.get_name());
		
    buffer_rectfill(390, 446, 640, 453, LGREY);
    delete_map(worldmap.get_name());
    sprintf(t, "maps/");
    strcat(t, worldmap.get_name().c_str());
    strcat(t, ".map");
    ofstream m(t);
    sprintf(t, "maps/");
    strcat(t, worldmap.get_name().c_str());
    strcat(t, ".ai");
    ofstream a(t);
    if ((m.bad()) || (a.bad()))
    {
      failure = true;
      failstring = "Unable to save ";
      failstring += worldmap.get_name();
    }
    else
    {
      m << worldmap.l << "\n";
      m << worldmap.h << "\n";
      if (HQ_exists(3) == 1)  //if 4 player map
      {
        m << "4" << "\n";
      }
      else if (HQ_exists(2) == 1)  //if 3 player map
      {
        m << "3" << "\n";
      }
      else  //2 player map
      {
        m << "2" << "\n";
      }
      i = 0;
      while (i < worldmap.h)  //saving the tiles
      {
        j = 0;
        while (j < worldmap.l)
        {
          m << (worldmap.tile[j][i].get_set() * 100) + worldmap.tile[j][i].get_number() << " ";
          m << worldmap.tile[j][i].get_owner() << " ";
          j++;
        }
        m << "\n";
        i++;
      }
      i = 0;
      while (i < 200)  //saving deployed units
      {
        if (mapunit[i].exist == 1)
        {
          m << mapunit[i].x << "\n";
          m << mapunit[i].y << "\n";
          m << mapunit[i].type << "\n";
          m << mapunit[i].player << "\n";
        }
        else
        {
          m << "-1\n";
          i = 200;
        }
        i++;
      }
      i = 0;
      while (i < NUM_UNITS)
      {
        a << theai.weight[i] << "\n" << theai.limit[i] << "\n";
        i++;
      }
      m.close();
      a.close();
    }
    if (!failure)
    {
      counter = 0;
      create_particle(320 - normal.Width("Map Saved") / 2, 200, P_FADETEXT, WHITE, "Map Saved");
      medit();
      return MEDIT;
    }
  }
  else
  {
    failure = true;
    failstring = "The map needs a name";
  }
  if (failure)
  {
    if (first)
    {
      map_message("CLEAR");
      map_message(failstring);
      first = false;
    }
    worldmap.draw_tiles();
    buffer_rectfill(0, 385, 640, 480, LGREY);
    buffer_hline(0, 384, 640, BLACK);
    draw_units(worldmap.scroll_x, worldmap.scroll_y);
    map_message("-");
    if (key[KEY_ENTER])
    {
      failure = false;
      first = true;
      return MEDIT;
    }
  }
  return MSAVE;
}





void draw_minimap(int x, int y, string file)
{
  int i, j, a, b, c, d, p, z, s, n, o;
  bool redo = true;
  static string previous = "";

  if (file == previous)
  {
    redo = false;
  }
  if (redo)
  {
    empdebug("Recreating minimap...");
		empdebug("  Reading data from file...");
		worldmap.reset();
    ifstream m(file.c_str());
    if (m.bad())
    {
      redo = false;
      return;
    }
    m >> worldmap.l >> worldmap.h >> p;
    j = 0;
    while (j < worldmap.h)
    {
      i = 0;
      while (i < worldmap.l)
      {
        m >> z;
        s = z / 100;
        n = z % 100;
        m >> o;
        worldmap.tile[i][j].change_type(s, n);
        worldmap.tile[i][j].change_owner(o);
        i++;
      }
      j++;
    }

    clear_all_units();
    i = 0;
    while (i < 50 * p)
    {
      m >> a;
      if (a != -1)  //if there is a saved unit for this i
      {
        m >> b >> c >> d;
        new_unit(a, b, c, d, 0);
        worldmap.tile[a][b].set_unit(d, c);
      }
      else  //if a reads -1, there are no more saved units
      {
        i = 50 * p;
      }
      i++;
    }
		empdebug("  ...done.  Calling minimap.create()...");
    minimap.create();
		empdebug("  Done");
    minimap.a = 1.0;

    m.close();
    previous = file;
  }
  minimap.draw(false, x, y, false);
}





void delete_map(string mapname)
{
  vector<string> fname;
  string temp;
  string fullfile = "maps/";
  fullfile += mapname;
  fullfile += ".map";
  delete_file(fullfile.c_str());
  fullfile = "maps/";
  fullfile += mapname;
  fullfile += ".ai";
  delete_file(fullfile.c_str());
}



void remove_previous_HQ(int player, int ignorex, int ignorey)
{
  int i = 0;
  int j;
  _tile *t;
  while (i < worldmap.h)
  {
    j = 0;
    while (j < worldmap.l)
    {
      if ((j != ignorex) || (i != ignorey))
      {
        t = &worldmap.tile[j][i];
        if ((t->is_HQ()) && (t->owned_by(player)))
        {
          t->reset();
        }
      }
      j++;
    }
    i++;
  }
}





int mcheck()
{
  static bool checked = false;
  char themessage[40];
  int i;
  worldmap.draw_tiles(worldmap.scroll_x, worldmap.scroll_y, XTILES, YTILES, false);
  draw_units(worldmap.scroll_x, worldmap.scroll_y);
  
  if (!checked)
  {
		empdebug("Checking map for problems...");
    map_message("CLEAR");
    if (!HQ_exists(0))
    {
      map_message("Player 1 has no HQ");
    }
    if (!HQ_exists(1))
    {
      map_message("Player 2 has no HQ");
    }
    i = 0;
    while (i < 4)
    {
      if (HQ_exists(i))
      {
        if (!base_exists(i) && (munit_exists(i) == 0))
        {
          sprintf(themessage, "Player %d has no bases or units", i + 1);
          map_message(themessage);
        }
      }
      if (base_exists(i) || (munit_exists(i) == 1))
      {
        if (!HQ_exists(i) && (i > 1))
        {  //(i > 1) so player 1 & 2 HQ errors aren't displayed twice
          sprintf(themessage, "Player %d has no HQ", i + 1);
          map_message(themessage);
        }
      }
      i++;
    }
		empdebug("Finished checking map");
    checked = true;
  }
  map_message("-");
  if (key[KEY_ENTER])
  {
    checked = false;
    return MEDIT;
  }
  return MCHECK;
}

void map_message(string message)
{
  unsigned int i = 0;
  static vector<string> display;
  if (message == "CLEAR")
  {
    display.clear();
		return;
  }
  else if (message == "-")
  {  //do nothing but display the previous messages
  }
  else
  {
    display.push_back(message);
		return;
  }
  buffer_rectfill(140, 100, 500, 350, GREY);
  buffer_rect(140, 100, 500, 350, BLACK);
  buffer_textout_centre_ex(normal, "Map Errors:", 320, 120, RED);
  buffer_textout_centre_ex(normal, "Press Enter", 320, 330, BLACK);
  i = 0;
  while (i < display.size())
  {
    buffer_textout_centre_ex(normal, display[i].c_str(), 320, 145 + (12 * i), BLACK);
    i++;
  }
}

bool HQ_exists(int playernum)
{
  int x, y;
  _tile *t;
  y = 0;
  while (y < worldmap.h)
  {
    x = 0;
    while (x < worldmap.l)
    {
      t = &worldmap.tile[x][y];
      if (t->is_HQ())
      {
        if (t->owned_by(playernum))
        {
          return true;
        }
      }
      x++;
    }
    y++;
  }
  return false;
}

bool base_exists(int playernum)
{
  int x, y;
  _tile *t;
  y = 0;
  while (y < worldmap.h)
  {
    x = 0;
    while (x < worldmap.l)
    {
      t = &worldmap.tile[x][y];
      if (t->is_unit_producing())
      {
        if (t->owned_by(playernum))
        {
          return true;
        }
      }
      x++;
    }
    y++;
  }
  return false;
}

void draw_trans_tile(int set, int num, int x, int y)
{
  buffer_blit_clipped(set_datafile(set)->bmp(num), x, y, 0, 0, MAP_TILE_SIZE, MAP_TILE_SIZE, 0.66);
}
