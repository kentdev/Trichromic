#include "empire.h"

_action action[3];

#define AI_UNIT_BEGIN        0
#define AI_UNIT_SCROLL       1
#define AI_UNIT_MAKE_PATHMAP 2
#define AI_UNIT_THINK        3
#define AI_UNIT_PAUSE        4
#define AI_UNIT_ACT          5

bool ai_unit_animating;
bool HQ_threat;
_action ai_unit_animation;
int in_battle, atk, def;
vector<_unit*> lander;
vector<_unit*> need_transport;
vector<_loc> base;
vector<_aitarget> targets;
_loc capital;  //location of the AI HQ - gets set at the beginning of each AI turn
vector<_aitarget> list_targets(_unit *u);

int do_ai(int plyr)
{
  return do_ai(plyr, false);
}

int do_ai(int plyr, bool reset_everything)
{
  static bool first = true;
  static list<_unit *> active_units;
  static _actionset a;
  static int draw_stats, last_statpos;
  static int initial_delay, delay, aistatus, anum;
  int t1, t2, t3, t4, i;
  _tile *thetile, *t;
  _loc temploc;
  _unit *u;
	unitprops *p;
	
  if (reset_everything)
  {  //if the user returned to the main menu in the middle of an AI turn
    first = 1;
    return 1;
  }
	
	if (first)
	{
		lander.clear();
		need_transport.clear();
		player[plyr].sort_units();
		active_units.clear();
		i = 0;
		while (i < 50)
		{
			if (player[plyr].unit[i].exists == 1)
			{
				p = &unitstats[player[plyr].unit[i].type];
				if ((p->movetype == M_LANDER) && (p->loadcapacity > 0))
				{  //this creates a vector of pointers to landers, handy later on
					lander.push_back(&player[plyr].unit[i]);
				}
				
				active_units.push_back(&player[plyr].unit[i]);
			}
			i++;
		}
		
		t2 = 0;
		while (t2 < worldmap.h)
		{
			t1 = 0;
			while (t1 < worldmap.l)
			{
				thetile = &worldmap.tile[t1][t2];
				if (thetile->owned_by(plyr))
				{
					if (thetile->is_unit_producing())
					{  //if it belongs to the player and produces units
						temploc.x = t1;
						temploc.y = t2;
						base.push_back(temploc);  //create a vector of base locations
					}
					else if (thetile->is_HQ())
					{  //mark the location of the player's HQ
						capital.x = t1;
						capital.y = t2;
					}
				}
				t1++;
			}
			t2++;
		}
		
		HQ_threat = enemy_threat(plyr);
		
		i = 0;
		first = false;
		in_battle = 0;
		atk = -1;
		def = -1;
		draw_stats = 0;
		last_statpos = -1;
		initial_delay = 0;
		delay = 0;
		aistatus = AI_UNIT_BEGIN;
		counter = 0;
	}
	
	if (initial_delay < 15)
	{  //have a little delay before the units start doing stuff
		initial_delay++;
	}
	else if (in_battle == 0)
	{
		if (check_explodes())
		{  //if check_explodes found an explosion to do
			return 30;  //get out of the AI function early and indicate an explosion
		}
		
		if (!active_units.empty())
		{
			u = *active_units.begin();
			if (u->ready == 0)
			{  //if it isn't ready, why is it in active_units anyway?
				active_units.pop_front();
				delay = 0;
				aistatus = AI_UNIT_BEGIN;
			}
			else
			{
				switch (aistatus)
				{
					case AI_UNIT_BEGIN:
						if (delay < 15)
						{  //have a little delay between units as well
							delay++;
						}
						else
						{
							aistatus = AI_UNIT_SCROLL;
						}
						break;
					case AI_UNIT_SCROLL:
						if (scroll_to_location(u->tilex, u->tiley) == 1)
						{  //smooth scroll to the unit, which returns 1 when finished
							aistatus = AI_UNIT_MAKE_PATHMAP;
							return 20;  //returning a 2 in the tens column prevents the player from bringing up a menu
						}
						break;
					case AI_UNIT_MAKE_PATHMAP:
						worldmap.create_pathmap(plyr, unitstats[u->type].movetype, u->tilex, u->tiley);
						show_movable_tiles(u);
						counter = 0;  //reset the counter, since creating the pathmap might take a while
						aistatus = AI_UNIT_THINK;
						break;
					case AI_UNIT_THINK:
						show_movable_tiles(u);
						t1 = (u->tilex - worldmap.scroll_x) * MAP_TILE_SIZE;
						t2 = (u->tiley - worldmap.scroll_y) * MAP_TILE_SIZE;
						switch(u->color)
						{
							case 0:
								t3 = ORANGE;
								break;
							case 1:
								t3 = BLUE;
								break;
							case 2:
								t3 = GREEN;
								break;
							case 3:
								t3 = YELLOW;
								break;
							default:
								t3 = BLACK;
								break;
						}  //draw a rectangle around the unit, colored the same as the unit's team
						buffer_rect(t1 + 1, t2 + 1, t1 + MAP_TILE_SIZE - 1, t2 + MAP_TILE_SIZE - 1, t3);
						targets.resize(0);
						a = ponder_action(u);
						delay = 0;
						aistatus = AI_UNIT_PAUSE;
						counter = 0;  //this may have taken a while, but nothing happened on
						break;        //screen, so just set the counter like it took no time
					case AI_UNIT_PAUSE:
						if (delay < 25)  //the unit's range is displayed for 25 logic loops (.5 seconds)
						{  //in other words, it shows the range while thinking, then pauses before acting
							show_movable_tiles(u);
							u->draw(false);
							t1 = (u->tilex - worldmap.scroll_x) * MAP_TILE_SIZE;
							t2 = (u->tiley - worldmap.scroll_y) * MAP_TILE_SIZE;
							switch(u->color)
							{
								case 0:
									t3 = ORANGE;
									break;
								case 1:
									t3 = BLUE;
									break;
								case 2:
									t3 = GREEN;
									break;
								case 3:
									t3 = YELLOW;
									break;
								default:
									t3 = BLACK;
									break;
							}  //draw a rectangle around the unit, colored the same as the unit's team
							buffer_rect(t1 + 1, t2 + 1, t1 + MAP_TILE_SIZE - 1, t2 + MAP_TILE_SIZE - 1, t3);
							delay++;
						}
						if (delay == 25)
						{
							aistatus = AI_UNIT_ACT;
							anum = 0;
							if (u->tilex - worldmap.scroll_x < 4)
							{
								draw_stats = 2;
							}
							else
							{
								draw_stats = 1;
							}
						}
						break;
					case AI_UNIT_ACT:
						if ((!u->is_moving()) && (!ai_unit_animating))
						{  //if the unit has is finished with an action or starting the first
							
							t = &worldmap.tile[u->tilex][u->tiley];
							if (unitstats[u->type].cancapture)
							{
								if ((t->is_building()) && ((t->is_unit_producing()) || (t->is_HQ())))
								{  //if it's a building that gets permanently captured
									if (!t->owned_by(u->color))
									{  //if it's not owned by the player
										capture_building(u);
									}  // (this means you can capture teammates' buildings)
								}
							}
							
							switch(anum)
							{
								case 0:
								case 1:
								case 2:
									act(u, a.action[anum]);
									anum++;
									if (u->exists == 0)
									{  //if a battle or being loaded into a transport causes it to disappear
										u->ready = 0;
										active_units.pop_front();
										aistatus = AI_UNIT_BEGIN;
										delay = 0;
									}
									break;
								case 3:  //if the final action has just been done, this unit is finished
									u->ready = 0;
									active_units.pop_front();
									aistatus = AI_UNIT_BEGIN;
									delay = 0;
									update_cities();
									break;
							}
						}
						else if (ai_unit_animating)
						{
							switch(ai_unit_animation.type)
							{
								case action_teleport:
									ai_teleport(u, ai_unit_animation.loc);
									break;
							}
						}
						else
						{
							u->do_movement();
							u->draw();
						}
						break;
				}
			}  //end of "if the unit is ready"
		}
		else  //if active_units is empty
		{
			if (delay == 0)
			{
				empdebug("AI is deciding what units to build...");
				
				make_new_units(plyr);
				
				empdebug("AI has finished building units.");
				counter = 0;  //this also tends to take a while, but has no on-screen effect
				delay++;
			}
			else if (delay < 35)
			{  //pause for a little over half a second after the units are made
				delay++;
			}
			else
			{
				first = true;
				delay = 0;
				empdebug("Finished AI turn");
				return 1;  //indicate that it is done
			}
		}
	}
	else  //if there is a battle animation going on
	{
		t1 = atk / 100;
		t2 = atk % 100;
		t3 = def / 100;
		t4 = def % 100;
		in_battle = do_battle(&player[t1].unit[t2], &player[t3].unit[t4]);
		return 20;
	}
	return 0;  //indicate that it is not finished
}

_actionset ponder_action(_unit *u)
{
	_actionset aset;
	action[0].type = action_none;
	action[0].loc.x = -1;
	action[0].loc.y = -1;
	action[1].type = action_none;
	action[1].loc.x = -1;
	action[1].loc.y = -1;
	action[2].type = action_none;  //third action only used for unloading landers
	action[2].loc.x = -1;
	action[2].loc.y = -1;
	targets = list_targets(u);
	unitprops *p = &unitstats[u->type];
	
	empdebug("AI thought:");
	empdebug(unitstats[u->type].name);
	empdebug("Current location:");
	empdebug(u->tilex);
	empdebug(u->tiley);
	
	if (HQ_threat)
	{
		empdebug("Check HQ park...");
		check_HQ_park(u);
		empdebug("Check capturing...");
		check_capturing(u);
	}
	else
	{
		empdebug("Check capturing...");
		check_capturing(u);
		empdebug("Check HQ park...");
		check_HQ_park(u);
	}
	
	empdebug("Check attackable units...");
	check_attackable_units(u);
	empdebug("Check low health...");
	check_low_health(u);

  if (action[0].type == action_none)
  {  //if it didn't capture, doesn't need to get fixed, and can't attack this turn
    if (p->cancapture)
    {  //if it can capture things, head for the closest capturable building
    	empdebug("Check closest capturable building...");
      check_closest_capturable(u);
    }
    if (p->attacktype != -1)
    {
      if (action[0].type == action_none)
      {
      	empdebug("Check closest attackable unit...");
        check_closest_attackable(u);
      }
    }
    else if ((p->basetype == LAND) && (p->loadtype == 1))
    {
    	empdebug("Check APC options...");
      check_APC_options(u);
    }
    else if ((p->basetype == AIR) && (p->loadtype == 1))
    {
    	empdebug("Check T_Copter options...");
      check_tcopter_options(u);
    }
    else if ((p->basetype == SEA) && (p->loadtype == 2))
    {  //if the unit is a lander
    	empdebug("Check Lander options...");
      check_lander_options(u);
    }
  }
  empdebug("Check available teleports...");
	check_teleports(u);  //if the unit is moving somewhere, see if it can get there faster by teleporting
											 //if the unit has nowhere to go, see if it can get somewhere by teleporting
  if ((p->movetype == M_INFANTRY) || (p->movetype == M_MECH))
  {
  	empdebug("Check for a convenient APC...");
    check_convenient_APC(u);
    if (action[0].type == action_none)
    {
    	empdebug("Check for an available T_Copter...");
      check_available_copter(u);
    }
  }
  if (action[0].type == action_none)
  {  //if there are no enemies to attack or teleport tiles handy, cover an enemy base
  	empdebug("Check if there's an enemy base to cover...");
    check_enemy_bases(u);
  }
  if ((p->basetype == LAND) && (action[0].type == action_none))
  {  //check for a lander if the unit can't do anything and could be moved in a lander
  	empdebug("Check for an available Lander...");
    check_available_lander(u);
  }
  if (action[0].type == action_none)
  {  //make sure it isn't preventing other units from being built
  	empdebug("Check to make sure it's not blocking a base...");
		check_blocking_base(u);
  }
  if (p->loadtype == 0)
  {  //if it isn't a transport
    if (p->basetype == LAND)
    {
      if (action[0].type == action_none)
      {  //if it still isn't doing anything
        need_transport.push_back(u);
      }
    }
  }
  aset.action[0] = action[0];
  aset.action[1] = action[1];
  aset.action[2] = action[2];
	
	empdebug("Done thinking for this unit");
	empdebug("Results:");
	empdebug("action[0]:");
	empdebug(action[0].type);
	empdebug(action[0].loc.x);
	empdebug(action[0].loc.y);
	empdebug("action[1]:");
	empdebug(action[1].type);
	empdebug(action[1].loc.x);
	empdebug(action[1].loc.y);
	empdebug("action[2]:");
	empdebug(action[2].type);
	empdebug(action[2].loc.x);
	empdebug(action[2].loc.y);
	
	return aset;
}

void act(_unit *u, _action a)
{
	int i;
	int q;
	char dbg[200];
	DyBytes data;
	vector<int> movelist;
	
	switch(a.type)
	{
		case action_move:
			if ((u->tilex != a.loc.x) || (u->tiley != a.loc.y))
			{  //a crash safeguard: units don't move if they're already at the destination
				if ((a.loc.x >= 0) && (a.loc.y >= 0))
				{
					empdebug("AI unit moving");
					empdebug(a.loc.x);
					empdebug(a.loc.y);
					set_best_path(u, a.loc.x, a.loc.y);
					empdebug("AI path set");
					play_moving_sound(u->type);
					u->do_movement();
				}
				else
				{  //move should never be called for location with a coord < 0, so log this error
					sprintf(dbg, "Error: A %s unit at (%d, %d) tried to move to point (%d, %d)", unitstats[u->type].name.c_str(), u->tilex, u->tiley, a.loc.x, a.loc.y);
					empdebug(dbg);
				}
			}
			
			if (global.netgame)
			{
				movelist = u->get_movelist();
				data.clear();
				data.addByte(NET_MOVEUNIT);
				data.addByte(u->tilex);
				data.addByte(u->tiley);
				data.addByte(movelist.size());
				i = 0;
				while ((unsigned)i < movelist.size())
				{
					data.addByte(movelist[i]);
					i++;
				}
				broadcast_datablob(&data);
			}
      break;
    case action_attack:
      in_battle = 1;
      def = any_unit_here(a.loc.x, a.loc.y);
      atk = (u->color * 100) + u->number;
      break;
    case action_load:
      q = unit_here_thorough(u->color, u->tilex, u->tiley, u->number);
      player[u->color].unit[q].load_unit(u);
      worldmap.tile[u->tilex][u->tiley].set_unit(u->color, q);
      player[u->color].unit[u->number].exists = 0;
      break;
    case action_unload1:
      u->load[0].loaded = 0;
      i = player[u->color].create_unit(u->load[0].type, a.loc.x, a.loc.y, u->color);
      player[u->color].unit[i].health = u->load[0].health;
      q = 0;
      while (q < 2)
      {
	      if (u->load[0].subload[q].loaded == 1)
	      {
	        player[u->color].unit[i].load[q].loaded = 1;
	        player[u->color].unit[i].load[q].type = u->load[0].subload[q].type;
	        player[u->color].unit[i].load[q].health = u->load[0].subload[q].health;
	        u->load[0].subload[q].loaded = 0;
	      }
	      q++;
      }
			
			if (global.netgame)
			{
				data.clear();
				data.addByte(NET_UNLOAD);
				data.addByte(u->tilex);
				data.addByte(u->tiley);
				data.addByte(0);
				data.addByte(a.loc.x);
				data.addByte(a.loc.y);
				broadcast_datablob(&data);
			}
      break;
    case action_unload2:
      u->load[1].loaded = 0;
      i = player[u->color].create_unit(u->load[1].type, a.loc.x, a.loc.y, u->color);
      player[u->color].unit[i].health = u->load[1].health;
      q = 0;
      while (q < 2)
      {
	      if (u->load[1].subload[q].loaded == 1)
	      {
	        player[u->color].unit[i].load[q].loaded = 1;
	        player[u->color].unit[i].load[q].type = u->load[1].subload[q].type;
	        player[u->color].unit[i].load[q].health = u->load[1].subload[q].health;
	        u->load[1].subload[q].loaded = 0;
	      }
	      q++;
      }
			
			if (global.netgame)
			{
				data.clear();
				data.addByte(NET_UNLOAD);
				data.addByte(u->tilex);
				data.addByte(u->tiley);
				data.addByte(0);
				data.addByte(a.loc.x);
				data.addByte(a.loc.y);
				broadcast_datablob(&data);
			}
      break;
    case action_standby:
    case action_none:
      break;
    case action_merge:
      merge_units(u, u->tilex, u->tiley);
      break;
		case action_teleport:
			ai_unit_animating = true;
			ai_unit_animation.type = action_teleport;
			ai_unit_animation.loc = a.loc;
			ai_unit_animation.destination = a.destination;
			
			if (global.netgame)
			{
				data.clear();
				data.addByte(NET_TELEPORT);
				data.addByte(u->color);
				data.addByte(u->number);
				data.addByte(a.destination.x);
				data.addByte(a.destination.y);
				broadcast_datablob(&data);
			}
			break;
  }
}

void set_best_path(_unit *u, int tx, int ty)
{
  int i, q;
  vector<int> list;
  list.reserve(10);
  vector<_aipath> t = best_aipath(u->color, unitstats[u->type].movetype, u->tilex, u->tiley, tx, ty);
  q = t.size();
  i = 0;
  while (i < q)
  {
    list.push_back(t[i].dir);
    i++;
  }
  u->set_moves(list);
}

int any_unit_here(int tx, int ty)
{
  if (worldmap.in_bounds(tx, ty))
  {
    return worldmap.tile[tx][ty].unit_here();
  }
  return -1;
}

int next_to_enemy(int plyr, int tx, int ty)
{
  int z;
  z = any_unit_here(tx + 1, ty);
  if (z != -1)
  {
    if (z / 100 != plyr)
    {
      return 1;
    }
  }
  z = any_unit_here(tx - 1, ty);
  if (z != -1)
  {
    if (z / 100 != plyr)
    {
      return 1;
    }
  }
  z = any_unit_here(tx, ty + 1);
  if (z != -1)
  {
    if (z / 100 != plyr)
    {
      return 1;
    }
  }
  z = any_unit_here(tx, ty - 1);
  if (z != -1)
  {
    if (z / 100 != plyr)
    {
      return 1;
    }
  }
  return 0;   
}

int intmax(int a, int b)
{
  if (a > b)
  {
    return a;
  }
  return b;
}

int intmin(int a, int b)
{
	if (a > b)
	{
		return b;
	}
	return a;
}

bool enemy_threat(int plyr)
{  //checks to see whether enemy units are near the HQ
	int i = 0, j;
	_unit *u;
	
	worldmap.backup_pathmap();
	while (i < numplayers)
	{
		if ((i != plyr) && (player[i].playing == 1))
		{
			j = 0;
			while (j < 50)
			{
				u = &player[i].unit[j];
				if (u->exists == 1)
				{
					worldmap.create_limited_pathmap(i, unitstats[u->type].movetype, u->tilex, u->tiley, u->move + 2);
					if (worldmap.tile[capital.x][capital.y].get_step() < 99)
					{
						worldmap.restore_pathmap();
						return true;
					}
				}
				j++;
			}
		}
		i++;
	}
	worldmap.restore_pathmap();
	
	return false;
}

bool enemy_within_radius(int plyr, int center_x, int center_y, int r)
{
 int x, y, t;
  y = center_y - r;
  while (y <= center_y + r)
  {
    x = center_x - r;
    while (x <= center_x + r)
    {
      if ((worldmap.in_bounds(x, y)) && (tile_distance(x, y, center_x, center_y) <= r))
      {
        t = any_unit_here(x, y);
        if (t != -1)
        {
          if (player[t / 100].team != player[plyr].team)
          {  //doesn't check whether the enemy unit can get to the base
            return true;  //only if it's within R tiles
          }  //yeah, I know it needs work
        }
      }
      x++;
    }
    y++;
  }
  return false;
}

void ai_teleport(_unit *u, _loc dest)
{
  static bool first = true;
  static int time;
  int t1, t2;
  
  if (first)
  {
    time = 0;
    first = false;
  }
  
  if (time == 0)
  {
    if (scroll_to_location(u->tilex, u->tiley) == 0)
    {
      time--;
    }
  }
  else if (time == 1)
  {
    t1 = u->tilex * MAP_TILE_SIZE;
    t2 = u->tiley * MAP_TILE_SIZE;
    create_particle(t1 + MAP_TILE_SIZE / 2, t2 + MAP_TILE_SIZE / 2, P_SHOCKWAVE);
    create_particle(t1, t2, P_TELEPORT);
    play_sound(S_TELEPORT);
  }
  else if (time == 9)
  {
    u->exists = 0;
  }
  else if (time == 25)
  {
    if (scroll_to_location(dest.x, dest.y) == 0)
    {
      time--;
    }
  }
  else if (time == 26)
  {
    u->set_tiles(dest.x, dest.y);
    create_particle(dest.x * MAP_TILE_SIZE + MAP_TILE_SIZE / 2, dest.y * MAP_TILE_SIZE + MAP_TILE_SIZE / 2, P_SHOCKWAVE);
    create_particle(dest.x * MAP_TILE_SIZE, dest.y * MAP_TILE_SIZE, P_TELEPORT);
    play_sound(S_TELEPORT);
  }
  else if (time == 34)
  {
    u->exists = 1;
  }
  else if (time > 50)
  {
    first = true;
    ai_unit_animating = false;
  }
  time++;
}
