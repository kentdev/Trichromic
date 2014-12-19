#include "empire.h"
#include "battleshot.h"

extern void broadcast_unithealth(_unit *u);
extern bool shieldgen_nearby(int tx, int ty, int team, bool ignore_shieldstatus = false);
extern int rounded_health(float h);
extern int attackable_unit_here(_unit *attacker, int tx, int ty);

typedef struct battleparams
{
	vector<pair<_loc, float> > healths[2];  // locations and final healths of all units involved (before / after counterattack)
	vector<_loc> treesburned;  //locations of any trees set on fire
	bool counter;  //whether there's a counterattack
	unsigned int d_attackdelay, a_attackdelay;
	_loc aloc, dloc;
	
	int in_healths(int healtharray, _loc l);
	float get_health(int healtharray, _loc l);
} battleparams;

int battleparams::in_healths(int n, _loc l)
{
	unsigned int i = 0;
	while (i < healths[n].size())
	{
		if ((l.x == healths[n][i].first.x) && (l.y == healths[n][i].first.y))
		{
			return i;
		}
		i++;
	}
	return -1;
}

float battleparams::get_health(int n, _loc l)
{
	int i = in_healths(n, l);
	if (i != -1) return healths[n][i].second;
	return 0;
}

battleparams netparams;

void do_damage(_unit *a, _unit *d, battleparams *p, bool is_counterattack);
void radial_damage(_unit *a, int tx, int ty, battleparams *p, bool is_counterattack);
void get_battlestats(_unit *a, _unit *d, battleparams *p);
void broadcast_battledata(battleparams *p);
void get_battledata(DyBytes *data);
bool rotate_units(_unit *a, _unit *d, int rotationspeed);
bool battle_attack(_unit *a, _unit *d);
int do_battle(_unit *a, _unit *d);
int basic_damage(_unit *a, _unit *b);
int defbonus(int tx, int ty);
float total_damage(_unit *a, _unit *d);

int do_battle(_unit *a, _unit *d)
{
	const short SCROLL_TO_BATTLE   = -2;
	const short SKIPPING_ANIMATION = -1;
	const short UNITS_ROTATING     = 0;
	const short ATTACKER_FIRING    = 1;
	const short DEFENDER_FIRING    = 2;
	const short FINISHED           = 3;
	
	static bool first = true;
	static bool attacked_twice;
	static short battlestatus;
	static battleparams result;
	
	unsigned int i, n;
	int z;
	_unit *u;
	_loc l;
	
  if (first)
  {
		battlestatus = SCROLL_TO_BATTLE;
		
		if (!global.netgame || (global.netgame && net.server))
			get_battlestats(a, d, &result);
		
		if (global.netgame && net.server)
		{
			broadcast_battledata(&result);
		}
		else if (global.netgame)
		{
			result = netparams;
		}
		
		attacked_twice = false;
		first = false;
	}
	
	switch(battlestatus)
	{
		case SCROLL_TO_BATTLE:
			if (scroll_to_location(a->tilex, a->tiley) == 1)
			{
				if (global.battle_animation == 1) battlestatus = UNITS_ROTATING;
				else battlestatus = SKIPPING_ANIMATION;
			}
			break;
		case SKIPPING_ANIMATION:
			empdebug("Skipping battle animation");
			a->attackdelay = result.a_attackdelay;
			d->attackdelay = result.d_attackdelay;
			
			n = 0;
			while (n < 2)
			{
				i = 0;
				while (i < result.healths[n].size())
				{
					l = result.healths[n][i].first;
					z = any_unit_here(l.x, l.y);
					if (z == -1)
					{
						empdebug("Error: no unit on a radialhealths-specified tile");
						empdebug(l.x); empdebug(l.y);
					}
					else
					{
						u = &player[z / 100].unit[z % 100];
						u->health = result.healths[n][i].second;
						if (u->health < 0.01)
						{
							player[u->color].stats.lost++;
							if (n == 0)
							{
								if (u->color != a->color) player[a->color].stats.killed++;
							}
							else
							{
								if (u->color != d->color) player[d->color].stats.killed++;
							}
							u->kill();
						}
					}
					i++;
				}
				n++;
			}
			battlestatus = FINISHED;
			break;
		case UNITS_ROTATING:
			if (rotate_units(a, d, 5))  //rotate_units returns true when it's finished
			{  //rotate fast, then rotate again with more accuracy
				if (rotate_units(a, d, 1))
			  {
  				battlestatus = ATTACKER_FIRING;
  			}
			}
			break;
	  case ATTACKER_FIRING:
			if (battle_attack(a, d))
			{
				i = 0;
				while (i < result.healths[0].size())
				{
					l = result.healths[0][i].first;
					z = any_unit_here(l.x, l.y);
					if (z == -1)
					{
						empdebug("Error: no unit on a radialhealths-specified tile");
						empdebug(l.x); empdebug(l.y);
					}
					else
					{
						u = &player[z / 100].unit[z % 100];
						u->health = result.healths[0][i].second;
						if (u->health < 0.01)
						{
							player[u->color].stats.lost++;
							if (u->color != a->color) player[a->color].stats.killed++;
							u->kill();
						}
					}
					i++;
				}
				
				if (result.counter) battlestatus = DEFENDER_FIRING;
				else battlestatus = FINISHED;
			}
			break;
		case DEFENDER_FIRING:
			if (battle_attack(d, a))
			{
				i = 0;
				while (i < result.healths[1].size())
				{
					l = result.healths[1][i].first;
					z = any_unit_here(l.x, l.y);
					if (z == -1)
					{
						empdebug("Error: no unit on a radialhealths-specified tile");
						empdebug(l.x); empdebug(l.y);
					}
					else
					{
						u = &player[z / 100].unit[z % 100];
						u->health = result.healths[1][i].second;
						if (u->health < 0.01)
						{
							player[u->color].stats.lost++;
							if (u->color != d->color) player[d->color].stats.killed++;
							u->kill();
						}
					}
					i++;
				}
				battlestatus = FINISHED;
			}
			break;
		case FINISHED:
		default:
			i = 0;
			while (i < result.treesburned.size())
			{
				l = result.treesburned[i];
				if ((l.x < 0) || (l.x > worldmap.l) || (l.y < 0) || (l.y > worldmap.h))
				{
					empdebug("Error: treesburned location not on the map");
					empdebug(l.x); empdebug(l.y);
				}
				else
				{
					worldmap.tile[l.x][l.y].burning = true;
					worldmap.tile[l.x][l.y].burnedfor = 0;
				}
				i++;
			}
			
			a->ready = 0;  // For battle animations, the unit needs to be marked "unready" afterwards
			first = true;
		  return 0;
	}
  return 1;
}

bool rotate_units(_unit *a, _unit *d, int rotationspeed)
{  //if it's a direct attack, rotate the defender to return fire
	static bool first = true;
	static int arotate, drotate;
	bool rotate_defender = (unitstats[a->type].attacktype == DIRECT);
	bool adone = false;
	bool ddone = false;
	
	if (first)
	{
		arotate = int((256 * atan2(d->tiley - a->tiley, d->tilex - a->tilex)) / (2 * M_PI));
		drotate = int((256 * atan2(a->tiley - d->tiley, a->tilex - d->tilex)) / (2 * M_PI));
  	while (arotate > 255) arotate -= 256;  //make sure rotation is between 0 and 255
  	while (arotate < 0) arotate += 256;
		while (drotate > 255) drotate -= 256;  //make sure rotation is between 0 and 255
  	while (drotate < 0) drotate += 256;
		first = false;
	}
	
	if (abs(arotate - a->rotation) > rotationspeed)
	{
		if (a->rotation < arotate)
		{
			if (abs(a->rotation - arotate) > 128)
			{
				a->rotation -= rotationspeed;
			}
			else
			{
				a->rotation += rotationspeed;
			}
		}
	  else
		{
			if (abs(a->rotation - arotate) > 128)
			{
				a->rotation += rotationspeed;
			}
			else
			{
	  		a->rotation -= rotationspeed;
			}
		}
	}
	else
	{
		adone = true;
	}
	
	if (rotate_defender)
	{
		if (abs(drotate - d->rotation) > rotationspeed)
		{
  		if (d->rotation < drotate)
		  {
  			if (abs(d->rotation - drotate) > 128)
			  {
  				d->rotation -= rotationspeed;
			  }
			  else
			  {
  				d->rotation += rotationspeed;
			  }
		  }
	    else
		  {
  			if (abs(d->rotation - drotate) > 128)
			  {
  				d->rotation += rotationspeed;
			  }
			  else
			  {
  	  		d->rotation -= rotationspeed;
			  }
		  }
		}
		else
		{
			ddone = true;
		}
	}
	else
	{
		ddone = true;
	}
	
	if ((adone) && (ddone))
	{
		first = true;
		return true;
	}
	return false;
}

bool battle_attack(_unit *a, _unit *d)
{  //just the attack animation, returns true when finished
	static bool first = true;
	static bool createparticles = true;
	static bool createdparticles = false;
	static int y, offset_y, btype;
	vector<_loc> points;
	int i, j;
	
	if (first)
	{
		y = 0;
		offset_y = 0;
		createparticles = true;
		createdparticles = false;
		
		btype = unitstats[a->type].attackparticle;
		a->attackdelay = unitstats[a->type].attackdelay;
		
		first = false;
	}

	if ((createparticles) && (!createdparticles))
	{
    points = bulletpoints(a, d);
	  i = 0;
	  while ((i < rounded_health(a->health)) && ((btype != B_NUKE) || (i < 1)))  //only one shot particle if it's a nuke
	  {
 			j = rand()%points.size();
		  create_battleshot(btype, MAP_TILE_SIZE * a->tilex + points[j].x, 
												MAP_TILE_SIZE * a->tiley + points[j].y + offset_y, 
											  MAP_TILE_SIZE * d->tilex + MAP_TILE_SIZE / 2, 
												MAP_TILE_SIZE * d->tiley + MAP_TILE_SIZE / 2,
											  i * 3, unitstats[a->type].attacksound);
		  i++;
	  }
		createdparticles = true;
	}
	
	if (!createparticles)
	{
		switch (unitstats[a->type].attackparticle)
		{
			case B_BOMB:
				a->reposition(0, -1);
			  y++;
			  if (y >= MAP_TILE_SIZE / 2)
				{
					createparticles = true;
				}
				offset_y--;
			  break;
		}
	}
	else if (do_battleshots())
	{
		if (offset_y != 0)
		{
			a->reposition(0, -offset_y);
		}
		first = true;
	  return true;
	}
	return false;
}

int basic_damage(_unit *a, _unit *b)
{
	int d = 0, i = 0;
	static _unit *oldb = NULL;
		
	while (i < 3)
	{
		if (b != oldb) empdebug(b->resist[i]);
		if (a->attack[i] - b->resist[i] > 0)
		{
			d += a->attack[i] - b->resist[i];
		}
		else if (a->attack[i] != 0)
		{
			d += int(10.0 * float(a->attack[i]) / float(b->resist[i]));
		}
		i++;
	}
	
	oldb = b;
	
	if (shieldgen_nearby(b->tilex, b->tiley, player[b->color].team))
	{
		d /= 2;
	}
	
	return d;
}

int defbonus(int tx, int ty)
{
  int s = worldmap.tile[tx][ty].get_set();
  int n = worldmap.tile[tx][ty].get_number();
  switch(s)
  {
    case 0:  //Land
      switch(n)
      {
        case 2:
          return 2;
          break;
        case 3:
          return 1;
          break;
        case 4:
          return 4;
          break;
        default:
          return 0;
          break;
      }
      break;
    case 1:  //River
      return 0;
      break;
    case 2:  //Sea
      if (n == 1)  //Reef
      {
        return 1;
      }
      else
      {
        return 0;
      }
      break;
    case 3:  //Shoal
      return 0;
      break;
    case 4:  //Building
      if (worldmap.tile[tx][ty].is_HQ())
      {
        return 4;
      }
      return 3;
      break;
  }
  return 0;
}

float total_damage(_unit *a, _unit *d)
{
	int def;
	float basic;
	float fraction = a->health / 10.0;
	if (fraction < 0.2) fraction = 0.2;
	
	basic = float(basic_damage(a, d)) * fraction;
	if (basic <= 0) return 0;
	
	def = defbonus(d->tilex, d->tiley);
	if (unitstats[d->type].basetype == AIR)
	{  //air units have no terrain defense bonus
		def = 0;
	}
	return basic - ((basic * float(def)) / 10);
}

void do_damage(_unit *a, _unit *d, battleparams *p, bool is_counterattack)
{
	int i, n = 0;
	float temphealth = a->health;
	_loc l;
	pair<_loc, float> end;
	
	if (is_counterattack) n = 1;
	
	l.x = a->tilex;
	l.y = a->tiley;
	i = p->in_healths(n, l);
	if (i != -1)
	{
		a->health = p->healths[n][i].second;
	}
	
	//have to temporarily set the attacking unit's health to whatever it is
	//and then set it back to what is was originally (before the battle)
	//this is so damages are calculated correctly but the displayed unit healths
	//don't change until after the animations are done
	float total = total_damage(a, d) + rand()%10 - 5;  //add some randomness (+- 5%) to the damage;
	
	a->health = temphealth;  //...and now we set it back
	
	if (total < 0) total = 0;
	
	l.x = d->tilex;
	l.y = d->tiley;
	i = p->in_healths(n, l);
	if (i == -1)
	{
		end.first = l;
		end.second = d->health - (total / 10.0);
		p->healths[n].push_back(end);
	}
	else
	{
		p->healths[n][i].second -= (total / 10.0);
	}
	
	if (unitstats[a->type].attacksburn)
	{
		if (worldmap.tile[d->tilex][d->tiley].same_type(0, 2))
		{  //if the defender is on a forest and the attacker's attacks burn
			if (!worldmap.tile[d->tilex][d->tiley].burning)
			{
				l.x = d->tilex;
				l.y = d->tiley;
				p->treesburned.push_back(l);
			}
		}
	}
}

void radial_damage(_unit *a, int tx, int ty, battleparams *p, bool is_counterattack)
{
	int x, y, z, r = unitstats[a->type].blastradius, i;
	float total, temphealth = a->health;
	pair<_loc, float> radialhit;
	_unit *d;
	_loc l;
	int n = 0;
	
	if (is_counterattack) n = 1;
	
	l.x = a->tilex;
	l.y = a->tiley;
	i = p->in_healths(n, l);
	if (i != -1)
	{  //like in do_damage, temporarily change a->health, then change it back
		a->health = p->healths[n][i].second;
	}
	
	y = ty - r;
	if (y < 0) y = 0;
	while ((y <= ty + r) && (y < worldmap.h))
	{
		x = tx - r;
		if (x < 0) x = 0;
		while ((x <= tx + r) && (x < worldmap.l))
		{
			if ((tile_distance(x, y, tx, ty) <= r) && ((x != tx) || (y != ty)))
			{
				z = any_unit_here(x, y);
				if (z != -1)
				{
					d = &player[z / 100].unit[z % 100];
					
					if (unitstats[d->type].basetype != AIR)
					{  //air units don't get hit with splash damage
						total = total_damage(a, d) / (1 + tile_distance(tx, ty, x, y)) + rand()%10 - 5;
						if (total < 0) total = 0;
						else
						{
							radialhit.first.x = x;
							radialhit.first.y = y;
							
							i = p->in_healths(n, radialhit.first);
							if (i == -1)
							{
								radialhit.second = d->health - (total / 10.0);
								p->healths[n].push_back(radialhit);
							}
							else
							{
								p->healths[n][i].second -= (total / 10.0);
							}
						}
					}
				}
				
				if (unitstats[a->type].attacksburn)
				{
					if (worldmap.tile[x][y].same_type(0, 2))
					{  //if the tile is a forest and the attacker's attacks burn
						if (!worldmap.tile[x][y].burning)
						{
							l.x = x;
							l.y = y;
							p->treesburned.push_back(l);
						}
					}
				}
			}
			x++;
		}
		y++;
	}
	
	a->health = temphealth;
}

void get_battlestats(_unit *a, _unit *d, battleparams *p)
{
	int i;
	_loc l;
	float ahealth = a->health, dhealth = d->health, temphealth;
	
	p->aloc.x = a->tilex;
	p->aloc.y = a->tiley;
	p->dloc.x = d->tilex;
	p->dloc.y = d->tiley;
	
	p->treesburned.clear();
	p->healths[0].clear();
	p->healths[1].clear();
	p->a_attackdelay = unitstats[a->type].attackdelay;
	p->d_attackdelay = d->attackdelay;
	p->counter = false;
	
	do_damage(a, d, p, false);
	if (unitstats[a->type].blastradius > 0) radial_damage(a, d->tilex, d->tiley, p, false);
	
	l.x = a->tilex;
	l.y = a->tiley;
	i = p->in_healths(0, l);
	if (i != -1) ahealth = p->healths[0][i].second;
	
	l.x = d->tilex;
	l.y = d->tiley;
	i = p->in_healths(0, l);
	if (i != -1) dhealth = p->healths[0][i].second;
	
	if ((dhealth >= 0.01) && (ahealth >= 0.01) && (unitstats[d->type].attacktype != RANGED) && (unitstats[a->type].attacktype != RANGED))
	{  //if neither unit is a ranged-attack unit
		if (attackable_unit_here(d, a->tilex, a->tiley) != -1)
		{  //if the defender can counterattack, do so (but not if it's allied)
			p->counter = true;
			p->d_attackdelay = unitstats[d->type].attackdelay;
			
			temphealth = d->health;
			d->health = dhealth;
			do_damage(d, a, p, true);
			d->health = temphealth;
			if (unitstats[d->type].blastradius > 0) radial_damage(d, a->tilex, a->tiley, p, true);
			
			l.x = a->tilex;
			l.y = a->tiley;
			i = p->in_healths(1, l);
			
			l.x = d->tilex;
			l.y = d->tiley;
			i = p->in_healths(1, l);
		}
	}
}

void broadcast_battledata(battleparams *p)
{
	unsigned int i;
	DyBytes data;
	
	data.clear();
	
	data.addByte(p->aloc.x);
	data.addByte(p->aloc.y);
	data.addByte(p->dloc.x);
	data.addByte(p->dloc.y);
	
	data.addByte(p->counter);
	data.addByte(p->a_attackdelay);
	data.addByte(p->d_attackdelay);
	
	data.addByte(p->healths[0].size());
	i = 0;
	while (i < p->healths[0].size())
	{
		data.addByte(p->healths[0][i].first.x);
		data.addByte(p->healths[0][i].first.y);
		data.appendVariable(p->healths[0][i].second);
		i++;
	}
	
	data.addByte(p->healths[1].size());
	i = 0;
	while (i < p->healths[1].size())
	{
		data.addByte(p->healths[1][i].first.x);
		data.addByte(p->healths[1][i].first.y);
		data.appendVariable(p->healths[1][i].second);
		i++;
	}
	
	data.addByte(p->treesburned.size());
	i = 0;
	while (i < p->treesburned.size())
	{
		data.addByte(p->treesburned[i].x);
		data.addByte(p->treesburned[i].y);
		i++;
	}
	
	data.compress(9);
	data.insertByte(NET_BATTLEDATA);
	broadcast_datablob(&data);
}

void get_battledata(DyBytes *data)
{
	unsigned int i, z;
	_loc l;
	pair<_loc, float> p;
	
	data->decompress();
	
	netparams.treesburned.clear();
	netparams.healths[0].clear();
	netparams.healths[1].clear();
	
	netparams.aloc.x = data->getByte(0);
	netparams.aloc.y = data->getByte(1);
	netparams.dloc.x = data->getByte(2);
	netparams.dloc.y = data->getByte(3);
	
	var.attacker = any_unit_here(netparams.aloc.x, netparams.aloc.y);
	var.defender = any_unit_here(netparams.dloc.x, netparams.dloc.y);
	
	netparams.counter = data->getByte(4);
	netparams.a_attackdelay = data->getByte(5);
	netparams.d_attackdelay = data->getByte(6);
	
	z = data->getByte(7);
	data->removeFront(8);
	i = 0;
	while (i < z)
	{
		p.first.x = data->getByte(0);
		p.first.y = data->getByte(1);
		data->extractVariable(2, p.second);
		data->removeFront(2 + sizeof(p.second));
		netparams.healths[0].push_back(p);
		i++;
	}
	
	z = data->getByte(0);
	data->removeFront(1);
	i = 0;
	while (i < z)
	{
		p.first.x = data->getByte(0);
		p.first.y = data->getByte(1);
		data->extractVariable(2, p.second);
		data->removeFront(2 + sizeof(p.second));
		netparams.healths[1].push_back(p);
		i++;
	}
	
	z = data->getByte(0);
	data->removeFront(1);
	i = 0;
	while (i < z)
	{
		l.x = data->getByte(0);
		l.y = data->getByte(1);
		data->removeFront(2);
		netparams.treesburned.push_back(l);
		i++;
	}
}
