#include "empire.h"

int mapcolor(int set, int num, int owner, bool defaultcolors);

void _minimap::create(bool defaultcolors)
{
  int x, y, s, n, z;
  time = 0;
  draw_called = false;
  a = 0;
  
	if (bmp)
	{
	  bmp->Destroy();
	}
	bmp = new ol::Bitmap(worldmap.l * 3, worldmap.h * 3);
  
	ol::Canvas::SetTo(*bmp);
	ol::Canvas::SetPixelWriteMode(ol::COLOR_AND_ALPHA);
	ol::Settings::RestoreOldProjection();

	ol::Rect(0, 0, worldmap.l * 3, worldmap.h * 3).Draw(ol::Rgba(BLACK, 255));
  _tile *t;
  y = 0;
  while (y < worldmap.h)
  {
    x = 0;
    while (x < worldmap.l)
    {
      t = &worldmap.tile[x][y];
      s = t->get_set();
      n = t->get_number();
      z = x + y * worldmap.l;
      
			ol::Rect(float(x * 3), float(y * 3), 3.0, 3.0).Draw(ol::Rgba(mapcolor(s, n, t->get_owner(), defaultcolors), 255));
			
      if (s == 4)  //buildings
      {
        if (n > 0)  //if a normal building
        {
					ol::Rect(float(x * 3), float(y * 3), 2.0, 2.0).DrawOutline(ol::Rgba(GREY, 255));
        }
        else  //if an HQ
        {
					ol::Rect(float(x * 3), float(y * 3), 2.0, 2.0).DrawOutline(ol::Rgba(WHITE, 255));
        }
      }
			if (s == 6)  //special tiles
			{
				if (n == 0) //teleporter
				{
					ol::Point(float(x * 3 + 1), float(y * 3)).Draw(ol::Rgba(WHITE, 255));
					ol::Point(float(x * 3), float(y * 3 + 2)).Draw(ol::Rgba(WHITE, 255));
					ol::Point(float(x * 3 + 2), float(y * 3 + 3)).Draw(ol::Rgba(WHITE, 255));
				}
			}

      x++;
    }
    y++;
  }

	ol::Canvas::Refresh();
	ol::Canvas::SetTo(ol::SCREEN_BACKBUF);
	ol::Settings::SetOrthographicProjection(800, 600);
}

void _minimap::check()
{  //this function should be called during the main game's logic loop
  if (draw_called)
  {  //if the minimap was drawn last logic loop
    draw_called = false;
    a += 0.03;
    if (a > 0.7) a = 0.7;
  }
  else
  {
    a -= 0.05;
    if (a <= 0) a = 0;
    if (a > 0) draw(false);
    draw_called = false;
  }
}

void _minimap::draw(bool mapeditor, int x, int y, bool outline, bool defaultcolors)
{
	const int color[4] = {player_color(0, 0), player_color(1, 0), player_color(2, 0), player_color(0, 2)};
  int i, j, z;
  
  draw_called = true;
	
	buffer_rectfill(x, y, x + 150, y + 150, BLACK);
	x += (150 - bmp->Width()) / 2;
	y += (150 - bmp->Height()) / 2;

  buffer_draw_sprite(bmp, x, y);//, a);
  
  time++;
  if (time / 50 == 0)
  {  //show where the units are
    j = 0;
    while (j < worldmap.h)
    {
      i = 0;
      while (i < worldmap.l)
      {
        z = worldmap.tile[i][j].unit_here();
        if (z != -1)
        {
        	if (defaultcolors)
        		buffer_mapblip(i * 3 + x - 2, j * 3 + y - 2, color[z / 100]);
        	else
          	buffer_mapblip(i * 3 + x - 2, j * 3 + y - 2, player[z / 100].tintcolor);//, a + 0.2);
        }
        i++;
      }
      j++;
    }
  }
  if (time > 100)
  {
    time = 0;
  }
  if (outline)
  {
    if (mapeditor)  //show where the viewable area is if in the map editor
    {
      buffer_rect((worldmap.scroll_x * 3) + 1 + x, (worldmap.scroll_y * 3) + 1 + y, (worldmap.scroll_x + XTILES) * 3 + x, (worldmap.scroll_y + YTILES) * 3 + y, BLACK);
    }
    else
    {
      buffer_rect((worldmap.scroll_x * 3) + 1 + x, (worldmap.scroll_y * 3) + y, (worldmap.scroll_x + XTILES) * 3 + x, (worldmap.scroll_y + YTILES) * 3 + y - 1, WHITE);
    }
  }
}

void _minimap::destroy()
{
  if (bmp)
  {
    bmp->Destroy();
  }
}

int mapcolor(int set, int num, int owner, bool defaultcolors)
{
	const int dcolor[4] = {player_color(0, 0), player_color(1, 0), player_color(2, 0), player_color(0, 2)};
  int color = BLUE;
  if (set == 0)
  {
    if ((num == 0) || (num == 1) || (num > 4))
    {  //if it is a road
      color = DGREY;
    }
    else if (num == 2)
    {
      color = makecol(0, 100, 0);
    }
    else if (num == 3)
    {
      color = GREEN;
    }
    else if (num == 4)
    {
      color = makecol(190, 180, 20);
    }
  }
  else if ((set == 2) && (num == 1))  //reef
  {
    color = makecol(200, 200, 250);
  }
  else if (set == 3)  //shoal
  {
    color = makecol(225, 220, 140);
  }
  else if (set == 4)  //buildings
  {
		if (owner != -1)
		{
			if (defaultcolors)
				color = dcolor[owner];
			else
				color = player[owner].tintcolor;
		}
		else
		{
			color = LGREY;
		}
	}
  else if (set == 6)
  {
    switch (num)
    {
      case 0:  //teleport
        color = makecol(128, 128, 255);
        break;
      case 1:  //barrier
        color = BLACK;
        break;
    }
  }
  return color;
}
