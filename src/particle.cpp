#include "empire.h"
#include "logic.h"  //needed for P_CASH

void create_particle(int x, int y, int type);
void create_particle(int x, int y, int type, int color, string text);

list<_particle> p_effects;

void _particle::create(int _x, int _y, int _type, int _color, string _text)
{
  int a, b, z;
  x = float(_x);
  y = float(_y);
  type = _type;
  a = rand()%2;
  if (a == 0) a = -1;
  b = rand()%2;
  if (b == 0) b = -1;
  color = _color;
  text = _text;
	//delay = 0;
  switch(type)
  {
    case P_FLAME:
      xv = (float(rand()%30) / 10) * float(a);
      yv = (float(rand()%30) / 10) * float(b);
      xa = 0;
      ya = 0;
      life = 8;
      break;
    case P_DUST:
      xv = (float(rand()%5) / 10) * float(a);
      yv = (float(rand()%5) / 10) * float(b) - 0.5;
      xa = 0;
      ya = 0;
      life = 10;
      break;
    case P_SMOKE:
      xv = (float(rand()%5) / 10) * float(a);
      yv = (float(rand()%2) / 10) * float(a);
      xa = 0;
      ya = -0.02;
      life = 25;
      break;
    case P_AIRTRAIL:
      xv = (float(rand()%6) / 10) * float(a);
      yv = (float(rand()%6) / 10) * float(b);
      xa = 0;
      ya = 0;
      life = 10;
      break;
    case P_SEATRAIL:
      xv = (float(rand()%5) / 10) * float(a);
      yv = (float(rand()%5) / 10) * float(b);
      xa = 0;
      ya = 0;
      life = 30;
      break;
    case P_HORIZONTAL:
      xv = (3.0 + float(rand()%12)) * float(a);
      yv = 0;
      xa = 0;
      ya = 0;
      life = 20;
      break;
    case P_FADETEXT:
      xv = 0;
      yv = 0;
      xa = 0;
      ya = 0;
      life = 50;
      break;
    case P_SHOCKWAVE:
      xv = 0;
      yv = 0;
      xa = 0;
      ya = 0;
      life = 25;
      break;
    case P_TELEPORT:
      xv = 0;
      yv = 0;
      xa = 0;
      ya = 0;
      life = 16;
      break;
		case P_SHOTHIT:
			xv = 0;
		  yv = 0;
		  xa = 0;
		  ya = 0;
		  life = 6;
		  break;
		case P_REPAIR:
			xv = 0;
			yv = -1.0;
			xa = 0;
			ya = 0;
			life = 50;
			break;
		case P_CASH:
			xv = 0;
			yv = -1.5;
			xa = 0;
			ya = 0;
			life = 50;
			break;
		case P_FIRE:
			z = rand()%3;
      if (z == 0) color = RED;
      else if (z == 1) color = ORANGE;
      else color = YELLOW;
			xv = float(rand()%20) / 10 - 1;
			yv = float(rand()%10) / 10 + 0.25;
			xa = 0;
			ya = -0.3 - float(rand()%10) / 10;
			life = 5;
			break;
		case P_NUKEBOOM:
			xv = 0;
			yv = 0;
			xa = 0;
			ya = 0;
			life = 52;
			break;
		case P_QUADSPIRAL:
			xv = 0;
			yv = 0;
			xa = 0;
			ya = 0;
			life = 50;
			break;
		case P_LOGO:
			xv = float(rand()%20 + 10) / 10.0 * float(a);
			yv = float(rand()%20 + 10) / 10.0 * float(b);
			xa = 0;
			ya = 0;
			life = 100;
			break;
  }
  exists = true;
}

void _particle::move()
{
  xv += xa;
  x += xv;
  yv += ya;
  y += yv;
  life--;
  if (life <= 0)
  {
    exists = false;
  }
}

void _particle::draw()
{
  int screenx = int(x) - (worldmap.scroll_x * MAP_TILE_SIZE) + worldmap.offset_x;
  int screeny = int(y) - (worldmap.scroll_y * MAP_TILE_SIZE) + worldmap.offset_y;
  int r, t1, t2;
  float ang;
  
  if (((screenx > -10) && (screenx < 650) && (screeny > -10) && (screeny < 610)) || (type == P_FADETEXT) || (type == P_LOGO))
  {
    switch(type)
    {
      case P_FLAME:
        r = (10 - life) / 5;
        buffer_circlefill(screenx, screeny, r, color);
        break;
      case P_DUST:
        r = (10 - life) / 5;
        buffer_circlefill(screenx, screeny, float(r + 1), LGREY, float(life) * 0.3);
        break;
      case P_SMOKE:
        r = (25 - life) / 5;
        buffer_circlefill(screenx, screeny, r, color, float(life) / 15.0);
        break;
      case P_AIRTRAIL:
        buffer_putpixel(screenx, screeny, WHITE, 0.6);
        break;
      case P_SEATRAIL:
        buffer_putpixel(screenx, screeny, makecol(170, 170, 255));
        break;
      case P_HORIZONTAL:
        buffer_putpixel(screenx, screeny, color);
        if (life % 5 == 0)
        {
          buffer_putpixel(screenx + 1, screeny, color);
          buffer_putpixel(screenx - 1, screeny, color);
          buffer_putpixel(screenx, screeny + 1, color);
          buffer_putpixel(screenx, screeny - 1, color);
        }
        else
        {
          buffer_putpixel(screenx + 1, screeny + 1, color);
          buffer_putpixel(screenx - 1, screeny + 1, color);
          buffer_putpixel(screenx + 1, screeny - 1, color);
          buffer_putpixel(screenx - 1, screeny - 1, color);
        }
        break;
      case P_FADETEXT:
        buffer_textout_ex(big, text, int(x), int(y), color, float(life) / 50);
        break;
      case P_SHOCKWAVE:
        r = (25 - life) * 3;
        buffer_circlefill(screenx, screeny, r, WHITE, float(life) / 25.0);
        break;
      case P_TELEPORT:
        t1 = screenx + MAP_TILE_SIZE / 2;
        t2 = screeny + MAP_TILE_SIZE / 2;
        if (life > 8)
        {
          buffer_rectfill(t1 - (16 - life)*2, 0, t1 + (16 - life)*2, t2, WHITE);
          buffer_circlefill(t1, t2, (16 - life)*2, WHITE);
        }
        else
        {
          buffer_rectfill(t1 - life*2, 0, t1 + life*2, t2, WHITE);
          buffer_circlefill(t1, t2, life*2, WHITE);
        }
        break;
			case P_SHOTHIT:
				buffer_circlefill(screenx, screeny, life, makecol(220 - life * 20, 0, 0), 0.5);
			  create_particle(int(x), int(y), P_SMOKE);
			  break;
			case P_CASH:
				buffer_textout_ex(big, "$", screenx, screeny, BLACK, (2.0 * float(life)) / 100);
				break;
			case P_REPAIR:
				glb.draw(38, screenx, screeny, (2.0 * float(life)) / 100);
				break;
			case P_FIRE:
				buffer_additive_circlefill(screenx, screeny, 3, color, life / 5.0);
				buffer_putpixel(screenx + rand()%16 - 8, screeny + rand()%16 - 8, color, life / 4.0);
				break;
			case P_NUKEBOOM:
				buffer_rectfill(0, 0, 800, 600, WHITE, 1.0 - (float(52 - life) / 52.0));
				buffer_draw_sprite(nukeboom.bmp(26 - (life / 2)), screenx - 112, screeny - 200);
				break;
			case P_QUADSPIRAL:
				ang = life / 18.0;
				buffer_circlefill(screenx + int(life * 0.5 * cos(ang)), screeny + int(life * 0.5 * sin(ang)), 5, color, 0.7);
				ang += AL_PI / 2;
				buffer_circlefill(screenx + int(life * 0.5 * cos(ang)), screeny + int(life * 0.5 * sin(ang)), 5, color, 0.7);
				ang += AL_PI / 2;
				buffer_circlefill(screenx + int(life * 0.5 * cos(ang)), screeny + int(life * 0.5 * sin(ang)), 5, color, 0.7);
				ang += AL_PI / 2;
				buffer_circlefill(screenx + int(life * 0.5 * cos(ang)), screeny + int(life * 0.5 * sin(ang)), 5, color, 0.7);
				
				create_particle(int(x) + rand()%MAP_TILE_SIZE - MAP_TILE_SIZE / 2, int(y) + rand()%MAP_TILE_SIZE - MAP_TILE_SIZE / 2, P_DUST);
				break;
			case P_LOGO:
				if (life > 60)
				{
					glb.draw(40, int(x), int(y), float(100 - life) / 40.0);
				}
				else if (life <= 40)
				{
					glb.draw(40, int(x), int(y), float(life) / 40.0);
				}
				else glb.draw(40, int(x), int(y));
				break;
    }
  }
}

void create_particle(int x, int y, int type)
{
	if (type == P_SMOKE)
	{
		create_particle(x, y, type, GREY);
	}
	else
	{
  	create_particle(x, y, type, WHITE);
  }
}

void create_particle(int x, int y, int type, int color, string text)
{
  _particle t;
  if (global.particles == 0)
  {
    return;
  }
	t.create(x, y, type, color, text);
	p_effects.push_front(t);
}

void do_particles()
{
	list<_particle>::iterator it = p_effects.begin();
	
  while (it != p_effects.end())
  {
		/*if ((*it).delay > 0)
		{
			(*it).delay--;
		}
		else */if (((*it).type != P_CASH) || (var.logicstate != SCROLLING))
		{  //special case: cash icons don't show when scrolling
			(*it).move();
			(*it).draw();
		}

		if (!(*it).exists)
		{
			it = p_effects.erase(it);
			it--;
		}
		it++;
  }
}

void clear_particles()
{
  p_effects.clear();
}
