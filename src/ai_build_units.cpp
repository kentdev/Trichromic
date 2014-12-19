#include "empire.h"

extern int unit_price(int type, int army);

void refresh_factory_list(vector<_tile *> *factories);
bool enough_cash(int plyr);
bool build_best_unit(_tile *factory);
int closest_enemy_distance(int plyr, int x, int y, int depth);
float get_ratio(int weight, int limit, int num);
_loc empty_build_space(int x, int y, int movetype);
vector<_tile *> factories_nearest_enemy(int plyr);
vector<_tile *> sort_factories(vector<pair<_loc, int> > factorypairs);
vector<_tile *> without_factory_types(vector<_tile *> *factories, int excludetype);

void make_new_units(int plyr)
{
	vector<_tile *> builders, filtered;
	
	if (player[plyr].units_in_play() >= 50) return;
	if (!enough_cash(plyr)) return;
	
	builders = factories_nearest_enemy(plyr);
	if (builders.empty()) return;
	
	while (1)
	{
		if (!build_best_unit(builders[0]))
		{
			filtered = without_factory_types(&builders, builders[0]->building_type());
			if (filtered.empty()) return;
			if (!build_best_unit(filtered[0]))
			{
				filtered = without_factory_types(&filtered, filtered[0]->building_type());
				if (filtered.empty()) return;
				if (!build_best_unit(filtered[0]))
				{
					return;
				}
			}
		}
		
		refresh_factory_list(&builders);
		if (builders.empty()) return;
		if (!enough_cash(plyr)) return;
		if (player[plyr].units_in_play() >= 50) return;
		if (key[KEY_B]) return;  //just in case of an infinite loop
	}
}

bool build_best_unit(_tile *t)
{
	bool add = false;
	int i, j, p = t->get_owner();
	int num[MAX_UNITS], best = -1;
	float val, minval;
	_loc buildloc;
	vector<int> factoryunits;
	vector<int>::iterator it, index;
	DyBytes data;
	
	i = 0;
	while (i < NUM_UNITS)
	{
		add = false;
		if (unitstats[i].isarmy[player[p].number])
		{
			switch (t->building_type())
			{
				case BASE:
					if (unitstats[i].basetype == LAND)
					{
						add = true;
					}
					break;
				case AIRPORT:
					if (unitstats[i].basetype == AIR)
					{
						add = true;
					}
					break;
				case PORT:
					if (unitstats[i].basetype == SEA)
					{
						add = true;
					}
					break;
			}
		}
		
		if (add)
		{
			factoryunits.push_back(i);
			j = 0;
			num[i] = 0;
			while (j < 50)
			{
				if (player[p].unit[j].exists == 1)
				{
					if (player[p].unit[j].type == i)
					{
						num[i]++;
					}
				}
				j++;
			}
		}
		i++;
	}
	
	minval = 10000;
	it = factoryunits.begin();
	index = it;
	while (it != factoryunits.end())
	{
		val = get_ratio(theai.weight[(*it)], theai.limit[(*it)], num[(*it)]);
		
		if (unitstats[(*it)].cancapture)
		{  //if the unit can capture
			val /= 2;
			if (num[(*it)] == 0)
			{  //and there aren't any units like this deployed
				val /= 4;  //make it more important (lower values = more build priority)
			}
		}
		if (unit_price((*it), player[p].number) > player[p].cash)
		{  //if the unit is out of the player's price range, lower its priority
			val *= 2;
		}
		if (empty_build_space(t->x, t->y, unitstats[(*it)].movetype).x == -1)
		{  //if this type of unit can't be built here
			val = 10000;
		}
		
		if (val < minval)
		{
			index = it;
			minval = val;
		}
		it++;
	}
	best = (*index);
	
	if (best != -1)
	{
		if (unit_price(best, player[p].number) <= player[p].cash)
		{
			buildloc = empty_build_space(t->x, t->y, unitstats[best].movetype);
			if (buildloc.x != -1)
			{
				i = player[p].create_unit(best, buildloc.x, buildloc.y, p);
				if (i != -1)
				{
					player[p].cash -= player[p].unit[i].price;
					player[p].stats.produced++;
					player[p].stats.spent += player[p].unit[i].price;
					
					empdebug("Created new unit:");
					empdebug(unitstats[best].name);
					empdebug("at location:");
					empdebug(buildloc.x);
					empdebug(buildloc.y);
					
					if (global.netgame)
					{
						data.clear();
						data.addByte(NET_PLAYERCASH);
						data.addByte(p);
						data.appendVariable(player[p].cash);
						broadcast_datablob(&data);
						
						data.clear();
						data.addByte(NET_NEWUNIT);
						data.addByte(p);
						data.addByte(best);
						data.addByte(buildloc.x);
						data.addByte(buildloc.y);
						broadcast_datablob(&data);
					}
					
					return true;
				}
			}
		}
	}
	
	return false;
}

bool enough_cash(int plyr)
{  //can the player, theoretically, build a unit?
	int armynum = player[plyr].number;
	int i = 0;
	while (i < NUM_UNITS)
	{
		if (unit_price(i, armynum) <= player[plyr].cash)
		{
			return true;
		}
		i++;
	}
	return false;
}

void refresh_factory_list(vector<_tile *> *fac)
{  //remove factories that no longer have clear build spaces
	int movetype;
	vector<_tile *>::iterator it = fac->begin();
	while (it != fac->end())
	{
		switch((*it)->building_type())
		{
			case BASE:
				movetype = M_MECH;
				break;
			case PORT:
				movetype = M_SEA;
				break;
			case AIRPORT:
				movetype = M_AIR;
				break;
		}
		if (((any_unit_here((*it)->x, (*it)->y) != -1) || (empty_build_space((*it)->x, (*it)->y, movetype).x == -1)))
		{
			it = fac->erase(it);
		}
		else
		{
			it++;
		}
	}
}

vector<_tile *> factories_nearest_enemy(int plyr)
{
	const int depth = 10;  //only searches the closest 20x20 tiles (10 up, 10 down, 10 left, 10 right)
	int i, j, movetype;
	pair<_loc, int> p;
	vector<pair<_loc, int> > factories;
	_loc l;
	_tile *t;
	
	j = 0;
	while (j < worldmap.h)
	{
		i = 0;
		while (i < worldmap.l)
		{
			t = &worldmap.tile[i][j];
			if (t->is_unit_producing())
			{
				if (t->owned_by(plyr))
				{
					switch(t->building_type())
					{
						case BASE:
							movetype = M_MECH;
							break;
						case PORT:
							movetype = M_SEA;
							break;
						case AIRPORT:
							movetype = M_AIR;
							break;
					}
					if ((any_unit_here(i, j) == -1) && (empty_build_space(i, j, movetype).x != -1))
					{  //any_unit_here: can't build, even on surrounding spaces, if the factory itself is covered
						l.x = i;
						l.y = j;
						p.first = l;
						p.second = closest_enemy_distance(plyr, i, j, depth);
						factories.push_back(p);
					}
				}
			}
			i++;
		}
		j++;
	}
	
	return sort_factories(factories);
}

vector<_tile *> sort_factories(vector<pair<_loc, int> > factorypairs)
{
	int i, min;
	_tile *closest;
	vector<_tile *> result;
	vector<pair<_loc, int> >::iterator it, index;
	
	while (!factorypairs.empty())
	{
		min = 100;
		closest = NULL;
		i = 0;
		it = factorypairs.begin();
		index = it;
		while (it != factorypairs.end())
		{
			if ((*it).second < min)
			{
				closest = &worldmap.tile[(*it).first.x][(*it).first.y];
				min = (*it).second;
				index = it;
			}
			it++;
		}
		
		if (closest != NULL)
		{
			result.push_back(closest);
			factorypairs.erase(index);
		}
		else
		{  //huh?  no closest?  get_step returned something > 100?
			factorypairs.erase(factorypairs.begin());
		}
	}
	return result;
}

int closest_enemy_distance(int plyr, int x, int y, int depth)
{
	int closest = 99, i, j, z;
	
	j = y - depth;
	if (j < 0) j = 0;
	while ((j <= y + depth) && (j < worldmap.h))
	{
		i = x - depth;
		if (i < 0) i = 0;
		while ((i <= x + depth) && (i < worldmap.l))
		{
			z = any_unit_here(i, j);
			if (z != -1)
			{
				if (player[z / 100].team != player[plyr].team)
				{
					if (tile_distance(i, j, x, y) < closest)
					{
						closest = tile_distance(i, j, x, y);
					}
				}
			}
			i++;
		}
		j++;
	}
	
	return closest;
}

_loc empty_build_space(int x, int y, int movetype)
{  //search the surrounding area (9x9) for empty spaces
	_loc l;
	int i, j = y - 1;
	l.x = -1; l.y = -1;
	if (j < 0) j = 0;
	while ((j <= y + 1) && (j < worldmap.h))
	{
		i = x - 1;
		if (i < 0) i = 0;
		while ((i <= x + 1) && (i < worldmap.l))
		{
			if ((i != x) || (j != y))
			{  //don't build on the factory itself unless all others are filled
				if ((moves_needed(movetype, i, j, 0) < 99) && (any_unit_here(i, j) == -1))
				{
					l.x = i;
					l.y = j;
					return l;
				}
			}
			i++;
		}
		j++;
	}
	
	if (any_unit_here(x, y) == -1)
	{
		l.x = x;
		l.y = y;
	}
	return l;
}

vector<_tile *> without_factory_types(vector<_tile *> *factories, int excludetype)
{
	unsigned int i = 0;
	vector<_tile *> result;
	
	while (i < factories->size())
	{
		if ((*factories)[i]->building_type() != excludetype)
		{
			result.push_back((*factories)[i]);
		}
		i++;
	}
	return result;
}

float get_ratio(int weight, int limit, int num)
{
  float r = 10000;
  if ((weight > 0) && (limit > num))
  {  //the lower the ratio, the higher the build priority
    r = (float(num) + 1) * 1.5 / float(weight);
  }  //if the weight is 0 or the limit is reached, the ratio stays at 10000;
  return r;
}
