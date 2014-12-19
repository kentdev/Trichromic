#include "empire.h"
#include "battleshot.h"

vector<_battleshot> bshot;

vector<_loc> bulletpoints(_unit *a, _unit *d)
{
	vector<_loc> bulletpoints;
	_loc t;
	
	switch (a->direction)
	{
		case UP:
			t.x = MAP_TILE_SIZE / 2;
		  t.y = MAP_TILE_SIZE / 4;
		  break;
		case DOWN:
			t.x = MAP_TILE_SIZE / 2;
		  t.y = 3 * MAP_TILE_SIZE / 4;
		  break;
		case RIGHT:
			t.x = 3 * MAP_TILE_SIZE / 4;
		  t.y = MAP_TILE_SIZE / 2;
		  break;
		case LEFT:
			t.x = MAP_TILE_SIZE / 4;
		  t.y = MAP_TILE_SIZE / 2;
		  break;
  }
	bulletpoints.push_back(t);
	
	//don't bother making this any more precise until I can get some decent unit graphics

	return bulletpoints;
}

void create_battleshot(int type, int ox, int oy, int dx, int dy, int delay, int sound)
{
	const float tconst = 10.0;
	const float gravity = 0.9;
  _battleshot b;
	
	ox += -3 + rand()%6;
	oy += -3 + rand()%6;
	
	b.type = type;
	b.x = float(ox);
	b.y = float(oy);
	b.dx = float(dx);
	b.dy = float(dy);
	b.delay = delay;
	b.exists = true;
	b.time = int(tconst);
	b.sound = sound;
	b.playedsound = false;
	
	switch (type)
	{
		case B_CANNON:
			create_particle(ox, oy, P_SMOKE, WHITE);
		  b.xv = float(dx - ox) / (tconst / 2);
		  b.yv = float(dy - oy) / (tconst / 2);
		  b.xa = 0;
		  b.ya = 0;
		  b.striplen = 3;
		  b.start = ol::Rgba(1.0, 1.0, 1.0, 1.0);
		  b.end = b.start;
		  b.time /= 2;
		  break;
		case B_BULLET:
		  b.xv = float(dx - ox) / tconst;
		  b.yv = float(dy - oy) / tconst;
		  b.xa = 0;
		  b.ya = 0;
		  b.striplen = 3;
		  b.start = ol::Rgba(1.0, 1.0, 0.0, 1.0);
		  b.end = ol::Rgba(1.0, 1.0, 1.0, 1.0);
		  break;
		case B_BALLISTIC:
			create_particle(ox, oy, P_SMOKE);
			b.xv = float(dx - ox) / (tconst * 2);
		  b.yv = (float(dy - oy) / (tconst * 2)) + (-gravity * tconst);
		  b.xa = 0;
		  b.ya = gravity;
		  b.striplen = 3;
		  b.time *= 2;
		  b.start = ol::Rgba(1.0, 1.0, 1.0, 1.0);
  		b.end = ol::Rgba(0.0, 0.0, 0.0, 0.0);
		  break;
		case B_NUKE:
			create_particle(ox, oy, P_SMOKE);
			b.xv = float(dx - ox) / (tconst * 4);
		  b.yv = (float(dy - oy) / (tconst * 4)) + (-gravity * tconst);
		  b.xa = 0;
		  b.ya = gravity / 2;
		  b.striplen = 3;
		  b.time *= 4;
		  b.start = ol::Rgba(1.0, 1.0, 1.0, 1.0);
  		b.end = ol::Rgba(0.0, 0.0, 0.0, 0.0);
			break;
		case B_BALTRAIL:
			create_particle(ox, oy, P_SMOKE);
			b.xv = float(dx - ox) / (tconst * 2);
		  b.yv = (float(dy - oy) / (tconst * 2)) + (-gravity * tconst);
		  b.xa = 0;
		  b.ya = gravity;
		  b.striplen = 20;
		  b.time *= 2;
		  b.start = ol::Rgba(1.0, 1.0, 1.0, 1.0);
  		b.end = ol::Rgba(0.0, 0.0, 0.0, 0.0);
			break;
		case B_ROCKET:
			create_particle(ox, oy, P_SMOKE);
		  b.xv = float(dx - ox) / (tconst * 2);
		  b.yv = float(dy - oy) / (tconst * 2);
		  b.xa = 0;
		  b.ya = 0;
		  b.striplen = 5;
		  b.time *= 2;
		  b.start = ol::Rgba(1.0, 1.0, 0.0, 1.0);
		  b.end = ol::Rgba(1.0, 0.0, 0.0, 0.5);
		  break;
		case B_BOMB:
			b.xv = float(dx - ox) / tconst;
		  b.yv = float(dy - oy) / tconst;
		  b.xa = 0;
		  b.ya = gravity / 4;
		  b.striplen = 2;
		  b.start = ol::Rgba(0.8, 0.8, 0.8, 1.0);
		  b.end = b.start;
		  break;
		case B_TORPEDO:
			b.xv = float(dx - ox) / (tconst * 3);
		  b.yv = float(dy - oy) / (tconst * 3);
		  b.xa = 0;
		  b.ya = 0;
		  b.striplen = 10;
		  b.start = ol::Rgba(1.0, 1.0, 1.0, 0.9);
		  b.end = ol::Rgba(1.0, 1.0, 1.0, 0.0);
		  b.time *= 3;
		  break;
		case B_SNIPER:
			b.xv = float(dx - ox) / (tconst / 2);
		  b.yv = float(dy - oy) / (tconst / 2);
			b.xa = 0;
			b.ya = 0;
			b.striplen = 6;
			b.start = ol::Rgba(1.0, 1.0, 1.0, 0.9);
			b.end = ol::Rgba(1.0, 1.0, 1.0, 0.0);
			b.time /= 2;
			break;
	}
	
	bshot.push_back(b);
}

bool do_battleshots()
{  //returns true when finished
	bool alldone = true;
	unsigned int i = 0;
	float screenx, screeny;
	
	i = 0;
	while (i < bshot.size())
	{
		if (bshot[i].exists)
		{
			alldone = false;
			if (bshot[i].delay > 0)
			{
				bshot[i].delay--;
			}
			else
			{
				if (!bshot[i].playedsound)
				{
					play_sound(bshot[i].sound);
					bshot[i].playedsound = true;
				}
				
				bshot[i].time--;
				
				bshot[i].x += bshot[i].xv;
				bshot[i].y += bshot[i].yv;
				bshot[i].xv += bshot[i].xa;
				bshot[i].yv += bshot[i].ya;
				
				screenx = bshot[i].x - (worldmap.scroll_x * MAP_TILE_SIZE) + worldmap.offset_x;
				screeny = bshot[i].y - (worldmap.scroll_y * MAP_TILE_SIZE) + worldmap.offset_y;
				
				if (bshot[i].strip.GetNumOfVertices() > bshot[i].striplen)
				{
					bshot[i].strip.DeleteLast();
				}
				
				bshot[i].strip.AddToBegin(ol::Vec2D(screenx, screeny));
				
				if (bshot[i].time <= 0)
				{  //if the particle has finished, create different explosion effects
					switch(bshot[i].type)
					{
						case B_BALLISTIC:
  						create_particle(int(bshot[i].x), int(bshot[i].y) + 6, P_SHOTHIT);
						  break;
						case B_BOMB:
							create_particle(int(bshot[i].x) - 4 + rand()%8, int(bshot[i].y) - rand()%7, P_SHOTHIT);
						  create_particle(int(bshot[i].x) - 4 + rand()%8, int(bshot[i].y) - rand()%7, P_SHOTHIT);
						  create_particle(int(bshot[i].x) - 4 + rand()%8, int(bshot[i].y) - rand()%7, P_SHOTHIT);
						  break;
					  case B_TORPEDO:
							create_particle(int(bshot[i].x), int(bshot[i].y), P_SHOTHIT);
						  break;
						case B_NUKE:
							create_particle(int(bshot[i].x), int(bshot[i].y), P_NUKEBOOM);
							break;
					}
					bshot[i].exists = false;
					play_sound(S_SHOTHIT1);
				}
				else
				{  //or, if it's still going, create whatever movement-related particles it should have
					switch(bshot[i].type)
					{
						case B_TORPEDO:
							//create_particle(int(bshot[i].x), int(bshot[i].y), P_SEATRAIL);
						  break;
					}
				}
				
				if (bshot[i].exists)
				{
					if (bshot[i].strip.GetNumOfVertices() > 1)
					{
						buffer_linestrip(&bshot[i].strip, bshot[i].start, bshot[i].end);
					}
				}
				
				if (bshot[i].type == B_NUKE)
				{
					buffer_rotate_sprite(glb.bmp(37), int(screenx), int(screeny), 64 + int(atan2(bshot[i].yv, bshot[i].xv) * 255 / (2 * AL_PI)));
				}
			
			}
		}
		i++;
	}
	
	if (alldone)
	{
		bshot.clear();
	  return true;
	}
	return false;
}
