#include "empire.h"
#include "logic.h"

extern bool ai_unit_animating;

bool translucent_units;

int pturn;
int mousetype;
vector<int> movelist;
vector<_unitexplosion> boom;

int logic()
{
	static bool attempting_reconnect = false;
	static int t, old_logicstate;
	bool sidebar_clickable;
	bool drawmouse = false;
	char c[100];
	int temp1, i, j;
	DyBytes data;
		
	if (global.very_first_turn == 1)
	{
		old_logicstate = NOTHING_HAPPENING;
    var.logicstate = NOTHING_HAPPENING;
    var.first = true;
    global.very_first_turn = 0;
		attempting_reconnect = false;
	}
	if (var.first)
	{  //if it's a new turn
		empdebug("New turn");
		init_logic();
		if (!global.netgame)
		{
			debug_savegame();  //save the game to debug later if the game crashes
		}
		var.first = false;
	}
	
	i = 0;
	while (i < 4)
	{
		if (player[i].playing == 1)
		{
			j = 0;
			while (j < 50)
			{
				if (player[i].unit[j].exists == 1)
				{
					if ((player[i].unit[j].tilex < 0) || (player[i].unit[j].tiley < 0) || (player[i].unit[j].tilex > worldmap.l) || (player[i].unit[j].tiley > worldmap.h))
					{
						sprintf(c, "HORRIBLE ERROR: Player %d, unit %d (%s) is somehow at location (%d, %d)!", i, j, unitstats[player[i].unit[j].type].name.c_str(), player[i].unit[j].tilex, player[i].unit[j].tiley);
						empdebug(c);
						exit(0);
					}
				}
				j++;
			}
		}
		i++;
	}
	
	if (old_logicstate != var.logicstate)
	{
		old_logicstate = var.logicstate;
		update_cities();
	}
	
	if ((var.logicstate == NOTHING_HAPPENING) || (var.logicstate == NETPLAYER_TURN) || ((var.logicstate == AI_TURN) && (!ai_unit_animating)))
  {  //if nothing's going on, or it's a normal AI turn
    temp1 = 0;
    while (temp1 < 4)
    {  //check each player to see if they are not beaten
      if (player[temp1].playing == 1)
      {
        if (player[temp1].defeated(temp1))
        {  //if they are defeated, but still on the map
          player[temp1].playing = 0;
          transform_buildings(temp1, -1);
          if (temp1 == pturn)
          {  //if the player was defeated on his own turn
            pturn++;  //it is the next person's turn
            while (player[pturn].playing != 1)
            {
              pturn++;
              if (pturn >= 4) pturn = 0;
            }
          }
          var.logicstate = PLAYER_DEFEATED;
          var.theplayer = temp1;
					destroy_theplayers_units();
        }
      }
      temp1++;
    }
  }
  if (var.logicstate == NOTHING_HAPPENING)
  {
    if (one_team_left())
    {
      var.logicstate = ONE_TEAM_LEFT;
      var.windelay = 40;
    }
  }

	if ((var.logicstate != IN_OPTIONS_MENU) && (var.logicstate != IN_HELP_SCREEN))
	{
		worldmap.draw_tiles();
		
		if (attempting_reconnect)
		{
			do_sidebar(pturn, NULL, true);
			
			temp1 = reconnect();
			draw_mouse();
			if (temp1 == 0)
			{
				return LOGIC;
			}
			else if (temp1 == -1)
			{
				attempting_reconnect = false;
				global.very_first_turn = 1;
				var.first = 1;
				play_song(title_music);
				return NETWORK;
			}
			else
			{
				attempting_reconnect = false;
			}
		}
		
		sidebar_clickable = true;
		if (var.logicstate == IN_GENERAL_MENU)
		{
			sidebar_clickable = false;
		}
		if (player[pturn].controller == HUMAN)
			do_sidebar(pturn, var.unitselected, !sidebar_clickable);
		else
			do_sidebar(pturn, NULL, !sidebar_clickable);
		
		if ((global.netgame) && (var.logicstate != BACK_TO_MAIN_MENU) && (var.logicstate != FADE_EXIT))
		{
			logic_netchat();
			
			if (net.server)
			{
				i = 0;
				while (i < net.engine->getMaxConnections())
				{
					if (!net.engine->isConnected(i))
					{
						j = 0;
						while (j < 3)
						{
							if (net.clientthread[j] == i)
							{
								empdebug("In logic loop, client");
								empdebug(j);
								empdebug("on thread");
								empdebug(net.clientthread[j]);
								empdebug("is no longer connected.");
								
								clear_clientdata(j);
								net.clientthread[j] = -1;
								sprintf(c, "(Player %d was disconnected)", j + 2);
								broadcast_chat_message(c);
								logic_netchat(c);
								
								empdebug("Removing disconnected player's stuff...");
								if (player[j + 1].playing == 1)
								{
									player[j + 1].playing = 0;
									transform_buildings(j + 1, -1);
									if (j + 1 == pturn)
									{  //if the player was disconnected on his own turn
										pturn++;  //it is the next person's turn
										while (player[pturn].playing != 1)
										{
											pturn++;
											if (pturn >= 4) pturn = 0;
										}
										
										data.clear();
										data.addByte(NET_NEWTURN);
										data.addByte(pturn);
										broadcast_datablob(&data);
									}
									var.logicstate = PLAYER_DEFEATED;
									var.theplayer = j + 1;
									destroy_theplayers_units();
								}
								empdebug("Notifying other clients of the disconnect.");
								
								data.clear();
								data.addByte(NET_PLAYERGONE);
								data.addByte(j + 1);
								broadcast_datablob(&data);
							}
							j++;
						}
					}
					i++;
				}
			}
			else  //if it's a network client
			{
				if (!net.engine->isConnected(net.thread))
				{
					attempting_reconnect = true;
					return LOGIC;
				}
			}
		}
	}

	if (gameoptions.time != -999)
	{  //if there is a time limit
		if ((var.logicstate != TIME_UP_INDICATOR) && (var.logicstate != AI_TURN) && (var.logicstate != IN_OPTIONS_MENU) && (var.logicstate != IN_HELP_SCREEN))
		{
			if (mouse_y < 240)
			{
				temp1 = 468;
			}
			else
			{
				temp1 = 0;
			}
			
			sprintf(c, "Time Left: %-1ds", var.time / 50);
			buffer_rectfill(318 - text_length(font, c) / 2, temp1, text_length(font, c) + 4, 12, DGREY, 0.75);
			buffer_textout_centre_ex(normal, c, 320, temp1, WHITE);
			if (var.time < 1)
			{
				pturn++;
				while (player[pturn].playing != 1)
				{
					pturn++;
					if (pturn >= 4) pturn = 0;
				}
				var.logicstate = TIME_UP_INDICATOR;
			}
		}
	}
	else
	{
		var.time = 0;  //if it's not being used, just keep it at 0 for simplicity
	}

  if (var.logicstate == NOTHING_HAPPENING)
  {
    if (check_explodes())
    {
      var.logicstate = EXPLODING;
    }
  }
	
	mousetype = 0;
	
	switch(var.logicstate)
	{
		case AI_TURN:
			ai();
			drawmouse = true;
			break;
		case NOTHING_HAPPENING:
			var.time--;
			nothing_happening();
			minimap.check();
			drawmouse = true;
			break;
		case IN_BATTLE:
			battle();
			break;
		case UNIT_SELECTED:
			var.time--;
			unit_selected();
			drawmouse = true;
			break;
		case UNIT_MOVING:
			unit_moving();
			drawmouse = true;
			break;
		case MECHANIC_CAPPING:
			mechanic_capping();
			drawmouse = true;
			break;
    case UNIT_UNLOADING:
      var.time--;
      unit_unloading();
      drawmouse = true;
      break;
    case UNIT_TELEPORTING:
      unit_teleporting();
      drawmouse = true;
      break;
		case TELEPORT_ANIMATION:
			teleport_animation(NULL, -1, -1);
			break;
    case SAVING_GAME:
      save_game();
      drawmouse = true;
      break;
    /*case AI_MENU:
      ai_menu();
      drawmouse = true;
      break;*/
    case IN_GENERAL_MENU:
      general_menu();
      drawmouse = true;
      break;
    case IN_UNIT_BUILD_MENU:
      var.time--;
      build_menu();
      drawmouse = true;
      break;
    case IN_OPTIONS_MENU:
      if (do_gameoptions(true) == 0)
      {
        var.logicstate = IN_GENERAL_MENU;
      }
      break;
		case IN_HELP_SCREEN:
			if (help_screen())  //help_screen() returns true when finished
			{
				var.logicstate = IN_GENERAL_MENU;
			}
			break;
    case SCROLLING:
      scroll_to_hq(pturn);
      break;
    case ONE_TEAM_LEFT:
      do_particles();
      windelay();
		  stop_music();
      break;
    case WINNER:
      var.first = 1;
      return GAMEOVER;
    case BACK_TO_MAIN_MENU:
      t = 0;
      var.logicstate = FADE_EXIT;
      break;
    case FADE_EXIT:
      if (t < 24)
      {
        buffer_rectfill(0, 0, 800, 600, BLACK, float(t * 10) / 255.0);
        t++;
      }
      else
      {
        buffer_clear_to_color(BLACK);
        player[0].reset();
        player[1].reset();
        player[2].reset();
        player[3].reset();
        var.first = 1;
        global.very_first_turn = 1;
        play_song(title_music);
        return MENU;
      }
      break;
    case TIME_UP_INDICATOR:
      time_up();
      break;
    case EXPLODING:
      if (do_explodes() == 1)
      {  //if the explosion has finished
        var.logicstate = NOTHING_HAPPENING;
      }
      break;
    case PLAYER_DEFEATED:
			destroy_theplayers_units();
			var.logicstate = NOTHING_HAPPENING;
			break;
		/*case NET_BATTLE:
			client_netbattle();
			break;*/
		case START_TURN_SYNC:
			if (sync_burning())
			{
				var.logicstate = NOTHING_HAPPENING;
			}
			break;
		case NET_WAITING_FOR_RESPONSE:
			draw_net_icon(XTILES * MAP_TILE_SIZE - 80, 0);
			netplayer();
			drawmouse = true;
			break;
	}
	
	if ((var.logicstate != IN_UNIT_BUILD_MENU) && (var.logicstate != IN_OPTIONS_MENU) && (var.logicstate != IN_HELP_SCREEN))
	{
		do_particles();
	}
	
	if (drawmouse)
	{
		draw_mouse(mousetype);
	}
	
	return LOGIC;
}

bool one_team_left()
{
  int temp1 = 0;
  int temp2 = -1;
  while (temp1 < 4)
  {  //this loop sees if there's more than one team left in the game
    if (player[temp1].playing == 1)
    {
      if (temp2 > -1)
      {
        if (player[temp1].team != temp2)
        {
          return false;  //found a different team
        }
      }
      else
      {  //the team of the first player in the game
        if (temp2 == -1)
        {
          temp2 = player[temp1].team;
        }
      }
    }
    temp1++;
  }
  return true;
}


void time_up()
{
  static bool first = true;
  static int t;
  if (first)
  {
    t = 50;
    first = false;
    play_sound(S_TANKFIRE);
  }
  buffer_textout_centre_ex(big, "TIME UP", 320, 200, WHITE);
  t--;
  if (t < 0)
  {
    var.first = true;
    var.logicstate = NOTHING_HAPPENING;
    first = true;
  }
}

/*void ai_menu()
{
  static bool first = true;
  static _button items[2];
  if (first)
  {
    items[0].init(502, 2, 135, 28, "Quit to Menu");
    items[1].init(502, 32, 135, 28, "Cancel");
    position_mouse(570, 45);
    //set_mouse_range(500, 0, 639, 61);
    first = false;
  }
  
  buffer_menuback(500, 0, 639, 63, GREY, GREY, LGREY, 0.8);
  items[0].check();
  items[1].check();
  if (items[0].clicked())
  {
    first = true;
    do_ai(pturn, true);  //this resets the static ints in do_ai()
    var.logicstate = BACK_TO_MAIN_MENU;
    //set_mouse_range(0, 0, 639, 479);
  }
  if (items[1].clicked())
  {
    first = true;
    var.logicstate = AI_TURN;
    //set_mouse_range(0, 0, 639, 479);
  }
}*/


void init_logic()
{
  int x, y, z, i, j;
  char str[15];

  var.unloadnum = 0;
	var.unitselected = NULL;
  if (!((player[pturn].controller == AI) && (player[pturn].units_in_play() > 0)))
  {  //if the player is human or an AI with no units, scroll to the HQ
    var.logicstate = SCROLLING;  //sets it to scroll to the current player's HQ
    worldmap.draw_tiles();
  }
  check_scrolling(worldmap.scroll_x, worldmap.scroll_y);  //if I don't do this, the static floats
  var.first = false;                                      //in check_scrolling aren't the right
  z = 0;                                                  //ones and bad things happen
  while (z < 4)
  {
    if (player[z].playing == 1)
    {
      if (global.loaded_game == 0)  //if the game has just been loaded from a
      {                             //file, the units should not be refreshed
        player[z].ready_units();
      }
    }
    z++;
  }
  if (gameoptions.time != -999)
  {
    var.time = gameoptions.time;
  }
  boom.resize(0);  //remove all explosions from existence
  p_effects.resize(0);  //same for particles
	update_cities();
	
	stop_music();
	play_song(ingame_music[player[pturn].number]);
	
	sprintf(str, "Player %d", pturn + 1);
	create_particle(320 - normal.Width(str) / 2, 400, P_FADETEXT, player[pturn].tintcolor, str);
	
  if (global.loaded_game == 0)  //likewise, the player does not receive more cash
  {                             //and units are not repaired when the game is loaded
    player[pturn].cash += money_producing_buildings_owned(pturn) * gameoptions.funding;
		player[pturn].stats.raised += money_producing_buildings_owned(pturn) * gameoptions.funding;
		
		y = 0;
		while (y < worldmap.h)
		{
			x = 0;
			while (x < worldmap.l)
			{
				if (worldmap.tile[x][y].owned_by(pturn))
				{
					if ((worldmap.tile[x][y].is_building(CITY)) || (worldmap.tile[x][y].is_building(HQ)))
					{
						create_particle(x * MAP_TILE_SIZE + 10,	y * MAP_TILE_SIZE, P_CASH);
						if (worldmap.tile[x][y].is_building(CITY))
						{
							worldmap.tile[x][y].turnsused++;
							if ((var.cashturns != 0) && (worldmap.tile[x][y].turnsused > var.cashturns))
							{  //oil derrick all used up
								init_deadoilderrick(x, y);
								z = worldmap.tile[x][y].unit_here();
								worldmap.tile[x][y].reset();
								worldmap.tile[x][y].x = x;
								worldmap.tile[x][y].y = y;
								if (z != -1)
								{
									worldmap.tile[x][y].set_unit(z / 100, z % 100);
								}
							}
						}
					}
				}
				x++;
			}
			y++;
		}
		z = 0;
		while (z < 50)
		{
			if (player[pturn].unit[z].exists == 1)
			{
				player[pturn].unit[z].attackdelay--;
				if (player[pturn].unit[z].attackdelay < 0) player[pturn].unit[z].attackdelay = 0;
				
				if (unitstats[player[pturn].unit[z].type].name == "Mechanic")
				{
					i = 0;
					while (i < 4)
					{
						x = player[pturn].unit[z].tilex;
						y = player[pturn].unit[z].tiley;
						if (i == 0) x++;
						if (i == 1) x--;
						if (i == 2) y++;
						if (i == 3) y--;
						
						if ((x > 0) && (x < worldmap.l) && (y > 0) && (y < worldmap.h))
						{
							j = any_unit_here(x, y);
							if ((j != -1) && (player[j / 100].team == player[pturn].team) && (player[j / 100].unit[j % 100].health < 10))
							{
								player[j / 100].unit[j % 100].health += 2;
								if (player[j / 100].unit[j % 100].health > 10)
								{
									player[j / 100].unit[j % 100].health = 10;
								}
								create_particle(x * MAP_TILE_SIZE, y * MAP_TILE_SIZE, P_REPAIR);
							}
						}
						i++;
					}
				}
			}
			z++;
		}
		if (player[pturn].cash > 1000000) player[pturn].cash = 1000000;
		repair_units(pturn);
		
		if (!global.netgame)
		{  //burning has randomness in it, so the host needs to be the one to decide
			update_burning();
		}
		else
		{  //plus, we might as well make sure everybody sees the same thing at the start of each turn
			var.logicstate = START_TURN_SYNC;
		}
	}
	if (global.loaded_game == 1)  //next turn, it won't matter whether the game
	{                             //was originally loaded from a file or not
		var.time = loaded_time;
		global.loaded_game = 0;
	}
	if (player[pturn].controller != HUMAN) position_mouse(320, 240);
}

void battle()
{
  int pa = var.attacker / 100;
  int ua = var.attacker % 100;
  int pd = var.defender / 100;
  int ud = var.defender % 100;
  int battle = do_battle(&player[pa].unit[ua], &player[pd].unit[ud]);
  if (battle == 0)  //if the battle just ended
  {
    var.logicstate = NOTHING_HAPPENING;
  }
}

void windelay()
{
  if (var.windelay <= 0)
  {
    var.logicstate = WINNER;
  }
  else
  {
  	do_explodes();
    var.windelay--;
  }
}


void save_game()
{
  if (do_savemenu() == 0)
  {  //if the savemenu has just closed
    var.logicstate = IN_GENERAL_MENU;
  }
}

void ai()
{
	DyBytes data;
  //bool can_rightclick = true;
	int temp1 = do_ai(pturn);
	int indicator = temp1 / 10;  //if the AI uses its power, it indicates that in temp1's tens column
  /*if (indicator == 2)  //signals that the AI is in a battle or scroll animation
  {
    can_rightclick = false;  //no bringing up the menu in a battle animation!
  }
  else*/ if (indicator == 3)  //signals that there's an explosion about to happen
	{
	var.logicstate = EXPLODING;
    //can_rightclick = false;
	}
	
  /*if ((can_rightclick) && (mouse_rclicked() == 1))
  {
		var.logicstate = AI_MENU;
  }*/
	
	temp1 %= 10;  //ignore the indicator information and just get a 1 or 0
	if (temp1 == 1)  //if do_ai() has indicated that it is finished
	{
		var.logicstate = NOTHING_HAPPENING;
		
		pturn++;  //switch to the next player
		while (player[pturn].playing != 1)
		{
			pturn++;
			if (pturn >= 4) pturn = 0;
		}
		
		if (global.netgame)
		{
			data.clear();
			data.addByte(NET_NEWTURN);
			data.addByte(pturn);
			broadcast_datablob(&data);
		}
		
		var.first = 1;
	}
}

void nothing_happening()
{
	static _unit *mechanicmoves = NULL;
  int t = -1, tx, ty, mx, my;
  _unit *u;
  mx = mouse_x;
  my = mouse_y;
  check_scrolling();
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
	if ((tx < worldmap.scroll_x + XTILES) && (ty < worldmap.scroll_y + YTILES))
	{  //if the mouse isn't on the sidebar or bottom bar
  	t = worldmap.tile[tx][ty].unit_here();
	}
	
	if (t != -1)
	{
		if (ty - worldmap.scroll_y < YTILES - 3)
		{
			if (tx - worldmap.scroll_x < XTILES - 4)
			{
				draw_unitstats(&player[t / 100].unit[t % 100], mx + 10, my + 10);
			}
			else
			{
				draw_unitstats(&player[t / 100].unit[t % 100], mx - 110, my + 10);
			}
		}
		else
		{
			if (tx - worldmap.scroll_x < XTILES - 4)
			{
				draw_unitstats(&player[t / 100].unit[t % 100], mx + 10, my - 60);
			}
			else
			{
				draw_unitstats(&player[t / 100].unit[t % 100], mx - 110, my - 60);
			}
		}
	}
	else
	{
		if (player[pturn].controller == HUMAN)
		{
			if (worldmap.tile[tx][ty].is_unit_producing())
			{
				if (worldmap.tile[tx][ty].owned_by(pturn))
				{
					mousetype = 1;
				}
			}
		}
	}
	
	if (player[pturn].controller == AI)
	{  //if the AI turn just started, set the logicstate accordingly
		var.logicstate = AI_TURN;
		return;
	}
	else if (global.netgame)
	{  //if it's a netplayer turn, set the logicstate to netplay
		//var.logicstate = NETPLAYER_TURN;
		netplayer();
		if ((player[pturn].controller == NET) || (player[pturn].controller == NETAI))
		{
			draw_wait_icon(XTILES * MAP_TILE_SIZE - 80, 0);
		}
	}
	
	if (translucent_units && !(mouse_b & 2)) translucent_units = false;
	if ((mechanicmoves != NULL) && !(mouse_b & 2)) mechanicmoves = NULL;
	
	if (mouse_b & 2)
	{  //draws a unit's attack range
		if (t != -1)
		{
			u = &player[t / 100].unit[t % 100];
			
			if (mechanicmoves != u)
			{  // "mechanicmoves": this originally only showed the movement range of Mechanics
				mechanicmoves = u;
				worldmap.create_limited_pathmap(u->color, unitstats[u->type].movetype, u->tilex, u->tiley, u->move);
			}
			
			show_movable_tiles(u, 0.4);
			if ((unitstats[u->type].attacktype != -1) || (unitstats[u->type].name == "Mechanic"))
			{
				draw_attackrange(u);
			}
			
			if (unitstats[u->type].name == "Shield Generator")
			{
				draw_shieldrange(u);
			}
			translucent_units = false;
		}
		else
		{
			translucent_units = true;
		}
	}
  else if ((mouse_clicked() == 1) && (player[pturn].controller == HUMAN))
  {
    if ((t != -1) && (t / 100 == pturn))
    {  //if one of the player's units is here
      u = &player[pturn].unit[t % 100];
      if (u->ready == 1)
      {  //if a unit is here and ready, select it
        var.unitselected = u;
        var.oldux = u->tilex;
        var.olduy = u->tiley;
        worldmap.create_limited_pathmap(pturn, unitstats[u->type].movetype, u->tilex, u->tiley, u->move + 1);
        var.logicstate = UNIT_SELECTED;
      }
    }
    else if (t == -1)
    {  //if there is no unit here, check for a building that can make units
      if (worldmap.tile[tx][ty].is_unit_producing())
      {
        if (worldmap.tile[tx][ty].owned_by(pturn))
        {
          var.unitmenu = worldmap.tile[tx][ty].building_type();
          var.logicstate = IN_UNIT_BUILD_MENU;
          var.unitx = tx;
          var.unity = ty;
          var.oldmx = mx;
          var.oldmy = my;
        }
      }
    }
  }
}

void build_menu()
{
	int t = do_unitmenu(var.unitmenu, var.unitx, var.unity, pturn);
	if (t != 0)  //do_unitmenu should return 1 when a unit has been
	{            //created or -1 if the player cancels the unitmenu
		var.logicstate = NOTHING_HAPPENING;
		
		if ((t == 1) && (global.netgame) && (!net.server))
		{  //if it wants to create a unit, but is a client in a netgame, wait for a server response
			var.logicstate = NET_WAITING_FOR_RESPONSE;
		}
	}
}

void general_menu()
{
	string s = do_generalmenu();
	DyBytes data;
	
  if (s != "-")
  {
    if (s == "Quit")
    {
			if (global.netgame)
			{
				if (net.server)
				{
					if (clientconnected(0)) net.engine->disconnectThread(net.clientthread[0]);
					if (clientconnected(1)) net.engine->disconnectThread(net.clientthread[1]);
					if (clientconnected(2)) net.engine->disconnectThread(net.clientthread[2]);
				}
				else
				{
					net.engine->disconnectThread(net.thread);
				}
			}
			pturn = 0;
			var.logicstate = BACK_TO_MAIN_MENU;
		}
		else if (s == "Options")
		{
			var.logicstate = IN_OPTIONS_MENU;
		}
		else if (s == "Help")
		{
			var.logicstate = IN_HELP_SCREEN;
		}
		else if (s == "Save")
		{
			var.logicstate = SAVING_GAME;
		}
		else if (s == "Surrender")
		{
			if (!global.netgame || (global.netgame && net.server))
			{
				if (global.netgame)
				{
					data.clear();
					data.addByte(NET_PLAYER_SURRENDER);
					data.addByte(pturn);
					data.addByte(global.surrenderval);
					broadcast_datablob(&data);
				}
				
				do_surrender(pturn, global.surrenderval);
				var.logicstate = NOTHING_HAPPENING;
			}
			else
			{
				data.clear();
				data.addByte(NET_CLIENT_SURRENDER);
				data.addByte(global.surrenderval);
				send_datablob(net.thread, &data);
				var.logicstate = NET_WAITING_FOR_RESPONSE;
			}
		}
		else if (s == "Cancel")
		{
			var.logicstate = NOTHING_HAPPENING;
		}
	}
}

void unit_moving()
{
	bool deselect = false;
	int z;
	_unit *u = var.unitselected;
	_tile *t;
	
	u->do_movement();
	u->draw();
	if (!u->is_moving())  //if the unit has just stopped moving
	{
		var.oldmx = mouse_x;
		var.oldmy = mouse_y;
		z = unit_here_thorough(pturn, u->tilex, u->tiley, u->number);
		
		if (z != -1)  //another unit is on this tile
		{
			var.unitselected = NULL;
			var.logicstate = NOTHING_HAPPENING;
			
			if (player[pturn].unit[z].can_load_unit(u) == 1)
			{  //if it can be loaded, load
				player[pturn].unit[z].load_unit(u);
				worldmap.tile[u->tilex][u->tiley].set_unit(pturn, z);  //makes sure unitmap is
				u->exists = 0;                                         //correct for this spot
				u->health = 0;
			}
			else
			{  //otherwise, this must be a merge
				merge_units(u, u->tilex, u->tiley);
				if (u->ready == 1)
				{
					if (player[u->color].controller == HUMAN)
					{
						var.unitselected = u;
						var.logicstate = UNIT_SELECTED;
					}
				}
			}
			return;
		}
		
		if (unitstats[u->type].attacktype == RANGED)
		{  //if it's a ranged attacker, it can't do anything after moving
			u->ready = 0;
			var.unitselected = NULL;
			var.logicstate = NOTHING_HAPPENING;
			return;
		}
		
		t = &worldmap.tile[u->tilex][u->tiley];
		if (unitstats[u->type].cancapture)
		{
			if ((t->is_building()) && ((t->is_unit_producing()) || (t->is_HQ())))
			{  //if it's a building that gets permanently captured
				if (!t->owned_by(u->color))
				{  //if it's not owned by the player
					if (t->is_HQ())
					{  //if we don't do this, winning the game by HQ capture doesn't happen until you deselect the unit
						deselect = true;
					}
					capture_building(u);
				}  // (this means you can capture teammates' buildings)
			}
		}
		
		if ((player[u->color].controller != HUMAN) || (deselect))
		{  //if it was a network player's unit or an AI, or the unit should be deselected for another reason
			var.unitselected = NULL;
			var.logicstate = NOTHING_HAPPENING;
			return;
		}
		var.logicstate = UNIT_SELECTED;
	}
}

void unit_selected()
{
	static bool first = true;
	static float sel, seldir;
	static vector<_loc> attackables;
	
  int mx = mouse_x;
  int my = mouse_y;
  int tx, ty;
  int x, y, t, z, dx, dy, damage;
	char damagepercent[10];
  bool ok;
  _unit *u = var.unitselected;
	DyBytes data;
	
	if (first)
	{
		attackables = find_attackables(u);
		do_moveselect(-1, -1, -1, -1);  //this resets do_moveselect's static ints
		sel = 0.5;
		seldir = 0.02;
		first = false;
	}

	x = (u->tilex - worldmap.scroll_x) * MAP_TILE_SIZE + worldmap.offset_x;
	y = (u->tiley - worldmap.scroll_y) * MAP_TILE_SIZE + worldmap.offset_y;
	buffer_rect(x, y, x + MAP_TILE_SIZE, y + MAP_TILE_SIZE, WHITE, sel);
	sel += seldir;
	if ((sel >= 1.0) || (sel <= 0.5))
	{
		seldir *= -1;
	}
	
	draw_attackables(attackables);
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
  if ((tx < worldmap.scroll_x + XTILES) && (ty < worldmap.scroll_y + YTILES))
	{
		if (u->canmove)
		{
			do_moveselect(pturn, var.unitselected->number, tx, ty);
			z = any_unit_here(tx, ty);
			if ((z != -1) && (worldmap.tile[tx][ty].get_step() <= var.unitselected->move) && (can_merge(u, &player[z / 100].unit[z % 100])))
			{
				if (my < 580)
				{
					buffer_textout_ex(med, "MERGE", mx + 14, my + 14, BLACK);
					buffer_textout_ex(med, "MERGE", mx + 15, my + 15, WHITE);
				}
				else
				{
					buffer_textout_ex(med, "MERGE", mx + 14, my + 14, BLACK);
					buffer_textout_ex(med, "MERGE", mx + 15, my - 15, WHITE);
				}
			}
		}
		if (gameoptions.show_damage)
		{
			z = attackable_unit_here(u, tx, ty, true);
			if (z != -1)
			{
				damage = int(total_damage(u, &player[z / 100].unit[z % 100]));
				sprintf(damagepercent, "%#.2f", float(damage) / 10.0);
				dx = mx + 14 - worldmap.offset_x;
				dy = my + 14 - worldmap.offset_y;
				buffer_rectfill(dx - 1, dy, dx + normal.Width(damagepercent), dy + 10, BLACK, 0.5);
				buffer_textout_ex(normal, damagepercent, dx, dy, WHITE);
				buffer_rect(dx - 1, dy, dx + normal.Width(damagepercent), dy + 10, DGREY);
			}
		}
	}
	else
	{  //if the unit is selected, but the mouse is on the sidebar or bottom bar
		if (u->canmove)
		{
			show_movable_tiles(u);
  		draw_path(u);
			u->draw(false);
		}
	}
	check_scrolling();
	
	if ((mouse_clicked() == 1) && ((tx < worldmap.scroll_x + XTILES) && (ty < worldmap.scroll_y + YTILES)))
	{
		t = worldmap.tile[tx][ty].unit_here();
		
		if ((t != -1) && (player[t / 100].team != player[u->color].team))
		{  //if there's an enemy on the clicked tile
			z = attackable_unit_here(u, tx, ty, true);
			if (z != -1)
			{  //this check is needed: just because it's an enemy doesn't mean it's attackable
				if ((!global.netgame) || (global.netgame && net.server))
				{
					var.logicstate = IN_BATTLE;
					var.attacker = (pturn * 100) + var.unitselected->number;
					var.defender = z;
					
					//sending battle data is taken care of in the battle function itself
				}
				else
				{
					data.clear();
					data.addByte(NET_CLIENT_ATTACK);
					data.addByte(var.unitselected->tilex);
					data.addByte(var.unitselected->tiley);
					data.addByte(player[z / 100].unit[z % 100].tilex);
					data.addByte(player[z / 100].unit[z % 100].tiley);
					send_datablob(net.thread, &data);
					var.logicstate = NET_WAITING_FOR_RESPONSE;
				}
				var.unitselected = NULL;
				first = true;
      }
		}
		else if (u->canmove)
		{
    	ok = false;
			if ((tx == u->tilex) && (ty == u->tiley))
			{
				ok = false;
			}
			else if ((t != -1) && (t / 100 == pturn))
    	{  //if another of the player's units is here
	      if (movelist.size() == 0)  //the unit hasn't moved at all, so the unit
      	{                          //detected on the tile is just the selected unit
	        ok = true;
      	}
      	else if (player[pturn].unit[t % 100].can_load_unit(u))
      	{  //if a unit is here but can load the selected unit, everything's OK
	        ok = true;
      	}
      	else if (player[pturn].unit[t % 100].type == u->type)
      	{  //or if it is the same unit type and is damaged, they can merge
	        if (can_merge(u, &player[pturn].unit[t % 100]))
        	{
	          ok = true;
        	}
      	}
    	}
    	else
    	{  //there is no unit or an allied unit here
      	if (worldmap.tile[tx][ty].get_step() <= u->move)
      	{  //only have movement if the mouse is on a movable tile
	        ok = true;
      	}
      	if (t != -1)
      	{  //if there is an allied unit here
	        ok = false;
      	}
    	}
    	if (ok)
    	{
	      if (movelist.size() > 0)  //so we can tell if a unit has been ordered to
      	{                         //move, which is important for ranged-attack units
					if (!global.netgame)
					{
						u->canmove = false;
						play_moving_sound(u->type);
						u->set_moves(movelist);
						var.logicstate = UNIT_MOVING;
					}
					else
					{  //a netgame
						if (net.server)
						{
							u->canmove = false;
							play_moving_sound(u->type);
							u->set_moves(movelist);
							var.logicstate = UNIT_MOVING;
							
							data.clear();
							data.addByte(NET_MOVEUNIT);
							data.addByte(u->tilex);
							data.addByte(u->tiley);
							data.addByte(movelist.size());
							z = 0;
							while ((unsigned)z < movelist.size())
							{
								data.addByte(movelist[z]);
								z++;
							}
							broadcast_datablob(&data);
						}
						else
						{  //client just requests that the unit be moved
							data.clear();
							data.addByte(NET_CLIENT_MOVEUNIT);
							data.addByte(u->tilex);
							data.addByte(u->tiley);
							data.addByte(movelist.size());
							z = 0;
							while ((unsigned)z < movelist.size())
							{
								data.addByte(movelist[z]);
								z++;
							}
							send_datablob(net.thread, &data);
							
							var.unitselected = NULL;
							var.logicstate = NET_WAITING_FOR_RESPONSE;  //the client must wait for the server to approve the movement
						}
					}
				}
				first = true;
    	}
    	else  //clicked on a tile that can't be moved to and with no enemy: deselect current unit
			{
				var.unitselected = NULL;
				var.logicstate = NOTHING_HAPPENING;
				first = true;
			}
		}
		else  //clicked on a tile that can't be moved to and with no enemy: deselect current unit
		{
			var.unitselected = NULL;
			var.logicstate = NOTHING_HAPPENING;
			first = true;
		}
  }
	
	if (mouse_rclicked() == 1)
	{
		first = true;
		var.unitselected = NULL;
		var.logicstate = NOTHING_HAPPENING;
	}
}

void unit_unloading()
{
	static bool first = true;
	static vector<_loc> locations;
	_unit *u = var.unitselected;
	int tx, ty;
	int mx = mouse_x;
	int my = mouse_y;
	DyBytes data;
	
	if (first)
	{
		first = false;
		locations.clear();
		locations.reserve(2);
		locations = find_unload_locations(u, u->load[var.unloadnum].type);
	}
	show_unload_locations(locations);
	if (mouse_clicked() == 1)
	{
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
		if (unit_unloadable_here(u->tilex, u->tiley, tx, ty, u->load[var.unloadnum].type))
		{
			if ((!global.netgame) || (global.netgame && net.server))
			{
				unload_unit(u, var.unloadnum, tx, ty);
				
				if (global.netgame)
				{  //server in a netgame
					data.clear();
					data.addByte(NET_UNLOAD);
					data.addByte(u->tilex);
					data.addByte(u->tiley);
					data.addByte(var.unloadnum);
					data.addByte(tx);
					data.addByte(ty);
					broadcast_datablob(&data);
				}
				
				// If something is exploding, that must mean the unloaded unit can capture, and was unloaded on an HQ
				// In which case, the APC needs to be deselected so the explosion animations can play
				if (!check_explodes()) var.logicstate = UNIT_SELECTED;
				else
				{
					var.unitselected = NULL;
					var.logicstate = NOTHING_HAPPENING;
				}
			}
			else
			{  //client in a netgame
				data.clear();
				data.addByte(NET_CLIENT_UNLOAD);
				data.addByte(u->tilex);
				data.addByte(u->tiley);
				data.addByte(var.unloadnum);
				data.addByte(tx);
				data.addByte(ty);
				send_datablob(net.thread, &data);
				var.logicstate = NET_WAITING_FOR_RESPONSE;
			}
			
			first = true;
		}
	}
	
	if (mouse_rclicked() == 1)
	{
		first = true;
		var.unloadnum = -1;
		var.logicstate = NOTHING_HAPPENING;
	}
	check_scrolling();
}

void unload_unit(_unit *u, int unum, int tx, int ty)
{
	int unloadee, i;
	_unit *v;
	_tile *t;
	
	u->load[unum].loaded = 0;
	unloadee = player[u->color].create_unit(u->load[unum].type, tx, ty, u->color);
	v = &player[u->color].unit[unloadee];
	v->health = u->load[unum].health;
	i = 0;
	while (i < 2)
	{
		if (u->load[unum].subload[i].loaded == 1)
		{
			v->load[i].loaded = 1;
			v->load[i].type = u->load[unum].subload[i].type;
			v->load[i].health = u->load[unum].subload[i].health;
			u->load[unum].subload[i].loaded = 0;
		}
		i++;
	}
	
	i = 0;
	while (i < 20)
	{
		create_particle(tx * MAP_TILE_SIZE + MAP_TILE_SIZE / 2 + rand()%32 - 16,
										ty * MAP_TILE_SIZE + MAP_TILE_SIZE / 2 + rand()%32 - 16, P_SMOKE, WHITE);
		i++;
	}
	
	t = &worldmap.tile[tx][ty];
	if ((t->is_unit_producing()) || (t->is_HQ()))
	{
		if (!(t->owned_by(v->color)))
		{
			capture_building(v);
		}
	}
}

void mechanic_capping()
{
	_unit *u = var.unitselected;
	static bool first = true, good[4];
	int x, y, tx, ty;
	int mx = mouse_x;
	int my = mouse_y;
	int i, j, z;
	DyBytes data;
	
	if (first)
	{
		i = 0;
		while (i < 4)
		{
			good[i] = false;
			x = u->tilex;
			y = u->tiley;
			
			if (i == 0) x++;
			if (i == 1) x--;
			if (i == 2) y++;
			if (i == 3) y--;
			
			if ((x > 0) && (x < worldmap.l) && (y > 0) && (y < worldmap.h))
			{
				j = any_unit_here(x, y);
				if ((j != -1) && (player[j / 100].team != player[pturn].team))
				{
					z = player[j / 100].unit[j % 100].type;
					if ((unitstats[z].movetype != M_INFANTRY) && (unitstats[z].movetype != M_MECH))
					{
						good[i] = true;
					}
				}
			}
			i++;
		}
		first = false;
	}
	
	i = 0;
	while (i < 4)
	{
		if (good[i])
		{
			x = (u->tilex - worldmap.scroll_x) * MAP_TILE_SIZE + worldmap.offset_x;
			y = (u->tiley - worldmap.scroll_y) * MAP_TILE_SIZE + worldmap.offset_y;
			
			if (i == 0) x += MAP_TILE_SIZE;
			if (i == 1) x -= MAP_TILE_SIZE;
			if (i == 2) y += MAP_TILE_SIZE;
			if (i == 3) y -= MAP_TILE_SIZE;
			
			buffer_rectfill(x, y, x + MAP_TILE_SIZE, y + MAP_TILE_SIZE, WHITE, 0.66);
		}
		i++;
	}
	
	if (mouse_clicked() == 1)
	{
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
		
		if (tx == u->tilex + 1) j = 0;
		else if (tx == u->tilex - 1) j = 1;
		else if (ty == u->tiley + 1) j = 2;
		else if (ty == u->tiley - 1) j = 3;
		else j = -1;
		
		if ((j != -1) && (good[j]))
		{
			if ((!global.netgame) || (global.netgame && net.server))
			{
				mechanic_capture(u, tx, ty);
				var.logicstate = NOTHING_HAPPENING;
				
				if (global.netgame && net.server)
				{
					data.clear();
					data.addByte(NET_MECHANIC_CAPTURE);
					data.addByte(pturn);
					data.addByte(u->number);
					data.addByte(tx);
					data.addByte(ty);
					broadcast_datablob(&data);
				}
			}
			else
			{  //netgame client
				var.logicstate = NET_WAITING_FOR_RESPONSE;
				data.clear();
				data.addByte(NET_CLIENT_MECHCAP);
				data.addByte(u->number);
				data.addByte(tx);
				data.addByte(ty);
				send_datablob(net.thread, &data);
			}
				
			var.unitselected = NULL;
			first = true;
		}
	}
	
	if (mouse_rclicked() == 1)
	{
		first = true;
		var.logicstate = NOTHING_HAPPENING;
	}
	check_scrolling();
}

void mechanic_capture(_unit *capper, int captx, int capty)
{
	int z = any_unit_here(captx, capty);
	int p = capper->color;
	int x, y, i;
	_unit *v = &player[z / 100].unit[z % 100];
	
	x = capper->tilex * MAP_TILE_SIZE + MAP_TILE_SIZE / 2;
	y = capper->tiley * MAP_TILE_SIZE + MAP_TILE_SIZE / 2;
	i = 0;
	while (i < 15)
	{
		create_particle(x + rand()%MAP_TILE_SIZE - MAP_TILE_SIZE / 2, y + rand()%MAP_TILE_SIZE - MAP_TILE_SIZE / 2, P_DUST);
		i++;
	}
	
	capper->exists = 0;
	capper->health = 0;
	worldmap.tile[capper->tilex][capper->tiley].set_unit(-1, -1);
	
	i = player[p].create_unit(v->type, v->tilex, v->tiley, p);
	player[p].unit[i].health = v->health;
	player[p].unit[i].load[0] = v->load[0];
	player[p].unit[i].load[1] = v->load[1];
	v->exists = 0;
	v->health = 0;
	worldmap.tile[captx][capty].set_unit(p, i);
	
	x = captx * MAP_TILE_SIZE + MAP_TILE_SIZE / 2;
	y = capty * MAP_TILE_SIZE + MAP_TILE_SIZE / 2;
	create_particle(x, y, P_QUADSPIRAL, player[p].tintcolor);
}


void unit_teleporting()
{
  static bool first = true;
  static bool selecting;
  static int destx, desty, time;
  static float a, inc;
  int t1, t2, t3, t4;
  _unit *u = var.unitselected;
  _tile *t;
	DyBytes data;
  
  if (first)
  {
    selecting = true;
    destx = -1;
    desty = -1;
    a = 0.5;
    inc = 0.02;
    time = 0;
    first = false;
  }
  
  if (selecting)
  {
    t2 = worldmap.scroll_y - 1;
    if (t2 < 0) t2 = 0;
    while ((t2 < worldmap.scroll_y + YTILES + 1) && (t2 < worldmap.h))
    {
      t1 = worldmap.scroll_x - 1;
      if (t1 < 0) t1 = 0;
      while ((t1 < worldmap.scroll_x + XTILES + 1) && (t1 < worldmap.l))
      {
        if (worldmap.tile[t1][t2].same_type(6, 0))
        {
          t3 = (t1 - worldmap.scroll_x) * MAP_TILE_SIZE;
          t4 = (t2 - worldmap.scroll_y) * MAP_TILE_SIZE;
          buffer_rect(t3 + worldmap.offset_x, t4 + worldmap.offset_y, t3 + MAP_TILE_SIZE + worldmap.offset_x, t4 + MAP_TILE_SIZE + worldmap.offset_y, WHITE, a);
        }
        t1++;
      }
      t2++;
    }
    a += inc;
		if (a >= 1.0)
		{
			a = 1.0;
			inc = -0.02;
		}
    if (a <= 0.25) inc = 0.02;
		
		buffer_textout_centre_ex(big, "Select Destination", 320, 0, WHITE, a);
		
		if (mouse_clicked() == 1)
		{
			t1 = worldmap.scroll_x + (mouse_x - worldmap.offset_x) / MAP_TILE_SIZE;
			t2 = worldmap.scroll_y + (mouse_y - worldmap.offset_y) / MAP_TILE_SIZE;
			if ((t1 != u->tilex) || (t2 != u->tiley))
			{
				t = &worldmap.tile[t1][t2];
				if (t->same_type(6, 0))  //if this tile is a teleporter
				{
					destx = t1;
					desty = t2;
					selecting = false;
				}
			}
		}
		else if (mouse_rclicked() == 1)
		{
			first = true;
			var.logicstate = UNIT_SELECTED;
		}
    check_scrolling();
  }
  else
  {  //teleportation animation
		if ((!global.netgame) || (global.netgame && net.server))
		{
			start_teleport_animation(u, destx, desty);
			
			if (global.netgame)
			{
				data.clear();
				data.addByte(NET_TELEPORT);
				data.addByte(pturn);
				data.addByte(u->number);
				data.addByte(destx);
				data.addByte(desty);
				broadcast_datablob(&data);
			}
		}
		else
		{  //client in a netgame
			data.clear();
			data.addByte(NET_CLIENT_TELEPORT);
			data.addByte(u->number);
			data.addByte(destx);
			data.addByte(desty);
			send_datablob(net.thread, &data);
			
			var.logicstate = NET_WAITING_FOR_RESPONSE;
		}
		
		var.unitselected = NULL;
		first = true;
  }
}

void start_teleport_animation(_unit *u, int tx, int ty)
{
	teleport_animation(u, tx, ty);
	var.logicstate = TELEPORT_ANIMATION;
}

void teleport_animation(_unit *initunit, int tx, int ty)
{
	static int time = 0, destx = -1, desty = -1;
	static _unit *u = NULL;
	int t1, t2, z;
	
	if (initunit != NULL)
	{  //if initializing the animation
		u = initunit;
		destx = tx;
		desty = ty;
		time = 0;
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
		if (scroll_to_location(destx, desty) == 0)
		{
			time--;
		}
	}
	else if (time == 26)
	{
		z = any_unit_here(destx, desty);
		if (z != -1)
		{
			if (u->color != z / 100) player[u->color].stats.killed++;
			player[z / 100].stats.lost++;
			
			player[z / 100].unit[z % 100].kill();
		}
		if (check_explodes())
		{
			do_explodes();
			time--;
		}
		else
		{
			u->set_tiles(destx, desty);
			create_particle(destx * MAP_TILE_SIZE + MAP_TILE_SIZE / 2, desty * MAP_TILE_SIZE + MAP_TILE_SIZE / 2, P_SHOCKWAVE);
			create_particle(destx * MAP_TILE_SIZE, desty * MAP_TILE_SIZE, P_TELEPORT);
			play_sound(S_TELEPORT);
		}
	}
	else if (time == 34)
	{
		u->exists = 1;
		u->ready = 0;
	}
	else if (time > 50)
	{
		var.logicstate = NOTHING_HAPPENING;
	}
	time++;
}

int money_producing_buildings_owned(int plyr)
{
  int temp1, temp2;
  int num = 0;
  temp1 = 0;
  while (temp1 < worldmap.h)
  {
    temp2 = 0;
    while (temp2 < worldmap.l)
    {
      if (worldmap.tile[temp2][temp1].owned_by(plyr))
      {
				if ((worldmap.tile[temp2][temp1].is_building(CITY)) || (worldmap.tile[temp2][temp1].is_building(HQ)))
				{
					num++;
				}
      }
      temp2++;
    }
    temp1++;
  }
  return num;
}

int unit_here_thorough(int plyr, int tx, int ty, int ignore_n)
{
  int i = 0;
  _unit *u;
  while (i < 50)
  {
    if (i != ignore_n)
    {
      u = &player[plyr].unit[i];
      if (u->exists == 1)
      {
        if ((u->tilex == tx) && (u->tiley == ty))
        {
          return i;
        }
      }
    }
    i++;
  }
  return -1;
}

int rounded_health(float h)
{
  int whole = int(h);
  int part = int((h - float(whole)) * 10);
  if ((whole == 0) && (h > 0))
  {
    return 1;
  }
  else if (whole == 10)
  {
    return 10;
  }
  else
  {
    if (part >= 5)
    {
      return whole + 1;
    }
    else
    {
      return whole;
    }
  }
}

void transform_buildings(int plyr, int plyr2)
{
  int i, j;
  _tile *t;
  i = 0;
  while (i < worldmap.l)
  {
    j = 0;
    while (j < worldmap.h)
    {
      t = &worldmap.tile[i][j];
      if ((t->is_unit_producing()) || (t->is_HQ()))
      {
        if (t->owned_by(plyr))
        {
          if (t->is_HQ())
          {
            t->change_to_city();
            t->change_owner(plyr2);
          }
          else
          {  //building here
            t->change_owner(plyr2);
          }
        }
      }
      j++;
    }
    i++;
  }
  minimap.create();
}

void repair_units(int plyr)
{
  int i = 0;
  int btype;
  bool rep;
  float oldh;
  _unit *u;
  _tile *t;
  while (i < 50)
  {
    u = &player[plyr].unit[i];
    if (u->exists == 1)
    {
      rep = false;
      t = &worldmap.tile[u->tilex][u->tiley];
      if (t->is_building())
      {
        if (t->owned_by(plyr))
        {
          btype = t->building_type();
          switch(unitstats[u->type].basetype)
          {
            case LAND:
              if ((btype == CITY) || (btype == HQ) || (btype == BASE))
              {
                rep = true;
              }
              break;
            case SEA:
              if (btype == PORT)
              {
                rep = true;
              }
              break;
            case AIR:
              if (btype == AIRPORT)
              {
                rep = true;
              }
              break;
          }
        }
      }
      if (rep == true)  //if the unit is on a friendly base
      {
        if (u->health < 10)  //if the unit is damaged, add 2 health but take some cash
        {
          oldh = u->health;
          u->health += 2;
          if (u->health > 10)
          {
            u->health = 10;
          }
          player[plyr].cash -= int((u->health - oldh) / 20 * u->price);
					player[plyr].stats.spent += int((u->health - oldh) / 20 * u->price);
          if (player[plyr].cash < 0)
          {
            player[plyr].cash = 0;
          }
        }
      }
    }
    i++;
  }
}

void play_moving_sound(int utype)
{
	play_sound(unitstats[utype].movesound);
}

void init_explode(int tx, int ty)
{
  _unitexplosion u;
  int i;
  boom.push_back(u);
  i = boom.size() - 1;
  boom[i].exists = 1;
  boom[i].tx = tx;
  boom[i].ty = ty;
  boom[i].frame = 0;
  boom[i].notaunit_oilderrick = false;
}

void init_deadoilderrick(int tx, int ty)
{
	_unitexplosion u;
	int i;
	boom.push_back(u);
	i = boom.size() - 1;
	boom[i].exists = 1;
	boom[i].tx = tx;
	boom[i].ty = ty;
	boom[i].frame = 0;
	boom[i].notaunit_oilderrick = true;
}

void destroy_theplayers_units()
{
	int i = 0;
	char str[20];
	
	i = 0;
	while (i < 50)
	{
		if (player[var.theplayer].unit[i].exists == 1)
		{
			player[var.theplayer].unit[i].kill();
		}
		i++;
	}
	
	sprintf(str, "Player %d Defeated", var.theplayer + 1);
	create_particle(320 - normal.Width(str) / 2, 200, P_FADETEXT, WHITE, str);
}

int do_explodes()
{
  static bool first = true;
  static bool scrolling;
  static int n, x, y;
  unsigned int i;
  
  if (first)
  {
		n = -1;
    i = 0;
    while (i < boom.size())
    {
      if (boom[i].exists == 1)  //find the first explosion on the list
      {
        n = i;
        i = boom.size();
      }
      i++;
    }
		if (n == -1) return 1;
		
    scrolling = true;
    first = false;
  }
  
  if (scrolling)
  {
    if (scroll_to_location(boom[n].tx, boom[n].ty) == 1)
    {  //if it has finished doing the smooth scroll to the explosion site
      scrolling = false;
			
			if (boom[n].notaunit_oilderrick)
			{
				x = boom[n].tx * MAP_TILE_SIZE + (MAP_TILE_SIZE / 2);
				y = boom[n].ty * MAP_TILE_SIZE + (MAP_TILE_SIZE / 2);
				i = 0;
				while (i < 15)
				{
					create_particle(x + rand()%MAP_TILE_SIZE - MAP_TILE_SIZE / 2, y + rand()%MAP_TILE_SIZE - MAP_TILE_SIZE / 2, P_DUST);
					i++;
				}
			}
			else
			{
				x = (boom[n].tx - worldmap.scroll_x) * MAP_TILE_SIZE + worldmap.offset_x;
				y = (boom[n].ty - worldmap.scroll_y) * MAP_TILE_SIZE + worldmap.offset_y;
				play_sound(S_EXPLOSION);
				worldmap.tile[boom[n].tx][boom[n].ty].set_unit(-1, -1);
			}
    }
  }
  if (!scrolling)
  {
    boom[n].frame += 1.5;
		if (boom[n].notaunit_oilderrick)
		{
			create_particle(x + rand()%MAP_TILE_SIZE - MAP_TILE_SIZE / 2, y + rand()%MAP_TILE_SIZE - MAP_TILE_SIZE / 2, P_DUST);
			create_particle(x + rand()%MAP_TILE_SIZE - MAP_TILE_SIZE / 2, y + rand()%MAP_TILE_SIZE - MAP_TILE_SIZE / 2, P_DUST);
			if (int(boom[n].frame) > 30)
			{
				boom[n].exists = 0;
				first = true;
				return 1;
			}
		}
		else
		{
			if (int(boom[n].frame) > 29)
			{
				boom[n].exists = 0;
				first = true;
				return 1;
			}
			else
			{
				create_particle(boom[n].tx * MAP_TILE_SIZE + (MAP_TILE_SIZE / 2), boom[n].ty * MAP_TILE_SIZE + (MAP_TILE_SIZE / 2), 0);
				explode1.draw(int(boom[n].frame), x, y);
			}
		}
  }
  return 0;
}

int secure_base(int plyr)
{
  int x, y, u;
  y = 0;
  while (y < worldmap.h)
  {
    x = 0;
    while (x < worldmap.l)
    {
      if (worldmap.tile[x][y].is_unit_producing())
      {
        if (worldmap.tile[x][y].owned_by(plyr))
        {  //if it belongs to the player
          u = worldmap.tile[x][y].unit_here();
          if ((u == -1) || (u / 100 == plyr) || (player[u / 100].team == player[plyr].team))
          {  //there is no unit here or a unit that belongs to this player or an ally
            return 1;
          }
        }
      }
      x++;
    }
    y++;
  }
  return 0;
}

void capture_building(_unit* u)
{
	char str[20];
  int z;
	_tile *t = &worldmap.tile[u->tilex][u->tiley];
	
	empdebug("Capturing building...");
	
	if (!((t->is_unit_producing()) || (t->is_HQ())))
	{  //can't do a capture action on a building that doesn't create units and isn't an HQ
		empdebug("Wait, this building can't be captured...  ERROR!!!");
		empdebug(u->tilex);
		empdebug(u->tiley);
		return;
	}
	
	if (t->is_HQ())
	{  //if a player's HQ was captured
		z = t->get_owner();
		transform_buildings(z, u->color);  //captured player's buildings go to the capturer
		var.logicstate = PLAYER_DEFEATED;
		var.theplayer = z;
		destroy_theplayers_units();
		player[z].playing = 0;
		sprintf(str, "HQ Captured");
		create_particle(320 - normal.Width(str) / 2, 400, P_FADETEXT, WHITE, str);
	}
	else
	{
		t->change_owner(u->color);
		str[0] = '\0';
		switch (t->building_type())
		{
			case BASE:
				sprintf(str, "Factory Captured");
				break;
			case AIRPORT:
				sprintf(str, "Airfield Captured");
				break;
			case PORT:
				sprintf(str, "Shipyard Captured");
				break;
		}
		create_particle(320 - normal.Width(str) / 2, 450, P_FADETEXT, WHITE, str);
	}
	minimap.create();
  empdebug("Building capture function complete.");
}

bool can_merge(_unit *a, _unit *b)
{
  int numloaded = 0;
	if (a->type != b->type) return false;
	if (a->color != b->color) return false;
	if (a == b) return false;  //can't merge with itself
  if ((a->health < 10) || (b->health < 10))
  {
    if (a->load[0].loaded == 1)
    {
      numloaded++;
    }
    if (a->load[1].loaded == 1)
    {
      numloaded++;
    }
    if (b->load[0].loaded == 1)
    {
      numloaded++;
    }
    if (b->load[1].loaded == 1)
    {
      numloaded++;
    }
		if (numloaded <= unitstats[a->type].loadcapacity)
		{
			return true;
		}
  }
  return false;
}

void merge_units(_unit* merger, int tx, int ty)
{
  int u = unit_here_thorough(merger->color, tx, ty, merger->number);
  _unit* mergee = &player[merger->color].unit[u];
  merger->health += mergee->health;
  if (merger->health > 10)
  {
  	player[merger->color].cash += int(merger->price * (merger->health - 10.0) / 10.0);
  	if (player[merger->color].cash > 1000000) player[merger->color].cash = 1000000;
    merger->health = 10;
  }
  if (mergee->load[0].loaded == 1)
  {
    if (merger->load[0].loaded == 0)
    {
      merger->load[0] = mergee->load[0];
    }
    else
    {
      merger->load[1] = mergee->load[0];
    }
  }
  if (mergee->load[1].loaded == 1)
  {
    if (merger->load[0].loaded == 0)
    {
      merger->load[0] = mergee->load[1];
    }
    else
    {
      merger->load[1] = mergee->load[1];
    }
  }
  if (mergee->ready == 0) merger->ready = 0;
  worldmap.tile[tx][ty].set_unit(merger->color, merger->number);
  mergee->exists = 0;
}

bool check_explodes()
{
  bool emptyboom = true;
  int temp1, temp2;
  if ((var.logicstate != EXPLODING) && (var.logicstate != IN_BATTLE))
  {  //if it's not in the middle of an explosion animation
    temp1 = 0;
    temp2 = boom.size();
    while (temp1 < temp2)
    {
      if (boom[temp1].exists == 1)  //check to see if it should be
      {
        emptyboom = false;
      }
      temp1++;
    }
    if (emptyboom)
    {  //if there aren't any existing explosions left
      boom.resize(0);  //get rid of all the finished explosions
    }
  }
  return !emptyboom;
}

void update_cities()
{
	char str[15];
	int i, j, z;
	_tile *t;
	
	j = 0;
	while (j < worldmap.h)
	{
		i = 0;
		while (i < worldmap.l)
		{
			t = &worldmap.tile[i][j];
			if ((t->is_building()) && (!(t->is_unit_producing())) && (!(t->is_HQ())))
			{  //if it's an oil derrick
				z = t->unit_here();
				if (z == -1)
				{
					t->change_owner(-1);
				}
				else if (unitstats[player[z / 100].unit[z % 100].type].cancapture)
				{
					if (!(t->owned_by(z / 100)))
					{
						t->change_owner(z / 100);
						sprintf(str, "Derrick Held");
						create_particle(320 - normal.Width(str) / 2, 450, P_FADETEXT, WHITE, str);
					}
				}
			}
			i++;
		}
		j++;
	}
	minimap.create();
}

void update_burning()
{
	const int d_direct = 75, d_adjacent = 15;
	int i, j, k, x, y, z;
	_unit *u;
	
	j = 0;
	while (j < worldmap.h)
	{
		i = 0;
		while (i < worldmap.l)
		{
			if (worldmap.tile[i][j].burning)
			{
				z = worldmap.tile[i][j].unit_here();
				if (z != -1)
				{
					u = &player[z / 100].unit[z % 100];
					if (u->exists == 1)
						do_firedamage(u, d_direct);
				}
				
				k = 0;
				while (k < 4)
				{
					x = i;
					y = j;
					if (k == 0)
					{
						x--;
					}
					else if (k == 1)
					{
						x++;
					}
					else if (k == 2)
					{
						y--;
					}
					else
					{
						y++;
					}
					if ((x > -1) && (x < worldmap.l) && (y > -1) && (y < worldmap.h))
					{
						z = worldmap.tile[x][y].unit_here();
						if (z != -1)
						{
							u = &player[z / 100].unit[z % 100];
							if (u->exists == 1)
								do_firedamage(u, d_adjacent);
						}
						if (worldmap.tile[i][j].burnedfor > 0)
						{  //if this forest wasn't just set on fire during this loop
							if ((!worldmap.tile[x][y].burning) && (worldmap.tile[x][y].same_type(0, 2)))
							{  //if there's an adjacent unburned forest
								if (rand()%4 == 0)
								{  //1 in 4 chance of it catching on fire
									worldmap.tile[x][y].burning = true;
									worldmap.tile[x][y].burnedfor = 0;
								}
							}
						}
					}
					k++;
				}
				worldmap.tile[i][j].burnedfor++;
				if (worldmap.tile[i][j].burnedfor > 4 + rand()%4)
				{
					worldmap.tile[i][j].burning = false;
					worldmap.tile[i][j].change_type(0, 20);
				}
			}
			i++;
		}
		j++;
	}
}

void do_firedamage(_unit *u, int damage)
{
	int d = 0;
	
	if (unitstats[u->type].basetype == AIR) return;
	
	if (damage - u->resist[D_FIRE] > 0)
	{
		d = damage - u->resist[D_FIRE];
	}
	else
	{
		d = int(10.0 * float(damage) / float(u->resist[D_FIRE]));
	}
	u->health -= d / 10.0;
	
	if (global.netgame)
	{
		if (net.server)
		{
			broadcast_unithealth(u);
		}
	}
	
	if (u->health < 0.01)
	{
		u->kill();
		player[u->color].stats.lost++;
	}
}

void debug_savegame()
{
  ofstream savefile("saves/gamedebug.sav");
  if (!(savefile.bad()))
  {
    savefile << "Debuggery\n" << worldmap.l << "\n" << worldmap.h << "\n";

		save_map_and_players(&savefile);
		
		savefile << "pturn " << pturn << "\n" << gameoptions.funding << "\n";
		savefile << gameoptions.show_damage << "\n" << gameoptions.time << "\n";
		savefile << var.time << "\n" << var.techlevel << "\n" << var.cashturns << "\n";
		savefile << worldmap.get_name().c_str();
  }
  savefile.close();
}

void save_map_and_players(ofstream *savefile)
{
	int o, p;
	_tile *tile;

  o = 0;
  while (o < worldmap.h)
  {
    p = 0;
    while (p < worldmap.l)
    {
      tile = &worldmap.tile[p][o];
      *savefile << ((tile->get_set() * 100) + tile->get_number());
      *savefile << " " << tile->get_owner() << " " << tile->burning;
      *savefile << " " << tile->burnedfor << " " << tile->turnsused;
      if (p != worldmap.l - 1)
      {
        *savefile << " ";
      }
      p++;
    }
    *savefile << "\n";
    o++;
  }

  o = 0;
  while (o < 4)
  {
    *savefile << "playing " << player[o].playing << "\n";
    *savefile << "everplaying " << player[o].everplaying << "\n";
    if (player[o].everplaying == 1)
    {
      *savefile << "number " << player[o].number << "\n";
      *savefile << "cash " << player[o].cash << "\n";
      *savefile << "controller " << player[o].controller << "\n";
      *savefile << "units " << player[o].units_in_play() << "\n";
      *savefile << "team " << player[o].team << "\n";
      *savefile << "tookturn " << player[o].tookturn << "\n";
      *savefile << "tintcolor " << player[o].tintcolor << "\n";
			*savefile << "stat_raised " << player[o].stats.raised << "\n";
			*savefile << "stat_spent " << player[o].stats.spent << "\n";
			*savefile << "stat_produced " << player[o].stats.produced << "\n";
			*savefile << "stat_killed " << player[o].stats.killed << "\n";
			*savefile << "stat_lost " << player[o].stats.lost << "\n";
    }
    p = 0;
    if (player[o].playing != 1)
    {
      p = 50;
    }
    while (p < 50)
    {
      if (player[o].unit[p].exists == 1)
      {
        *savefile << "unittype " << player[o].unit[p].type << "\n";
        *savefile << "unittilex " << player[o].unit[p].tilex << "\n";
        *savefile << "unittiley " << player[o].unit[p].tiley << "\n";
        *savefile << "unithealth " << player[o].unit[p].health << "\n";
        *savefile << "unitready " << player[o].unit[p].ready << "\n";
        *savefile << "unitattackdelay " << player[o].unit[p].attackdelay << "\n";
        *savefile << "unitcanmove " << player[o].unit[p].canmove << "\n";
        *savefile << "unitload0loaded " << player[o].unit[p].load[0].loaded << "\n";
        *savefile << "unitload0type " << player[o].unit[p].load[0].type << "\n";
        *savefile << "unitload0health " << player[o].unit[p].load[0].health << "\n";
        *savefile << "unitload0subloaded0 " << player[o].unit[p].load[0].subload[0].loaded << "\n";
        *savefile << "unitload0subtype0 " << player[o].unit[p].load[0].subload[0].type << "\n";
        *savefile << "unitload0subhealth0 " << player[o].unit[p].load[0].subload[0].health << "\n";
        *savefile << "unitload0subloaded1 " << player[o].unit[p].load[0].subload[1].loaded << "\n";
        *savefile << "unitload0subtype1 " << player[o].unit[p].load[0].subload[1].type << "\n";
        *savefile << "unitload0subhealth1 " << player[o].unit[p].load[0].subload[1].health << "\n";
        *savefile << "unitload1loaded " << player[o].unit[p].load[1].loaded << "\n";
        *savefile << "unitload1type " << player[o].unit[p].load[1].type << "\n";
        *savefile << "unitload1health " << player[o].unit[p].load[1].health << "\n";
        *savefile << "unitload1subloaded0 " << player[o].unit[p].load[1].subload[0].loaded << "\n";
        *savefile << "unitload1subtype0 " << player[o].unit[p].load[1].subload[0].type << "\n";
        *savefile << "unitload1subhealth0 " << player[o].unit[p].load[1].subload[0].health << "\n";
        *savefile << "unitload1subloaded1 " << player[o].unit[p].load[1].subload[1].loaded << "\n";
        *savefile << "unitload1subtype1 " << player[o].unit[p].load[1].subload[1].type << "\n";
        *savefile << "unitload1subhealth1 " << player[o].unit[p].load[1].subload[1].health << "\n";
      }
      p++;
    }
    o++;
  }
}

bool sync_burning()
{
	int i;
	DyBytes data;
	
	if (net.server)
	{
		update_burning();
		broadcast_tiledata();
		
		data.clear();
		data.addByte(NET_FINISHEDSYNC);
		broadcast_datablob(&data);
		return true;
	}
	else  //if this is a client machine
	{
		data = get_host_datablob();
		if (data.size() > 0)
		{
			i = data.getByte(0);
			data.removeFront(1);
			switch(i)
			{
				case NET_UPDATINGTILES:
					get_tiledata(&data);
					break;
				case NET_UNITHEALTH:
					get_unithealth(&data);
					break;
				case NET_FINISHEDSYNC:
					return true;
					break;
			}
		}
	}
	return false;
}

void broadcast_unithealth(_unit *u)
{
	DyBytes data;
	
	data.clear();
	data.addByte(NET_UNITHEALTH);
	data.addByte(u->tilex);
	data.addByte(u->tiley);
	data.appendVariable(u->health);
	
	broadcast_datablob(&data);
}

void get_unithealth(DyBytes *data)
{
	int p = data->getByte(0), q = data->getByte(1);
	int z = any_unit_here(p, q);
	_unit *u;
	
	data->removeFront(2);
	
	if (z == -1)
	{
		empdebug("NET ERROR: Received unit health, but no unit on that tile!");
		empdebug(p);
		empdebug(q);
		return;
	}
	
	u = &player[z / 100].unit[z % 100];
	data->extractVariable(0, u->health);
	
	if (u->health < 0.01)
	{
		u->kill();
		player[u->color].stats.lost++;
	}
}

void broadcast_tiledata()
{
	int x, y;
	_tile *t;
	DyBytes data, tiledata;
	
	tiledata.clear();
	y = 0;
	while (y < worldmap.h)
	{
		x = 0;
		while (x < worldmap.l)
		{
			t = &worldmap.tile[x][y];
			tiledata.addByte(t->burning);
			tiledata.addByte(t->burnedfor);
			tiledata.addByte(t->get_set());
			tiledata.addByte(t->get_number());
			tiledata.addByte(t->get_owner());
			x++;
		}
		y++;
	}
	tiledata.compress(9);
	data.clear();
	data.addByte(NET_UPDATINGTILES);
	data.append(tiledata);
	broadcast_datablob(&data);
}

void get_tiledata(DyBytes *data)
{
	int x, y;
	_tile *t;
	
	data->decompress();
	y = 0;
	while (y < worldmap.h)
	{
		x = 0;
		while (x < worldmap.l)
		{
			t = &worldmap.tile[x][y];
			t->burning = data->getByte(0);
			t->burnedfor = data->getByte(1);
			if (!t->same_type(data->getByte(2), data->getByte(3)))
			{
				t->change_type(data->getByte(2), data->getByte(3));
			}
			t->change_owner(data->getByte(4));
			data->removeFront(5);
			
			x++;
		}
		y++;
	}
}

void draw_net_icon(int x, int y)
{
	static float a = 0.0;
	static float av = 0.04;
	
	a += av;
	if (a >= 1.0)
	{
		a = 1.0;
		av = -av;
	}
	else if (a <= 0.0)
	{
		a = 0.0;
		av = -av;
	}
	
	buffer_draw_sprite(glb.bmp(41), x, y, a);
}

void draw_wait_icon(int x, int y)
{
	static float a = 0.0;
	static float av = 0.04;
	
	a += av;
	if (a >= 1.0)
	{
		a = 1.0;
		av = -av;
	}
	else if (a <= 0.0)
	{
		a = 0.0;
		av = -av;
	}
	
	buffer_draw_sprite(glb.bmp(50), x, y, a);
}

void draw_shieldrange(_unit *u)
{
	int i, j, x, y;
	static float alpha = 0.3;
	static float alphachange = 0.007;
	
	j = u->tiley - SHIELD_RADIUS;
	if (j < 0) j = 0;
	while ((j <= u->tiley + SHIELD_RADIUS) && (j < worldmap.h))
	{
		i = u->tilex - SHIELD_RADIUS;
		if (i < 0) i = 0;
		while ((i <= u->tilex + SHIELD_RADIUS) && (i < worldmap.l))
		{
			if (tile_distance(i, j, u->tilex, u->tiley) <= SHIELD_RADIUS)
			{
				if ((i != u->tilex) || (j != u->tiley))
				{
					x = (i - worldmap.scroll_x) * MAP_TILE_SIZE + worldmap.offset_x;
					y = (j - worldmap.scroll_y) * MAP_TILE_SIZE + worldmap.offset_y;
					buffer_rectfill(x, y, x + MAP_TILE_SIZE, y + MAP_TILE_SIZE, BLUE, alpha);
				}
			}
			i++;
		}
		j++;
	}
	alpha += alphachange;
	if ((alpha >= 0.66) || (alpha <= 0.3)) alphachange *= -1;
}

bool help_screen()
{
	glb.draw(53, 0, 0);
	if (mouse_clicked()) return true;
	return false;
}

void do_surrender(int from, int to)
{
	var.theplayer = from;
	if (to == -1) destroy_theplayers_units();
	else convert_theplayers_units(from, to);
	
	transform_buildings(from, to);
	player[from].playing = 0;
	
	empdebug("SURRENDERING");
	empdebug("from");
	empdebug(from);
	empdebug("to");
	empdebug(to);
	
	if (pturn == from)
	{
		while (player[pturn].playing != 1)
		{
			pturn++;
			if (pturn >= 4) pturn = 0;
		}
	}
}

void convert_theplayers_units(int from, int to)
{
	bool full = false;
	int a, i = 0;
	char str[30];
	_unit *u, *v;
	
	while ((i < 50) && !full)
	{
		u = &player[from].unit[i];
		if (u->exists == 1)
		{
			a = player[to].create_unit(u->type, u->tilex, u->tiley, to);
			if (a == -1)
			{
				full = true;
				i--;
			}
			else
			{
				v = &player[to].unit[a];
				v->health = u->health;
				v->load[0] = u->load[0];
				v->load[1] = u->load[1];
				u->exists = 0;
				u->health = 0;
				worldmap.tile[v->tilex][v->tiley].set_unit(to, a);
			} 
		}
		i++;
	}
	while (i < 50)
	{
		u = &player[from].unit[i];
		if (u->exists == 1)
		{
			player[to].cash += int(float(u->price) * (u->health / 10.0));
			u->kill();
		}
		i++;
	}
	
	sprintf(str, "Player %d Defeated", from + 1);
	create_particle(320 - normal.Width(str) / 2, 200, P_FADETEXT, WHITE, str);
}
