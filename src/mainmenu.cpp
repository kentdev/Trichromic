#include "empire.h"

int numplayers;
int loaded_time;

_button start[8];
_button back;
_button ok;
_button opt[10];

_ingameoptions gameoptions;

extern void create_sidebars();
extern void load_saved_characters(const char *savefilename);
extern void draw_minimap(int x, int y, string file);
extern void refresh_maplist();
//extern int ingame_options();
extern int do_gameoptions(bool ingame = false);

void assign_tintcolor(int p, int *colors);
void description(int c, int x, int y, bool centeredtext = true);
void description(_character *c, int x, int y, bool centeredtext = true);
void list_players(int numplayers);
void display_powers(_character *c, int x, int y, int iconsperrow);
void get_connected_beaches();
void add_surrounding_beaches(int bnum, int x, int y);
void get_teleports();
int is_beach(int x, int y);
int beach_already_listed(int x, int y);
int do_first();
int do_load();
//int do_options();
int do_gameload();
//int do_transition(int target);
string describe_strength(int num);
string describe_weakness(int num);

vector<_connectedlocs> beaches;
vector<_loc> teleports;

int menu()
{
  static int menunum = MENU_FIRST;
  static int startsongin = 2;
  //int oldmenunum = menunum;
  int t;
  
  if (startsongin == 0)
  {  //delay the song by 2 logic loops (so it isn't triggered early by transition)
    play_song(title_music);
    startsongin--;
  }
  else if (startsongin > 0)
  {
    startsongin--;
  }
  
  switch (menunum)
  {
    case MENU_FIRST:
      menunum = do_first();
      break;
    case MENU_LOAD:
      menunum = do_load();
      break;
    /*case MENU_OPTIONS:
      menunum = do_options();
      break;*/
    case MENU_GAME_OPTIONS:
      menunum = do_gameoptions();
      break;
    /*case MENU_INGAME_OPTIONS:
      menunum = ingame_options();
      break;*/
    case MENU_GAME_LOAD:
      menunum = do_gameload();
      break;
    case QUIT:
      return QUIT;
      break;
    default:        //default cases would lead out of the menu, so
      t = menunum;  //the menu has to open in FIRST next time
      menunum = MENU_FIRST;
      return t;
      break;
  }
  return MENU;
}





int do_first()
{
	static bool first = true, firstloop;
	static short icondelay;
	int x, y;
	
	if (first)
	{
		empdebug("  init main menu...");
		firstloop = true;
		player[0].tintcolor = player_color(0, 0);
		player[1].tintcolor = player_color(1, 0);
		player[2].tintcolor = player_color(2, 0);
		player[3].tintcolor = player_color(0, 2);
		
		start[0].init(381, 96, 0, 0, "Hotseat", 3);
		start[1].init(583, 81, 0, 0, "Load Game", 4);
		start[7].init(380, 299, 0, 0, "Network Game", 3);
		start[6].init(690, 570, 100, 20, "How to Play", 5);
		start[2].init(581, 402, 0, 0, "Quit", 4);  //quit
		start[3].init(556, 199, 0, 0, "Options", 3);
		start[4].init(305, 240, 0, 0, "Map Editor", 4);
		start[5].init(10, 570, 100, 20, "Credits", 5);
		//start[5].init(288, 420, 100, 20, "Player Editor", 1);
		icondelay = 0;
		first = false;
		clear_particles();
		empdebug("  ...finished init main menu");
	}
  
  if (icondelay <= 0)
  {
		x = rand()%700 + 50;
		y = rand()%500 + 50;
		
		create_particle(x, y, P_LOGO);
		icondelay = 25;
  }
  else icondelay--;

	if (firstloop) empdebug("---First loop through main menu---");
	glb.draw(32, 0, 0);
	do_particles();
	if (firstloop) empdebug("   Successfully drew particles");
	glb.draw(39, 0, 0);
	//glb.draw(0, 140, 100);
	//glb.draw(1, 58, 0);

	start[0].check();
	start[1].check();
	start[2].check();
	start[3].check();
	start[4].check();
	start[5].check();
	start[6].check();
	
	buffer_textout_ex(normal, "1.0", 5, 5, WHITE);
	
	if (global.networking) start[7].check();

  if (start[0].clicked())
  {
		global.netgame = false;
    first = true;
    return MENU_LOAD;
  }
  if (start[1].clicked())
  {
		global.netgame = false;
    first = true;
    return MENU_GAME_LOAD;
  }
  if ((start[2].clicked()) || (key[KEY_ESC]))
  {
    first = true;
    return QUIT;
  }
  if (start[3].clicked())
  {
    first = true;
    return MENU_GAME_OPTIONS;
  }
  if (start[4].clicked())
  {
    first = true;
    return MAPEDIT;
  }
  if (start[5].clicked())
  {
    first = true;
    return CREDITS;
  }
	if (start[6].clicked())
	{
		first = true;
		return HELPSCREEN;
	}
	if ((global.networking) && (start[7].clicked()))
	{
		global.netgame = true;
		first = true;
		return NETWORK;
	}
	
	draw_mouse();
  
  if (firstloop)
  {
  	empdebug("---Successfully finished first loop of main menu---");
  	firstloop = false;
  }
  
  return MENU_FIRST;
}


int do_load()
{
	static bool first = true, changemap = false;
	static int oldsel, prevmap, colors[4];
	static _button team, plyrcycle[4], teamcycle[4], colorcycle[4], fundsperbuilding, showdamage,
									timelimit, back, mapchange, mapok, mapcancel, start, techlevel, startingcash,cashturns;
	static _listbox mapbox;
	static bool aremaps = false;
	int i, j, k, l;
	char c[150];
	string s;
	
	if (first)
	{
		play_song(battlemenu_music);
		
		empdebug("Loading the list of maps...");
		refresh_maplist();
		empdebug("  ...done");
		
		mapchange.init(348, 525, 170, 40, "Change map");
		mapbox.init(348, 410, 170, 160);
		mapbox.setsel(0);
		prevmap = 0;
		
		i = 0;
		while (unsigned(i) < maplist.size())
		{
			mapbox.additem(maplist[i].c_str());
			if (maplist[i] == global.lastmap)
			{
				mapbox.setsel(i);
				prevmap = i;
			}
			i++;
		}
		if (maplist.size() == 0)
		{
			aremaps = false;
			mapbox.additem("NO MAPS FOUND!");
		}
		else
		{
			aremaps = true;
			s = mapbox.selected();
			s.erase(0, 5); //get rid of the (2P) / (3P) / (4P)
			sprintf(c, "maps/%s.map", s.c_str());
			load_map(mapbox.selected(), c);
			minimap.create(true);
			minimap.a = 1.0;
			oldsel = mapbox.getsel();
		}
		
		gameoptions.reset();
		i = 0;
		oldsel = -1;
		changemap = false;
		
		i = 0;
		while (i < 4)
		{
			player[i].team = i + 1;
			teamcycle[i].init(150, 70 + (116 * i), 40, 40, "", glb.bmp(14 + i));

			j = i;
			if (unsigned(j) >= character.size()) j -= character.size();  //in case playable characters < numplayers
			player[i].number = j;
			
			plyrcycle[i].init(43, 43 + (116 * i), 80, 80, "", character[player[i].number]->picture);
			player[i].tintcolor = player_color(player[i].number, 0);
			k = 0;
			l = 0;
			while (k < i)
			{
				if (player[k].tintcolor == player[i].tintcolor)
				{
					l++;
					if (l > 3) l = 0;
					player[i].tintcolor = player_color(player[i].number, l);
				}
				k++;
			}
			colors[i] = l;
			colorcycle[i].init(275, 28 + (116 * i), 40, 40, "", NULL, 2, player[i].tintcolor);
			currentcharacter[i] = character[player[i].number];
			opt[i].init(137, 32 + (116 * i), 130, 15, "Human");
			
			i++;
		}
		
		mapok.init(348, 575, 80, 21, "OK");
		mapcancel.init(438, 575, 80, 21, "Cancel");
		changemap = false;
		
		back.init(10, 575, 100, 20, "Back");
		start.init(695, 570, 100, 25, "  Start!", 1, BLACK);
		startingcash.init(348, 389, 170, 20, "Starting Cash: 0");
		fundsperbuilding.init(348, 410, 170, 20, "Funds per Building: 4000");
		showdamage.init(348, 431, 170, 20, "Show Attacking Damage: Yes");
		timelimit.init(348, 452, 170, 20, "Turn Time Limit: None");
		techlevel.init(348, 473, 170, 20, "Tech Level: 3");
		var.techlevel = 3;
		cashturns.init(348, 494, 170, 20, "Derricks Last: 10 Turns");
		var.cashturns = 10;
		
		first = false;
	}
	
	glb.draw(36, 0, 0);
	glb.draw(52, 339, 29);
	back.check();
	
	buffer_rect(start.x - 3, start.y - 3, start.x + start.w + 3, start.y + start.h + 3, BLACK);
	buffer_rect(start.x, start.y, start.x + start.w, start.y + start.h, BLACK);
	
	if (changemap)
	{
		mapbox.check();
		mapok.check();
		mapcancel.check();
		if (mapok.clicked())
		{
			prevmap = mapbox.getsel();
			changemap = false;
		}
		if (mapcancel.clicked())
		{
			mapbox.setsel(prevmap);
			changemap = false;
		}
	}
	else
	{
		if (aremaps)
		{
			start.check();
			if (start.clicked())
			{
				global.lastmap = mapbox.selected();;
				
				sprintf(c, "Starting game:\n  %d players\n", numplayers);
				empdebug(c);
				sprintf(c, "  Selected map: %s\n", mapbox.selected());
				empdebug(c);
				
				first = true;
				i = 0;
				while (i < 4)  //set whether players 3 and four are actually in the game
				{
					if (i < numplayers)
					{
						if (opt[i].get_text() == "Human")
						{
							player[i].controller = HUMAN;
						}
						else
						{
							player[i].controller = AI;
						}
						
						if (player[i].number == -1)
						{  //random army
							player[i].number = rand()%(character.size());
							currentcharacter[i] = character[player[i].number];
							player[i].tintcolor = player_color(player[i].number, 0);
							k = 0;
							l = 0;
							while (k < 4)
							{
								if ((k < numplayers) && (i != k))
								{
									if (player[k].tintcolor == player[i].tintcolor)
									{
										l++;
										if (l > 3) l = 0;
										k = -1;
										player[i].tintcolor = player_color(player[i].number, l);
									}
								}
								k++;
								colors[i] = l;
							}
						}
						
						player[i].reapply_unitstats();  //apply army-specific stats now
					}
					else
					{
						player[i].playing = 0;
						player[i].everplaying = 0;
					}
					i++;
				}
				create_sidebars();
				pturn = 0;  //make player 1 start first
				global.loaded_game = 0;  //the game was not loaded from a saved file
				global.very_first_turn = 1;
				worldmap.scroll_x = 0;
				worldmap.scroll_y = 0;
				
				s = mapbox.selected();
				s.erase(0, 5); //get rid of the (2P) / (3P) / (4P)
				load_ai(s);
				get_connected_beaches();
				get_teleports();
				
				clear_particles();
				return LOGIC;
			}
		}
		mapchange.check();
		startingcash.check();
		fundsperbuilding.check();
		showdamage.check();
		timelimit.check();
		techlevel.check();
		cashturns.check();
		if (mapchange.clicked())
		{
			changemap = true;
		}
		
		if (startingcash.clicked())
		{
			player[0].cash += 5000;
			if (player[0].cash > 50000) player[0].cash = 0;
			i = 1;
			while (i < 4)
			{
				player[i].cash = player[0].cash;
				i++;
			}
			sprintf(c, "Starting Cash: %d", player[0].cash);
			startingcash.init(348, 389, 170, 20, c);
		}
		if (startingcash.rclicked())
		{
			player[0].cash -= 5000;
			if (player[0].cash < 0) player[0].cash = 50000;
			i = 1;
			while (i < 4)
			{
				player[i].cash = player[0].cash;
				i++;
			}
			sprintf(c, "Starting Cash: %d", player[0].cash);
			startingcash.init(348, 389, 170, 20, c);
		}
		
		if (fundsperbuilding.clicked())
		{
			gameoptions.funding += 1000;
			if (gameoptions.funding > 10000)
			{
				gameoptions.funding = 1000;
			}
			sprintf(c, "Funds per Building: %-1d", gameoptions.funding);
			fundsperbuilding.init(348, 410, 170, 20, c);
		}
		if (fundsperbuilding.rclicked())
		{
			gameoptions.funding -= 1000;
			if (gameoptions.funding < 1000) gameoptions.funding = 10000;
			sprintf(c, "Funds per Building: %-1d", gameoptions.funding);
			fundsperbuilding.init(348, 410, 170, 20, c);
		}
		
		if (showdamage.clicked() || showdamage.rclicked())
		{
			if (gameoptions.show_damage)
			{
				gameoptions.show_damage = false;
				showdamage.init(348, 431, 170, 20, "Show Attacking Damage: No");
			}
			else
			{
				gameoptions.show_damage = true;
				showdamage.init(348, 431, 170, 20, "Show Attacking Damage: Yes");
			}
		}
		
		if (timelimit.clicked())
		{
			if (gameoptions.time == -999)
			{
				gameoptions.time = 1000;
			}
			else
			{
				gameoptions.time += 1000;
				if (gameoptions.time > 15000)
				{
					gameoptions.time = -999;
				}
			}
			if (gameoptions.time == -999)
			{
				timelimit.init(348, 452, 170, 20, "Turn Time Limit: None");
			}
			else
			{
				sprintf(c, "Turn Time Limit: %-1d seconds", gameoptions.time / 50);
				timelimit.init(348, 452, 170, 20, c);
			}
		}
		if (timelimit.rclicked())
		{
			if (gameoptions.time == -999)
			{
				gameoptions.time = 15000;
			}
			else
			{
				gameoptions.time -= 1000;
				if (gameoptions.time < 1000)
				{
					gameoptions.time = -999;
				}
			}
			if (gameoptions.time == -999)
			{
				timelimit.init(348, 452, 170, 20, "Turn Time Limit: None");
			}
			else
			{
				sprintf(c, "Turn Time Limit: %-1d seconds", gameoptions.time / 50);
				timelimit.init(348, 452, 170, 20, c);
			}
		}
		
		if (techlevel.clicked())
		{
			var.techlevel++;
			if (var.techlevel > 3) var.techlevel = 1;
			sprintf(c, "Tech Level: %d", var.techlevel);
			techlevel.init(348, 473, 170, 20, c);
		}
		if (techlevel.rclicked())
		{
			var.techlevel--;
			if (var.techlevel < 1) var.techlevel = 3;
			sprintf(c, "Tech Level: %d", var.techlevel);
			techlevel.init(348, 473, 170, 20, c);
		}
		
		if (cashturns.clicked())
		{
			var.cashturns += 5;
			if (var.cashturns > 30) var.cashturns = 0;
			
			if (var.cashturns == 0)
			{
				cashturns.init(348, 494, 170, 20, "Derricks Last: Forever");
			}
			else
			{
				sprintf(c, "Derricks Last: %d Turns", var.cashturns);
				cashturns.init(348, 494, 170, 20, c);
			}
		}
		if (cashturns.rclicked())
		{
			var.cashturns -= 5;
			if (var.cashturns < 0) var.cashturns = 30;
			
			if (var.cashturns == 0)
			{
				cashturns.init(348, 494, 170, 20, "Derricks Last: Forever");
			}
			else
			{
				sprintf(c, "Derricks Last: %d Turns", var.cashturns);
				cashturns.init(348, 494, 170, 20, c);
			}
		}
	}
	if (oldsel != mapbox.getsel())
	{
		s = mapbox.selected();
		s.erase(0, 5); //get rid of the (2P) / (3P) / (4P)
		sprintf(c, "maps/%s.map", s.c_str());
		load_map(mapbox.selected(), c, false);
		minimap.create(true);
		minimap.a = 1.0;
		oldsel = mapbox.getsel();
	}
	
	i = 0;
	while ((i < 4) && (i < numplayers))
	{
		buffer_rectfill(36, 36 + (116 * i), 127, 127 + (116 * i), WHITE);
		plyrcycle[i].check();
		buffer_textout_ex(med, "Team", 156, 55 + 116 * i, WHITE);
		buffer_textout_ex(med, "Team", 158, 55 + 116 * i, WHITE);
		buffer_textout_ex(med, "Team", 156, 57 + 116 * i, WHITE);
		buffer_textout_ex(med, "Team", 158, 57 + 116 * i, WHITE);
		buffer_textout_ex(med, "Team", 157, 56 + 116 * i, BLACK);
		teamcycle[i].check();
		if (player[i].number != -1) colorcycle[i].check();
		opt[i].check();
		buffer_rect(38, 36 + (116 * i), 128, 126 + (116 * i), BLACK);
		
		if (teamcycle[i].clicked())
		{
			player[i].team++;
			if (player[i].team > 4) player[i].team = 1;
			teamcycle[i].init(150, 70 + (116 * i), 40, 40, "", glb.bmp(13 + player[i].team));
		}
		if (teamcycle[i].rclicked())
		{
			player[i].team--;
			if (player[i].team < 1) player[i].team = 4;
			teamcycle[i].init(150, 70 + (116 * i), 40, 40, "", glb.bmp(13 + player[i].team));
		}
		
		if (opt[i].clicked() || opt[i].rclicked())
		{
			if (opt[i].get_text() == "Human")
			{
				opt[i].init(137, 32 + (116 * i), 130, 15, "AI");
			}
			else if (opt[i].get_text() == "AI")
			{
				opt[i].init(137, 32 + (116 * i), 130, 15, "Human");
			}
		}
		
		if (plyrcycle[i].clicked())
		{
			j = player[i].number + 1;
			if (unsigned(j) >= character.size()) j = -1;
			player[i].number = j;
			
			if (j != -1)
			{
				currentcharacter[i] = character[player[i].number];
				plyrcycle[i].init(43, 43 + (116 * i), 80, 80, "", currentcharacter[i]->picture);
				assign_tintcolor(i, colors);
			}
			else
			{
				currentcharacter[i] = NULL;
				plyrcycle[i].init(43, 43 + (116 * i), 80, 80, "", glb.bmp(51));
				colors[i] = -1;
				player[i].tintcolor = WHITE;
			}
			colorcycle[i].init(275, 28 + (116 * i), 40, 40, "", NULL, 2, player[i].tintcolor);
		}
		if (plyrcycle[i].rclicked())
		{
			j = player[i].number - 1;
			if (j < -1) j = character.size() - 1;
			player[i].number = j;
			
			if (j != -1)
			{
				currentcharacter[i] = character[player[i].number];
				plyrcycle[i].init(43, 43 + (116 * i), 80, 80, "", currentcharacter[i]->picture);
				assign_tintcolor(i, colors);
			}
			else
			{
				currentcharacter[i] = NULL;
				plyrcycle[i].init(43, 43 + (116 * i), 80, 80, "", glb.bmp(51));
				colors[i] = -1;
				player[i].tintcolor = WHITE;
			}
			colorcycle[i].init(275, 28 + (116 * i), 40, 40, "", NULL, 2, player[i].tintcolor);
		}
		if (plyrcycle[i].mouseover())
		{
			buffer_rectrounded(mouse_x + 10, mouse_y + 10, mouse_x + 290, mouse_y + 40, BLACK, 0.6);
			description(player[i].number, mouse_x + 20, mouse_y + 15, false);
		}
		
		if ((player[i].number != -1) && (colorcycle[i].clicked()))
		{
			colors[i]++;
			if (colors[i] > 3) colors[i] = 0;
			assign_tintcolor(i, colors);
			colorcycle[i].init(275, 28 + (116 * i), 40, 40, "", NULL, 2, player[i].tintcolor);
		}
		if ((player[i].number != -1) && (colorcycle[i].rclicked()))
		{
			colors[i]--;
			if (colors[i] < 0) colors[i] = 3;
			assign_tintcolor(i, colors);
			colorcycle[i].init(275, 28 + (116 * i), 40, 40, "", NULL, 2, player[i].tintcolor);
		}
		
		i++;
	}
	
	if (back.clicked())
	{
		first = true;
		play_song(title_music);
		return MENU_FIRST;
	}
	
	buffer_textout_ex(normal, worldmap.get_name(), 620, 384, BLACK);
	sprintf(c, "%d Players", numplayers);
	buffer_textout_ex(normal, c, 620, 397, BLACK);
	minimap.draw(false, 619, 412, false);
	
	draw_mouse();
	
	return MENU_LOAD;
}

void description(int c, int x, int y, bool centeredtext)
{
	if ((c > -1) && (c < signed(character.size())))
	{
		description(character[c], x, y, centeredtext);
	}
	else if (c == -1)
	{
		if (centeredtext) buffer_textout_centre_ex(normal, "Random army", x, y, WHITE);
		else buffer_textout_ex(normal, "Random army", x, y, WHITE);
	}
}

void description(_character *c, int x, int y, bool centeredtext)
{
  string n, m;

	n = c->getname();
  m = c->getdescription();

	if (centeredtext)
	{
  	buffer_textout_centre_ex(normal, n.c_str(), x, y, WHITE);
  	buffer_textout_centre_ex(normal, m.c_str(), x, y + 11, WHITE);
	}
	else
	{
		buffer_textout_ex(normal, n.c_str(), x, y, WHITE);
  	buffer_textout_ex(normal, m.c_str(), x, y + 11, WHITE);
	}
}

void list_players(int numplayers)
{
  int i = 0;
  char m[15];
  while (i < numplayers)
  {
    sprintf(m, "Player %-1d", i + 1);
    //buffer_rectfill_gradient(0, 25 + (90 * i), 155, 75 + (90 * i), player_color(i), BLACK, BLACK, player_color(i));
    buffer_textout_ex(big, m, 30, 36 + (90 * i), WHITE);
    i++;
  }
}

int do_gameload()
{
  int i, o, p, num, safety;
  int type, tilex, tiley, ready, unum;
  float health;
  int thechoice = -1;
  int temp;
  char text[30] = "";
  char d[30];
  string mname;
  string ainame;
  string s;
  static int first = 1;
  static _button choice[10];
  static _button delsave[9];
  char path[30];
  
  if (first == 1)
  {
    o = 0;
    while (o < 9)
    {
      sprintf(path, "saves/game%-1d.sav", o);
      if (exists(path) != 0)
			{
			  ifstream n(path);
        if (n.bad())
        {
          choice[o].init(260, 80 + (o * 40), 140, 25, "Empty", 5);
        }
        else
        {
          n >> text;
          choice[o].init(260, 80 + (o * 40), 140, 25, text, 5);
        }
				n.close();
			}
			else
			{
				choice[o].init(260, 80 + (o * 40), 140, 25, "Empty", 5);
			}
      delsave[o].init(425, 82 + (o * 40), 80, 20, "Delete", 5);
      o++;
    }
    choice[9].init(10, 575, 100, 20, "Back");
    gameoptions.reset();
    first = 0;
  }
  glb.draw(8, 0, 0);
  glb.draw(9, 0, 0);
  i = 0;
  while (i < 10)
  {
    choice[i].check();
    if ((choice[i].clicked()) && (choice[i].get_text() != "Empty"))
    {
      thechoice = i;
    }
    if (i < 9)
    {
      delsave[i].check();
      if (delsave[i].clicked())
      {  //if the delete option was chosen for this save file
        sprintf(path, "saves/game%-1d.sav", i);
        delete_file(path);
        choice[i].init(260, 80 + (i * 40), 140, 25, "Empty", 5);
      }
    }
    i++;
  }
  if (((thechoice != -1) && (thechoice != 9)) || (key[KEY_Z]))  //Z = LAST DEBUG SAVE
  {
    if ((thechoice != -1) && (thechoice != 9))
    {
      sprintf(path, "saves/game%-1d.sav", thechoice);
    }
    else
    {
      sprintf(path, "saves/gamedebug.sav");
    }
    ifstream savefile(path);
    if (!(savefile.bad()))
    {
      worldmap.reset();
      savefile >> d >> worldmap.l >> worldmap.h;
      o = 0;
      while (o < worldmap.h)
      {
        p = 0;
        while (p < worldmap.l)
        {
        	worldmap.tile[p][o].x = p;
        	worldmap.tile[p][o].y = o;
          savefile >> temp;
          worldmap.tile[p][o].change_type(temp / 100, temp % 100);
          savefile >> temp;
          worldmap.tile[p][o].change_owner(temp);
          savefile >> temp;
          worldmap.tile[p][o].burning = (temp != 0);
          savefile >> temp;
          worldmap.tile[p][o].burnedfor = temp;
          savefile >> temp;
          worldmap.tile[p][o].turnsused = temp;
          p++;
        }
        o++;
      }
      safety = 0;
      o = 0;
      while (o < 4)
      {
        player[o].clear_units();
        savefile >> d >> player[o].playing;
        savefile >> d >> player[o].everplaying;
        if (player[o].everplaying == 1)
        {
					numplayers = o + 1;
          savefile >> d >> player[o].number;
          currentcharacter[o] = character[player[o].number];
          savefile >> d >> player[o].cash;
          savefile >> d >> player[o].controller;
          savefile >> d >> unum;
          savefile >> d >> player[o].team;
          savefile >> d >> player[o].tookturn;
					savefile >> d >> player[o].tintcolor;
					
					savefile >> d >> player[o].stats.raised;
					savefile >> d >> player[o].stats.spent;
					savefile >> d >> player[o].stats.produced;
					savefile >> d >> player[o].stats.killed;
					savefile >> d >> player[o].stats.lost;
          p = 0;
          while (p < unum)
          {
            savefile >> d >> type;
            savefile >> d >> tilex;
            savefile >> d >> tiley;
            savefile >> d >> health;
            savefile >> d >> ready;
            num = player[o].create_unit(type, tilex, tiley, o);
            player[o].unit[num].health = health;
            player[o].unit[num].ready = ready;
            savefile >> d >> player[o].unit[num].attackdelay;
            savefile >> d >> player[o].unit[num].canmove;
            savefile >> d >> player[o].unit[num].load[0].loaded;
            savefile >> d >> player[o].unit[num].load[0].type;
            savefile >> d >> player[o].unit[num].load[0].health;
            savefile >> d >> player[o].unit[num].load[0].subload[0].loaded;
            savefile >> d >> player[o].unit[num].load[0].subload[0].type;
            savefile >> d >> player[o].unit[num].load[0].subload[0].health;
            savefile >> d >> player[o].unit[num].load[0].subload[1].loaded;
            savefile >> d >> player[o].unit[num].load[0].subload[1].type;
            savefile >> d >> player[o].unit[num].load[0].subload[1].health;
            savefile >> d >> player[o].unit[num].load[1].loaded;
            savefile >> d >> player[o].unit[num].load[1].type;
            savefile >> d >> player[o].unit[num].load[1].health;
            savefile >> d >> player[o].unit[num].load[1].subload[0].loaded;
            savefile >> d >> player[o].unit[num].load[1].subload[0].type;
            savefile >> d >> player[o].unit[num].load[1].subload[0].health;
            savefile >> d >> player[o].unit[num].load[1].subload[1].loaded;
            savefile >> d >> player[o].unit[num].load[1].subload[1].type;
            savefile >> d >> player[o].unit[num].load[1].subload[1].health;
            p++;
          }
        }
        o++;
      }
      savefile >> d >> pturn >> gameoptions.funding;
      savefile >> gameoptions.show_damage >> gameoptions.time >> loaded_time >> var.techlevel >> var.cashturns;
      getline(savefile, mname);  //this first one makes mname empty for some reason
      getline(savefile, mname);  //so I have to call it again to actually get the line
      worldmap.set_name(mname);
      s = mname;
			s.erase(0, 5); //get rid of the (2P) / (3P) / (4P)
      load_ai(s);
			create_sidebars();
      global.loaded_game = 1;  //the game has been loaded from a file
      first = 1;
      get_connected_beaches();
			get_teleports();
      global.very_first_turn = 1;
      minimap.create();
      clear_particles();
      counter = 0;
      return LOGIC;
    }
  }
  else if (thechoice == 9)
  {
    first = 1;
    return MENU_FIRST;
  }
  draw_mouse();
  return MENU_GAME_LOAD;
}

void get_connected_beaches()
{
  int x, y;
  _loc d;
  _connectedlocs t;
  beaches.clear();
  beaches.reserve(5);
  y = 0;
  while (y < worldmap.h)
  {
    x = 0;
    while (x < worldmap.l)
    {
      if (is_beach(x, y) == 1)
      {
        if (beach_already_listed(x, y) == 0)
        {
          d.x = x;
          d.y = y;
          beaches.push_back(t);
          beaches[beaches.size() - 1].loc.reserve(10);
          beaches[beaches.size() - 1].loc.push_back(d);
          add_surrounding_beaches(beaches.size() - 1, x, y);
        }
      }
      x++;
    }
    y++;
  }
}

void add_surrounding_beaches(int bnum, int x, int y)
{
  _loc d;
  if (x - 1 > -1)
  {
    if (is_beach(x - 1, y) == 1)
    {
      if (beach_already_listed(x - 1, y) == 0)
      {
        d.x = x - 1;
        d.y = y;
        beaches[bnum].loc.push_back(d);
        add_surrounding_beaches(bnum, x - 1, y);
      }
    }
  }
  if (x + 1 < worldmap.l)
  {
    if (is_beach(x + 1, y) == 1)
    {
      if (beach_already_listed(x + 1, y) == 0)
      {
        d.x = x + 1;
        d.y = y;
        beaches[bnum].loc.push_back(d);
        add_surrounding_beaches(bnum, x + 1, y);
      }
    }
  }
  if (y - 1 > -1)
  {
    if (is_beach(x, y - 1) == 1)
    {
      if (beach_already_listed(x, y - 1) == 0)
      {
        d.x = x;
        d.y = y - 1;
        beaches[bnum].loc.push_back(d);
        add_surrounding_beaches(bnum, x, y - 1);
      }
    }
  }
  if (y + 1 < worldmap.h)
  {
    if (is_beach(x, y + 1) == 1)
    {
      if (beach_already_listed(x, y + 1) == 0)
      {
        d.x = x;
        d.y = y + 1;
        beaches[bnum].loc.push_back(d);
        add_surrounding_beaches(bnum, x, y + 1);
      }
    }
  }  
}

int is_beach(int x, int y)
{
  if (worldmap.tile[x][y].get_set() == 3)
  {  //3 is the set for beaches (called shoals in mapedit)
    if (worldmap.tile[x][y].get_number() < 25)  //shoal tiles 25-28 are actually sea transitions
    {
      return 1;
    }
  }
  if (worldmap.tile[x][y].is_building(PORT))
  {
    return 1;
  }
  return 0;
}

int beach_already_listed(int x, int y)
{
  unsigned int i, j;
  _loc d;
  d.x = x;
  d.y = y;
  i = 0;
  while (i < beaches.size())
  {
    j = 0;
    while (j < beaches[i].loc.size())
    {
      if ((d.x == beaches[i].loc[j].x) && (d.y == beaches[i].loc[j].y))
      {
        return 1;
      }
      j++;
    }
    i++;
  }
  return 0;
}

void get_teleports()
{
	int i, j;
	_loc l;
	teleports.clear();
	
	j = 0;
	while (j < worldmap.h)
	{
		i = 0;
		while (i < worldmap.l)
		{
			if (worldmap.tile[i][j].same_type(6, 0))
			{
				l.x = i;
				l.y = j;
				teleports.push_back(l);
			}
			i++;
		}
		j++;
	}
}

void assign_tintcolor(int p, int *colors)
{
	int k;
	
	if (colors[p] > 3) colors[p] = 0;
	player[p].tintcolor = player_color(player[p].number, colors[p]);
	
	k = 0;
	while (k < numplayers)
	{
		if (p != k)
		{
			if (player[k].tintcolor == player[p].tintcolor)
			{
				colors[p]++;
				if (colors[p] > 3) colors[p] = 0;
				k = -1;
				player[p].tintcolor = player_color(player[p].number, colors[p]);
			}
		}
		k++;
	}
}
