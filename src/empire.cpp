/*
 _____    _      _                         _
|_   _|  (_)    | |                       (_)
  | |_ __ _  ___| |__  _ __ ___  _ __ ___  _  ___
  | | '__| |/ __| '_ \| '__/ _ \| '_ ` _ \| |/ __|
  | | |  | | (__| | | | | | (_) | | | | | | | (__
  \_/_|  |_|\___|_| |_|_|  \___/|_| |_| |_|_|\___|
  BY KENT DEVILLAFRANCA

(changed mid-project from "Empire Allegro 2", which explains all the "empire" stuff)
*/

#include "empire.h"
#define DEBUGGING 1

volatile int counter;

_globalsettings global;
_logic_variables var;
_character *currentcharacter[4];
//_character *loadedcharacter[4];
vector<_character *> character;
//vector<ol::Bitmap *> avatars;
_player player[4];
_map worldmap;
//_tile tile;
_minimap minimap;

extern void reset_mouse_counter();
extern void check_mouse_stuff();

//int first_mission_character;  //the first "mission-only" AI character
int NUM_UNITS;

bitmapdat land, river, sea, shoal, buildings, special;
bitmapdat mouse;
bitmapdat glb;
bitmapdat greystatbmp, statbmp, sidebarbmp;
bitmapdat explode1, nukeboom;
bitmapdat units[MAX_UNITS], bigunits;

ol::Bitmap *transition;
//ol::Bitmap **mapunits;

ol::TextRenderer normal;
ol::TextRenderer med;
ol::TextRenderer big;

_armystats armystats[3];

string title_music;
string ingame_music[3];
string victory_music;
string battlemenu_music;

void load_global_settings();
void load_army_settings();
void load_unit_settings();
void init_tile_animations();
int load_characters();

int intro();
int logic();
int menu();
int mapedit();
//int playeredit();
int show_winner();
int helpscreen();
int credits();
int do_transition(int target);
//int player_color(int c);
int player_color(int army, int tintnum);
void save_debug_info();
void save_pathmap_info();
void save_unitmap_info();
void save_ownermap_info();
//void save_tile_healthmap();
void empdebug(string text);
void empdebug(int i);
void empdebug(float f);
string init_everything();

string unit_name(int type);
int unit_type(string name);

void timer_handler()
{
  counter++;
}
END_OF_FUNCTION(timer_handler);

void close_button_handler(void)
{
	global.gamestatus = QUIT;
}
END_OF_FUNCTION(close_button_handler)

_init_variables init;


int main()
{
  int j;
  unsigned int i;
  bool draw;
  i = 0;
  
	title_music = "data/title.ogg";
	ingame_music[0] = "data/crimson.ogg";
	ingame_music[1] = "data/cobalt.ogg";
	ingame_music[2] = "data/pine.ogg";
	battlemenu_music = "data/battlemenu.ogg";
	victory_music = "data/victory.ogg";
  
  empdebug("-CLEAR-");
  empdebug(""); empdebug("Starting Trichromic...");
  empdebug("Setting up all the stuff...");
	
	#ifdef ALLEGRO_LINUX
		XInitThreads();  //without this, moving the window will crash on Intel i810 systems, and probably other chipsets too
  #endif
	
	string s = init_everything();
  if (s != "-")
  {
    set_gfx_mode(GFX_TEXT, 0, 0, 0, 0);
    empdebug(s);
    return 1;
  }
  
  global.gamestatus = GAMEINTRO;
  
  drawqueue.reserve(500);
  
  counter = 0;
  worldmap.reset();
  var.first = true;
  draw = true;
  
  empdebug("Finished doing the initial setup and loading, going into the actual game...");
  
  while (global.gamestatus != QUIT)
  {

		while (counter > 0)
		{
			reset_mouse_counter();
			check_mouse_stuff();
			
			if (global.sound)
			{
				FMOD_System_Update(fmodmain);
			  ensure_music_is_playing();
			}
			
      drawqueue.clear();
      buffer_clear_to_color(BLACK);
      if ((key[KEY_LCONTROL]) && (key[KEY_B]))
      {
        global.gamestatus = QUIT;
      }
      switch(global.gamestatus)
      {
      	case GAMEINTRO:
      		global.gamestatus = intro();
      		break;
        case MENU:
          global.gamestatus = menu();
          break;
        case LOGIC:
          global.gamestatus = logic();
          break;
        case GAMEOVER:
          global.gamestatus = show_winner();
          break;
        case MAPEDIT:
          global.gamestatus = mapedit();
          break;
        case HELPSCREEN:
        	global.gamestatus = helpscreen();
        	break;
				case CREDITS:
					global.gamestatus = credits();
					break;
        /*case PLAYEREDIT:
          global.gamestatus = playeredit();
          break;
        case MISSION:
          global.gamestatus = mission();
          break;*/
				case NETWORK:
					global.gamestatus = network();
					if (global.gamestatus != NETWORK)
					{  //if exiting from the networking menu, you may have played a netgame, and may have had the ini data changed
						load_army_settings();
						load_unit_settings();  //so make sure the ini data is set back to what it was originally
					}
					break;
        /*case TRANSITION:
          global.gamestatus = do_transition(transitionto);
          break;*/
      }
      draw = true;
			/*if ((global.gamestatus != oldstatus) && (oldstatus != TRANSITION))
      {
        if ((oldstatus != GAMEINTRO) && (global.gamestatus != QUIT))
        {
          transitionto = global.gamestatus;
          if (transitionto == TRANSITION)  //if the gamestatus was set to TRANSITION
          {
            transitionto = MENU;  //it was from a change in the menu
          }
          draw_all();
          copy_screen_to(transition);
          draw = false;
          global.gamestatus = TRANSITION;
        }
      }*/
      counter--;
    }
    if (draw)
    {
      draw_all();
      draw = false;
    }

    if (key[KEY_LCONTROL] && key[KEY_D] && (global.gamestatus == LOGIC))
    {  //save a metric crapload of debug info
      save_debug_info();
      save_pathmap_info();
      save_unitmap_info();
			save_ownermap_info();
      //save_tile_healthmap();
    }
		if (key[KEY_F12])
		{  //save a screenshot
			ol::Canvas::Save("screenshot.png");
			counter = 0;
		}
		
		if (counter < 1)
    	rest(1);
  }
  
  ol::Settings::RestoreOldProjection();
	buffer_clear_to_color(BLACK);
	buffer_textout_ex(big, "Goodbye", 5, 5, WHITE);
	draw_all();
	
	empdebug("Game over, exiting...");
	empdebug("Saving settings...");
  
	set_config_int("empiresettings", "mousespeed", global.mspeed);
	set_config_int("empiresettings", "sfxvolume", global.volume);
	set_config_int("empiresettings", "musicvolume", global.music_volume);
	set_config_int("empiresettings", "fullscreen", global.fullscreen);
	set_config_int("empiresettings", "particles", global.particles);
	set_config_int("empiresettings", "maporder", global.maporder);
	set_config_int("empiresettings", "windowedres", global.windowed_resolution);
	set_config_int("empiresettings", "scrolldivider", global.scroll_divider);
	set_config_int("empiresettings", "AIscrolldivider", global.AI_scroll_divider);
	set_config_int("empiresettings", "battleanimation", global.battle_animation);
	set_config_string("empiresettings", "netname", global.netname.c_str());
	set_config_string("empiresettings", "lastmap", global.lastmap.c_str());
 	
  empdebug("Unloading army bitmaps...");
	
	i = 0;
	while (i < character.size())
	{
		character[i]->destroy();
		delete(character[i]);
		i++;
	}
	
	shutdown_sound();
	empdebug("Shutting down networking...");
	stop_networking();
	empdebug("Done.");
	
	multiline_textout_ex(normal, "/GAMEOVER/", 0, 0, 0, 0);  //frees malloc'd memory used in static char*s
	
	empdebug("Unloading graphics...");
	empdebug("  minimap");
  minimap.destroy();
  empdebug("  land");
  land.destroy();
  empdebug("  river");
  river.destroy();
  empdebug("  sea");
  sea.destroy();
  empdebug("  shoal");
  shoal.destroy();
  empdebug("  buildings");
  buildings.destroy();
  empdebug("  special");
  special.destroy();
  empdebug("  mouse");
  mouse.destroy();
  empdebug("  misc");
  glb.destroy();
  empdebug("  stat");
	statbmp.destroy();
	empdebug("  sidebar");
	sidebarbmp.destroy();
	empdebug("  explosions");
  explode1.destroy();
  empdebug("Done.");
	empdebug("Unloading unit data...");
  j = 0;
  while (j < NUM_UNITS)
  {
    units[j].destroy();
    j++;
  }
	//free(mapunits);
	empdebug("...done");
  empdebug("Finished unloading everything, exiting...");
  allegro_exit();
  empdebug("Done.");
	exit(0);
  return 0;
}
END_OF_MAIN()



string init_everything()
{
  bool goodmode = false;
  bool fullscreen = false;
  int i, j, z;
  int desktop_w, desktop_h;
	
  srand(time(NULL));
  LOCK_VARIABLE(counter);
  LOCK_FUNCTION(timer_handler);
	LOCK_FUNCTION(close_button_handler);
	
	//Settings::StoreMemoryBitmaps(true);
	
  empdebug("  Setup::SetupProgram()...");
	if (!ol::Setup::SetupProgram())
  {
    return "Couldn't set up the program for some reason.";
  }
	empdebug("    ...done");
	
	set_close_button_callback(close_button_handler);
	set_window_title("Trichromic");	
	
	set_config_file("data/empire.ini");
	empdebug("  loading settings...");
	load_global_settings();
	
	if (global.fullscreen == 0)
	{
		if (get_desktop_resolution(&desktop_w, &desktop_h) == 0)  // 0 means no error getting the resolution
		{
			if ((desktop_w < 800) || (desktop_h < 600))
			{
				allegro_message("Your desktop resolution is too low!  It needs to be at least 800x600 to play Trichromic in windowed mode.  If your monitor can do higher resolutions and you want to play the game in fullscreen, open up data/empire.ini and change fullscreen to 0.");
				return "Error: Desktop resolution too low, exiting.";
			}
		}
	}
	
	load_army_settings();
	load_unit_settings();
	empdebug("    ...done");
	
  if (global.fullscreen == 1)
  {
    fullscreen = true;
  }
  
	empdebug("  Setup::SetupScreen()...");
	goodmode = false;
  z = 0;
  while ((!goodmode) && (z < 4))
  {
    if (fullscreen)
    {
      i = 800; j = 600;
    }
    else
    {
      switch(global.windowed_resolution)
      {
        default:
        case 0:
          i = 800; j = 600;
          break;
        case 1:
          i = 1024; j = 768;
          break;
        case 2:
          i = 1280; j = 960;
          break;
      }
    }
    if (!ol::Setup::SetupScreen(i, j, fullscreen))
    {
			if (fullscreen)
			{
				global.fullscreen = false;
				fullscreen = false;
				z--;
				empdebug("Couldn't start using fullscreen!  Switching to windowed.");
			}
			else
			{
				empdebug("Couldn't start using windowed resolution, trying next one...");
        global.windowed_resolution++;
        if (global.windowed_resolution > 2) global.windowed_resolution = 0;
		  }
    }
    else
    {
      goodmode = true;
    }
    z++;
    if (fullscreen) z = 4;
  }
  if (!goodmode)
  {
    return "Couldn't set up a graphics mode (this game uses OpenGL, so maybe it's your drivers).";
  }
	empdebug("    ...done");
	global.screen_width = i;
	global.screen_height = j;
	if (global.fullscreen)
	{
		global.current_resolution = 0;
	}
	else
	{
		global.current_resolution = global.windowed_resolution;
	}
  ol::Settings::SetOrthographicProjection(800, 600);
	
	glPointSize(2);
	
	//gfx_driver->w = 800;  //these two lines are so OpenLayer/Glyph Keeper
  //gfx_driver->h = 600;  //gets the text size right
	
	if (global.current_resolution == 1)
	{
		normal.Load("data/rock.ttf", 15, 13, ol::Rgba(0, 0, 0, 255));
		med.Load("data/rock.ttf", 15, 18, ol::Rgba(0, 0, 0, 255));
		big.Load("data/rock.ttf", 26, 26, ol::Rgba(0, 0, 0, 255));
	}
	else if (global.current_resolution == 2)
	{
		normal.Load("data/rock.ttf", 19, 16, ol::Rgba(0, 0, 0, 255));
		med.Load("data/rock.ttf", 19, 22, ol::Rgba(0, 0, 0, 255));
		big.Load("data/rock.ttf", 32, 32, ol::Rgba(0, 0, 0, 255));
	}
	else
	{
		normal.Load("data/rock.ttf", 12, 10, ol::Rgba(0, 0, 0, 255));
		med.Load("data/rock.ttf", 12, 14, ol::Rgba(0, 0, 0, 255));
		big.Load("data/rock.ttf", 20, 20, ol::Rgba(0, 0, 0, 255));
	}
  
	ol::Canvas::Fill(ol::Rgba(BLACK, 255));
  big.SetColor(ol::Rgba(WHITE, 255));
  big.Print("Loading...", 20, 30);
	ol::Canvas::Refresh();

  install_int_ex(timer_handler, BPS_TO_TIMER(50));
	
	global.sound = init_sound();
  if (!global.sound)
  {
		empdebug("  Couldn't set up FMOD, continuing without music or sound...");
  }
	global.networking = init_networking();
	if (!global.networking)
	{
		empdebug("  Couldn't set up networking, continuing without networked multiplayer...");
	}
	global.returned_from_netgame = false;
  
	if (global.sound)
	{
  	load_sounds();
	}
	//jpgalleg_init();
	
	glb.load("data/graphics/misc");
	glb.name = "misc bitmaps";
	land.load("data/graphics/tiles/land");
	land.name = "land tiles";
	buildings.load("data/graphics/tiles/buildings");
	buildings.name = "building tiles";
	river.load("data/graphics/tiles/river");
	river.name = "river tiles";
	sea.load("data/graphics/tiles/sea");
	sea.name = "sea tiles";
	shoal.load("data/graphics/tiles/shoal");
	shoal.name = "shoal tiles";
	special.load("data/graphics/tiles/special");
	special.name = "special tiles";
	statbmp.load("data/graphics/stats");
	statbmp.name = "stat bitmaps";
	
	init_tile_animations();
	mouse.load("data/graphics/mouse");
	mouse.name = "mouse sprites";
	explode1.load("data/graphics/explosions/explode1");
	explode1.name = "explosion sprites";
	nukeboom.load("data/graphics/explosions/nuke");
	nukeboom.name = "nuke animation";
	bigunits.load("data/graphics/units/big");
	bigunits.name = "big unit bitmaps";
	empdebug("    ...done");	
	//transition = new ol::Bitmap(800, 600);

	char c[50];  
	i = 0;
	while (i < NUM_UNITS)
	{
		sprintf(c, "data/graphics/units/unit%d", i);
		units[i].load(c);  //load all the unit bitmaps
		units[i].name = unitstats[i].name + " sprites";
		i++;
	}
	empdebug("    ...done");
	
  empdebug("  load_characters()...");
  if (load_characters() != 0)
  {
    return "Unable to load data/players.dat or data/missionplayers.dat";
  }
	empdebug("  ...done");
  
  mouse_type = MOUSE_NORMAL;
  return "-";
}

void init_tile_animations()
{
  bitmapdat *tiledats[6] = {&land, &river, &sea, &shoal, &buildings, &special};
  bitmapdat *current;
  int i = 0;
  int j;
  animtiles temp;
  
  while (i < 6)
  {
    current = tiledats[i];
    j = 0;
    while (j < current->size)
    {
      if (current->bmp(j)->Width() >= MAP_TILE_SIZE * 2)
      {
        temp.set = i;
        if (i > 4) temp.set++;  //compensate for the units datafile (which is set 5, but doesn't show up here)
        temp.num = j;
        temp.framenum = current->bmp(j)->Width() / MAP_TILE_SIZE;
        temp.frameinc = 0.1;
        animated_tiles.push_back(temp);
      }
      j++;
    }
    i++;
  }
}




void load_global_settings()
{
	global.sound = true;
	global.mspeed = get_config_int("empiresettings", "mousespeed", 2);
	global.volume = get_config_int("empiresettings", "sfxvolume", 50);
	global.music_volume = get_config_int("empiresettings", "musicvolume", 50);
	global.fullscreen = get_config_int("empiresettings", "fullscreen", 0);
	global.particles = get_config_int("empiresettings", "particles", 1);
	global.maporder = get_config_int("empiresettings", "maporder", 0);
	global.windowed_resolution = get_config_int("empiresettings", "windowedres", 0);
	global.scroll_divider = get_config_int("empiresettings", "scrolldivider", 4);
	global.AI_scroll_divider = get_config_int("empiresettings", "AIscrolldivider", 2);
	global.battle_animation = get_config_int("empiresettings", "battleanimation", 1);
	global.netname = get_config_string("empiresettings", "netname", "Default");
	global.netlobby = get_config_string("empiresettings", "netlobby", "gamecreation.org");
	global.lastmap = get_config_string("empiresettings", "lastmap", "");
	
	if ((global.scroll_divider < 1) || (global.scroll_divider > 16))
	{  //if someone's been messing with the config file stupidly
		global.scroll_divider = 4;  //this will set scroll_divider back to the default
		set_config_int("empiresettings", "scroll_divider", 4);
	}  //(preventing weird scrolling issues and possibly a divide by zero error)
	if ((global.AI_scroll_divider < 1) || (global.AI_scroll_divider > 16))
	{  //if someone's been messing with the config file stupidly
		global.AI_scroll_divider = 2;  //this will set AI_scroll_divider back to the default
		set_config_int("empiresettings", "AI_scroll_divider", 2);
	}  //(preventing weird scrolling issues and possibly a divide by zero error)
}

void load_army_settings()
{
	char c[20];
	int i = 0;
	
	while (i < 3)
	{
		switch(i)
		{
			case 0:
				sprintf(c, "Crimson");
				break;
			case 1:
				sprintf(c, "Cobalt");
				break;
			case 2:
				sprintf(c, "Pine");
				break;
		}
		armystats[i].pricefraction = get_config_int(c, "pricefraction", 100);
		armystats[i].movemod = get_config_int(c, "movemod", 0);
		armystats[i].rangemaxmod = get_config_int(c, "rangemaxmod", 0);
		armystats[i].attackfraction[0] = get_config_int(c, "attackfraction_BULLET", 100);
		armystats[i].attackfraction[1] = get_config_int(c, "attackfraction_FIRE", 100);
		armystats[i].attackfraction[2] = get_config_int(c, "attackfraction_EXPLODE", 100);
		armystats[i].resistfraction[0] = get_config_int(c, "resistfraction_BULLET", 100);
		armystats[i].resistfraction[1] = get_config_int(c, "resistfraction_FIRE", 100);
		armystats[i].resistfraction[2] = get_config_int(c, "resistfraction_EXPLODE", 100);
		i++;
	}
}

void load_unit_settings()
{
	bool nextunit = true;
	char unitstr[15] = "-";
	char str[20];
	vector<string> unitname;
	string n, d;
	int i, j;
	
	NUM_UNITS = 0;
	while ((nextunit) && (NUM_UNITS < MAX_UNITS))
	{
		sprintf(unitstr, "unit%d", NUM_UNITS);
		n = get_config_string("unitnames", unitstr, "-");
		if (n == "-")
		{  //no entry for this unit number
			nextunit = false;
		}
		else
		{
			NUM_UNITS++;
			unitname.push_back(n);
		}
	}
	empdebug(NUM_UNITS);
	
	i = 0;
	while (i < NUM_UNITS)
	{  //most defaults are 0 so that any loading errors can be easily spotted;
		unitstats[i]._move = get_config_int(unitname[i].c_str(), "move", 0);
		unitstats[i].movetype = get_config_int(unitname[i].c_str(), "movetype", 0);
		unitstats[i].attacktype = get_config_int(unitname[i].c_str(), "attacktype", 0);
		unitstats[i].basetype = get_config_int(unitname[i].c_str(), "basetype", 0);
		unitstats[i]._rangemin = get_config_int(unitname[i].c_str(), "rangemin", 0);
		unitstats[i]._rangemax = get_config_int(unitname[i].c_str(), "rangemax", 0);
		unitstats[i]._price = get_config_int(unitname[i].c_str(), "price", 0);
		unitstats[i].description = get_config_string(unitname[i].c_str(), "description", "");
		unitstats[i].loadcapacity = get_config_int(unitname[i].c_str(), "loadcapacity", 0);
		if (unitstats[i].loadcapacity < 0) unitstats[i].loadcapacity = 0;
		if (unitstats[i].loadcapacity > 2) unitstats[i].loadcapacity = 2;
		unitstats[i].loadtype = get_config_int(unitname[i].c_str(), "loadtype", 0);
		if (unitstats[i].loadtype == 0) unitstats[i].loadcapacity = 0;
		unitstats[i].canhit = get_config_int(unitname[i].c_str(), "canhit", 0);
		unitstats[i].attackparticle = get_config_int(unitname[i].c_str(), "attackparticle", 0);
		unitstats[i].attacksound = get_config_int(unitname[i].c_str(), "attacksound", 0);
		unitstats[i].movesound = get_config_int(unitname[i].c_str(), "movesound", 5);
		unitstats[i].moveparticle = get_config_int(unitname[i].c_str(), "moveparticle", 0);
		unitstats[i].cancapture = (get_config_int(unitname[i].c_str(), "cancapture", 0) != 0);
		unitstats[i].isarmy[0] = (get_config_int(unitname[i].c_str(), "isarmy1", 1) != 0);
		unitstats[i].isarmy[1] = (get_config_int(unitname[i].c_str(), "isarmy2", 1) != 0);
		unitstats[i].isarmy[2] = (get_config_int(unitname[i].c_str(), "isarmy3", 1) != 0);
		unitstats[i].techlevel = get_config_int(unitname[i].c_str(), "techlevel", 2);
		unitstats[i].attacksburn = (get_config_int(unitname[i].c_str(), "attacksburn", 0) != 0);
		unitstats[i].healthdrawhack = (get_config_int(unitname[i].c_str(), "healthdepdraw", 0) != 0);
		unitstats[i].blastradius = get_config_int(unitname[i].c_str(), "blastradius", 0);
		if (unitstats[i].blastradius < 0) unitstats[i].blastradius = 0;
		unitstats[i].attackdelay = get_config_int(unitname[i].c_str(), "attackdelay", 0);
		if (unitstats[i].attackdelay < 0) unitstats[i].attackdelay = 0;
		j = 0;
		while (j < 3)
		{
			sprintf(str, "attack%d", j);
			unitstats[i]._attack[j] = get_config_int(unitname[i].c_str(), str, 0);
			sprintf(str, "resist%d", j);
			unitstats[i]._resist[j] = get_config_int(unitname[i].c_str(), str, 0);
			j++;
		}
		j = 0;
		while (unsigned(j) < unitname[i].length())
		{
			if (unitname[i][j] == '_')
			{
				unitname[i][j] = ' ';
			}
			j++;
		}
		unitstats[i].name = unitname[i];
		
		n = "loaded ";
		n += unitname[i];
		empdebug(n);
		i++;
	}
}


int intro()
{
	static bool fadein = true;
	static float alpha = 0.0, aspeed = 0.02;
	static int time = 0;
	
	if (fadein)
	{
		if (alpha < 1.0)
		{
			alpha += aspeed;
			if (alpha > 1.0) alpha = 1.0;
		}
		else
		{
			time++;
			if (time > 40) fadein = false;
		}
	}
	else
	{
		if (alpha <= 0.0) return MENU;
		else
		{
			alpha -= aspeed;
			if (alpha < 0.0) alpha = 0.0;
		}
	}
	
	glb.draw(44, 0, 0, alpha);
	return GAMEINTRO;
}


int do_transition(int target)
{
	return target;
/*  static int time = 0;
  static int type = 0;
  static bool out = true;
  static bool first = true;
  int j;
  
  if (first)
  {
    type = 0;//rand()%4;
    time = 0;
    first = false;
    out = true;
  }

  time++;
  if (out)
  {
    buffer_draw_spite(transition, 0, 0);
    switch(type)
    {
      case 0:
        j = 0;
        while (j < 32)
        {
					buffer_rectfill(20 * j, 0, time + 20 * j, 480, BLACK);
          j++;
        }
        if (time >= 20)
        {
          time = -1;
          out = false;
        }
        break;
      case 1:
        buffer_rectfill(0, 0, 32 * time, 480, BLACK);
        if (time >= 20)
        {
          time = -1;
          out = false;
        }
        break;
      case 2:
        j = 0;
        while (j < 480)
        {
          if (j % 2 == 0)
          {
            buffer_hline(0, j, 40 * time - (j / 2), BLACK);
          }
          else
          {
            buffer_hline(640 - (40 * time) + (j / 2), j, 640, BLACK);
          }
          j++;
        }
        if (time >= 23)
        {
          time = -1;
          out = false;
        }
        break;
      case 3:
        buffer_rectfill(0, 0, 640, time * 20, BLACK);
        buffer_rectfill(0, 480 - (time * 20), 640, 480, BLACK);
        if (time >= 12)
        {
          time = -1;
          out = false;
        }
        break;
    }
  }
  else
  {
    if (time == 0)
    {
      switch(target)
      {  //do the transitioned-to function once, to draw it
        case MENU:
          menu();
          break;
        case MAPEDIT:
          mapedit();
          break;
        case PLAYEREDIT:
          playeredit();
          break;
        case MISSION:
          mission();
          break;
      }
      draw_all(transition);
    }
    else
    {
      buffer_draw_spite(transition, 0, 0);
      switch (type)
      {
        case 0:
          j = 0;
          while (j < 32)
          {
            buffer_rectfill(time + 20 * j, 0, 20 + 20 * j, 480, BLACK);
            j++;
          }
          if (time >= 20)
          {
            first = true;
            return target;
          }
          break;
        case 1:
          buffer_rectfill(32 * time, 0, 640, 480, BLACK);
          if (time >= 20)
          {
            first = true;
            return target;
          }
          break;
        case 2:
          j = 0;
          while (j < 480)
          {
            if (j % 2 == 0)
            {
              buffer_hline(40 * time - (j / 2), j, 640, BLACK);
            }
            else
            {
              buffer_hline(0, j, 640 - (40 * time) + (j / 2), BLACK);
            }
            j++;
          }
          if (time >= 23)
          {
            first = true;
            return target;
          }
          break;
        case 3:
          buffer_rectfill(0, 0, 640, 240 - (time * 20), BLACK);
          buffer_rectfill(0, 240 + (time * 20), 640, 480, BLACK);
          if (time >= 12)
          {
            first = true;
            return target;
          }
          break;
      }
    }
  }
  return TRANSITION;*/
}

int load_characters()
{
	int i;
	char str[30];
	character.reserve(3);
	
	i = 0;
	while (i < 3)
	{
		sprintf(str, "army%d_name", i + 1);
		character.push_back(new _character);
		character[i]->setname(get_config_string("armies", str, "army"));
		sprintf(str, "army%d_description", i + 1);
		character[i]->setdescription(get_config_string("armies", str, "-"));
		sprintf(str, "data/graphics/army%d.png", i + 1);
		character[i]->picture = new ol::Bitmap(str);
		i++;
	}
	
	return 0;
}

int show_winner()
{
	static bool gotstats = false;
	static int time = 0;
	static int delay = 0;
	static int current = -1;
	static int left = 0;
  int i, t;
  char c[40];
  DyBytes data;
  
	if (global.netgame && !net.server && !gotstats)
	{
		data = get_host_datablob();
		if (data.size() > 0)
		{
			if (data.getByte(0) == NET_STATS)
			{
				data.removeFront(1);
				data.decompress();
				i = 0;
				while (i < 4)
				{
					data.extractVariable(0, player[i].stats.raised);
					data.removeFront(sizeof(int));
					data.extractVariable(0, player[i].stats.spent);
					data.removeFront(sizeof(int));
					data.extractVariable(0, player[i].stats.produced);
					data.removeFront(sizeof(int));
					data.extractVariable(0, player[i].stats.killed);
					data.removeFront(sizeof(int));
					data.extractVariable(0, player[i].stats.lost);
					data.removeFront(sizeof(int));
					i++;
				}
				gotstats = true;
			}
		}
	}
	
	if (time == 0)
	{
		stop_music();
		play_song(victory_music);
		
		if (global.netgame && net.server)
		{
			data.clear();
			i = 0;
			while (i < 4)
			{
				data.appendVariable(player[i].stats.raised);
				data.appendVariable(player[i].stats.spent);
				data.appendVariable(player[i].stats.produced);
				data.appendVariable(player[i].stats.killed);
				data.appendVariable(player[i].stats.lost);
				i++;
			}
			data.compress(9);
			data.insertByte(NET_STATS);
			broadcast_datablob(&data);
		}
		
		left = 410;
		i = 0;
		while (i < 4)
		{
			if (player[i].everplaying == 1)
			{
				left -= 50;
			}
			i++;
		}
		
    delay = 0;
    current = -1;
    time++;
  }
  else if (time < 60)
  {
    t = 60 - time;
    buffer_rectfill(0, 0, 399 * time / 60, 600, DGREY);
    buffer_vline(400 * time / 60, 0, 600, GREY);
    buffer_vline(401 * time / 60, 0, 600, LGREY);
    buffer_rectfill(400 + (401 * t / 60), 0, 800, 600, DGREY);
    buffer_vline(400 + (400 * t / 60), 0, 600, GREY);
    buffer_vline(399 + (400 * t / 60), 0, 600, LGREY);
    buffer_textout_centre_ex(big, "Game Over", 400, time - 50, WHITE);
    time++;
  }
  else if (time < 128)
  {  //fade to black
    t = 128 - time;
    buffer_clear_to_color(makecol(t, t, t));
    buffer_textout_centre_ex(big, "Game Over", 400, 10, WHITE);
    time++;
  }
  else if (time < 188)
  {  //move player pictures from the bottom of the screen to y = 100
    time++;
    t = 188 - time;
    buffer_textout_centre_ex(big, "Game Over", 400, 10, WHITE);
    i = 0;
    while (i < 4)
    {
      if (player[i].everplaying == 1)
      {
        buffer_rectfill(left + (100 * i), 100 + (t * 6), left + (100 * i) + 83, 183 + (t * 6), player[i].tintcolor);
        buffer_draw_sprite(currentcharacter[i]->picture, left + 2 + (100 * i), 102 + (t * 6));
      }
      i++;
    }
  }
  else if (time < 220)
  {  //pause for a bit
    buffer_textout_centre_ex(big, "Game Over", 400, 10, WHITE);
    i = 0;
    while (i < 4)
    {
      if (player[i].everplaying == 1)
      {
        buffer_rectfill(left + (100 * i), 100, left + (100 * i) + 83, 183, player[i].tintcolor);
        buffer_draw_sprite(currentcharacter[i]->picture, left + 2 + (100 * i), 102);
				if (global.netgame) buffer_textout_centre_ex(med, player[i].name, left + (100 * i) + 40, 185, WHITE);
      }
      i++;
    }
    time++;
  }
  else if (time == 220)
  {  //go through and replace defeated players with an X 
    buffer_textout_centre_ex(big, "Game Over", 400, 10, WHITE);
    i = 0;
    while (i < 4)
    {
      if (player[i].everplaying == 1)
      {
        buffer_rectfill(left + (100 * i), 100, left + (100 * i) + 83, 183, player[i].tintcolor);
      }
      i++;
    }
    i = 0;
    while (i < 4)
    {
      if (i <= current)
      {
        if (player[i].everplaying == 1)
        {
          if (player[i].playing == 0)
          {
            glb.draw(19, left + 2 + (100 * i), 102);
          }
          else
          {
            buffer_draw_sprite(currentcharacter[i]->picture, left + 2 + (100 * i), 102);
            if (global.netgame) buffer_textout_centre_ex(med, player[i].name, left + (100 * i) + 40, 185, WHITE);
          }
        }
      }
      else
      {
        if (player[i].everplaying == 1)
        {
          buffer_draw_sprite(currentcharacter[i]->picture, left + 2 + (100 * i), 102);
          if (global.netgame) buffer_textout_centre_ex(med, player[i].name, left + (100 * i) + 40, 185, WHITE);
        }
      }
      i++;
    }
    if ((delay == 0) && (current < 4))
    {
      delay = 25;
      current++;
      if ((player[current].everplaying == 1) && (player[current].playing == 0))
      {
        i = rand()%3;
        switch(i)
        {
          case 0:
            play_sound(S_TANKFIRE);
            break;
          case 1:
            play_sound(S_MDTANKFIRE);
            break;
          case 2:
            play_sound(S_EXPLOSION);
            break;
        }
      }
    }
    else if (current < 4)
    {
      delay--;
    }
    if (current == 4)
    {
      time++;  //keeps this section active until current == 4
    }
  }
  else
  {
    if (time == 221)
    {
      play_sound(S_VICTORY);
    }
    else
    {
      time--;  //just to keep time from increasing any more
    }
    buffer_textout_centre_ex(big, "Game Over", 400, 10, WHITE);
    i = 0;
    t = 0;
		
		buffer_textout_ex(normal, "Money Raised  :", 100, 220, WHITE);
		buffer_textout_ex(normal, "Money Spent   :", 100, 240, WHITE);
		buffer_textout_ex(normal, "Units Created :", 100, 260, WHITE);
		buffer_textout_ex(normal, "Enemies Killed:", 100, 280, WHITE);
		buffer_textout_ex(normal, "Units Lost    :", 100, 300, WHITE);
		
    while (i < 4)
    {
      if (player[i].everplaying == 1)
      {
				sprintf(c, "$%d", player[i].stats.raised);
				buffer_textout_centre_ex(normal, c, left + (100 * i) + 40, 220, WHITE);
				sprintf(c, "$%d", player[i].stats.spent);
				buffer_textout_centre_ex(normal, c, left + (100 * i) + 40, 240, WHITE);
				sprintf(c, "%d", player[i].stats.produced);
				buffer_textout_centre_ex(normal, c, left + (100 * i) + 40, 260, WHITE);
				sprintf(c, "%d", player[i].stats.killed);
				buffer_textout_centre_ex(normal, c, left + (100 * i) + 40, 280, WHITE);
				sprintf(c, "%d", player[i].stats.lost);
				buffer_textout_centre_ex(normal, c, left + (100 * i) + 40, 300, WHITE);
				
				buffer_rectfill(left + (100 * i), 100, left + (100 * i) + 83, 183, player[i].tintcolor);//player_color(i));
				if (player[i].playing == 0)
				{
					glb.draw(19, left + 2 + (100 * i), 102);
				}
				else
				{
					t++;
					buffer_rectrounded(left - 1 + (100 * i), 99, left + 84 + (100 * i), 184, WHITE);
					buffer_draw_sprite(currentcharacter[i]->picture, left + 2 + (100 * i), 102);
					if (global.netgame) buffer_textout_centre_ex(med, player[i].name, left + (100 * i) + 40, 185, WHITE);
				}
      }
      i++;
    }
    if (t == 1)
    {
      buffer_textout_centre_ex(big, "The Winner!", 400, 350, WHITE);
    }
    else
    {
      buffer_textout_centre_ex(big, "The Winners!", 400, 350, WHITE);
    }
    buffer_textout_centre_ex(normal, "Press Enter", 400, 570, WHITE);
    time++;
  }
  if (key[KEY_ENTER])
  {
		gotstats = false;
		
    play_song(title_music);
    position_mouse(400, 300);
    time = 0;
    player[0].reset();
    player[1].reset();
    player[2].reset();
    player[3].reset();
		
		if (global.netgame)
		{
			global.returned_from_netgame = true;
			return NETWORK;
		}
		
    return MENU;
  }
  return GAMEOVER;
}

void save_debug_info()
{
  int i = 0;
  int j, k, l;
  _unit* u;
  ofstream dump("empiredump.txt");
  while (i < 4)
  {
    if (player[i].playing == 1)
    {
      dump << "PLAYER " << i << "\n";
      j = 0;
      while (j < 50)
      {
        u = &player[i].unit[j];
        dump << "  UNIT " << j << "\n";
        dump << "    exists:         " << u->exists << "\n";
        dump << "    type:           " << unit_name(u->type) << "\n";
        dump << "    tilex:          " << u->tilex << "\n";
        dump << "    tiley:          " << u->tiley << "\n";
        dump << "    ready:          " << u->ready << "\n";
        dump << "    color:          " << u->color << "\n";
        dump << "    number:         " << u->number << "\n";
        dump << "    health:         " << u->health << "\n";
        k = 0;
        while (k < unitstats[u->type].loadcapacity)
        {
          dump << "      UNIT LOAD " << k << "\n";
          dump << "        loaded:  " << u->load[k].loaded << "\n";
          dump << "        type:    " << unit_name(u->load[k].type) << "\n";
          dump << "        health:  " << u->load[k].health << "\n";
          l = 0;
          while (l < 2)
          {
          	if (u->load[k].subload[l].loaded == 1)
          	{
	            dump << "          UNIT LOAD " << k << " SUBLOAD\n";
            	dump << "            loaded:  " << u->load[k].subload[l].loaded << "\n";
            	dump << "            type:    " << unit_name(u->load[k].subload[l].type) << "\n";
            	dump << "            health:  " << u->load[k].subload[l].health << "\n";
            }
            l++;
          }
          k++;
        }
        dump << "\n";
        j++;
      }
      dump << "\n";
    }
    i++;
  }
}

void save_pathmap_info()
{
  int i, j, k;
  ofstream path("pathmap.txt");
  j = 0;
  while (j < worldmap.h)
  {
    i = 0;
    while (i < worldmap.l)
    {
      k = worldmap.tile[i][j].get_step();
      if (k < 10) path << " ";
      path << k << " ";
      if (k < 99) path << " ";
      i++;
    }
    path << "\n";
    j++;
  }
}

void save_unitmap_info()
{
  int i, j, k;
  ofstream unit("unitmap.txt");
  j = 0;
  while (j < worldmap.h)
  {
    i = 0;
    while (i < worldmap.l)
    {
      k = worldmap.tile[i][j].unit_here();
      if ((k < 100) && (k != -1)) unit << " ";
      if (k < 10) unit << " ";
      unit << k << " ";
      i++;
    }
    unit << "\n";
    j++;
  }
}

void save_ownermap_info()
{
	int i, j, k;
	ofstream unit("ownermap.txt");
	j = 0;
	while (j < worldmap.h)
	{
		i = 0;
		while (i < worldmap.l)
		{
			k = worldmap.tile[i][j].get_owner();
			if ((k < 100) && (k != -1)) unit << " ";
			if (k < 10) unit << " ";
			unit << k << " ";
			i++;
		}
		unit << "\n";
		j++;
	}
}

string unit_name(int type)
{
	if ((type > -1) && (type < NUM_UNITS))
	{
		return unitstats[type].name;
	}
	return "-";
}

int unit_type(string name)
{
	int i = 0;
	while (i < NUM_UNITS)
	{
		if (name == unitstats[i].name)
		{
			return i;
		}
		i++;
	}
	return -1;
}

int player_color(int army, int tintnum)
{
	switch (army)
	{
		case 0:  //crimson
			if (tintnum == 0) return RED;
			if (tintnum == 1) return makecol(255, 120, 0);
			if (tintnum == 2) return makecol(255, 200, 30);
			return makecol(255, 0, 100);
			break;
		case 1:  //cobalt
			if (tintnum == 0) return BLUE;
			if (tintnum == 1) return makecol(0, 255, 255);
			if (tintnum == 2) return makecol(0, 100, 255);
			return makecol(115, 0, 255);
			break;
		case 2: //pine
			if (tintnum == 0) return MGREEN;
			if (tintnum == 1) return makecol(180, 200, 0);
			if (tintnum == 2) return makecol(0, 200, 90);
			return makecol(140, 230, 65);
			break;
	}
	return BLACK;	
}

void empdebug(string text)
{
	if (DEBUGGING == 1)
	{
		if (text == "-CLEAR-")
		{
			ofstream d("empdebug.txt");
			d.close();
		}
		else
		{
			ofstream d("empdebug.txt", ios::app);
			if (!d.bad())
			{
				d << text.c_str() << "\n";
			}
			d.close();
		}
	}
}

void empdebug(int i)
{
	char c[30];
	sprintf(c, "%d", i);
	empdebug(c);
}

void empdebug(float f)
{
	char c[30];
	sprintf(c, "%f", f);
	empdebug(c);
}

int helpscreen()
{
	static bool first = true;
	static short n = 0;
	static _button next;
	
	if (first)
	{
		n = 0;
		next.init(689, 551, 101, 39, "", 6);
		first = false;
	}
	
	glb.draw(45 + n, 0, 0);
	next.check();
	if (next.clicked())
	{
		n++;
		if (n > 3)
		{
			first = true;
			return MENU;
		}
	}
	
	draw_mouse();
	return HELPSCREEN;
}

void create_sidebars()
{
	ol::Rgba tint;
	sidebarbmp.destroy();
	ol::Settings::RestoreOldProjection();
	
	int i = 0;
	while (i < 4)
	{
		tint = ol::Rgba(player[i].tintcolor, 80);
		
		sidebarbmp.b.push_back(new ol::Bitmap(160, 600, ol::Rgba(BLACK, 0)));
		ol::Canvas::SetTo(*sidebarbmp.b[i * 3]);
		ol::Canvas::SetPixelWriteMode(ol::COLOR_AND_ALPHA);
		statbmp.b[0]->Blit(0, 0, ol::Tinted(tint));
		ol::Canvas::Refresh();
		
		sidebarbmp.b.push_back(new ol::Bitmap(160, 600, ol::Rgba(BLACK, 0)));
		ol::Canvas::SetTo(*sidebarbmp.b[1 + i * 3]);
		ol::Canvas::SetPixelWriteMode(ol::COLOR_AND_ALPHA);
		statbmp.b[1]->Blit(0, 0, ol::Tinted(tint));
		ol::Canvas::Refresh();
		
		sidebarbmp.b.push_back(new ol::Bitmap(640, 24, ol::Rgba(BLACK, 0)));
		ol::Canvas::SetTo(*sidebarbmp.b[2 + i * 3]);
		ol::Canvas::SetPixelWriteMode(ol::COLOR_AND_ALPHA);
		if (global.netgame) statbmp.b[2]->Blit(0, 0, ol::Tinted(tint));
		else statbmp.b[13]->Blit(0, 0, ol::Tinted(tint));
		ol::Canvas::Refresh();
		
		sidebarbmp.size += 3;
		i++;
	}
	
	ol::Canvas::SetTo(ol::SCREEN_BACKBUF);
	ol::Settings::SetOrthographicProjection(800, 600);
}

int credits()
{
	static bool first = true;
	static _button done;
	
	if (first)
	{
		done.init(10, 570, 100, 20, "Back");
		first = false;
	}
	
	glb.draw(32, 0, 0);
	buffer_textout_centre_ex(big, "Credits", 400, 40, WHITE);
	
	buffer_textout_right_ex(big, "Project Leader, Programming:", 490, 100, WHITE);
	buffer_textout_ex(big, "Kent deVillafranca", 510, 100, WHITE);
	buffer_textout_right_ex(big, "Unit Type & Balancing, Game Design, Graphics:", 490, 130, WHITE);
	buffer_textout_ex(big, "Chris DeLeon", 510, 130, WHITE);
	buffer_textout_right_ex(big, "User Interface:", 490, 160, WHITE);
	buffer_textout_ex(big, "Greg Peng", 510, 160, WHITE);
	buffer_textout_right_ex(big, "Title Music:", 490, 190, WHITE);
	buffer_textout_ex(big, "Sam Chien", 510, 190, WHITE);
	
	buffer_textout_ex(normal, "Other music (except Holst) licensed on behalf of Chris DeLeon by Stockmusic.net (Digital Sound Media), a division of REM Publishing Ltd.", 5, 250, WHITE);
	
	buffer_textout_ex(normal, "Battle Setup:", 30, 270, WHITE);
	buffer_textout_ex(normal, "Track Name: Revelation of One", 50, 285, WHITE);
	buffer_textout_ex(normal, "Track Length: 2:00", 50, 300, WHITE);
	buffer_textout_ex(normal, "Composer (and Organisation): Joseph Bennie (APRA)", 50, 315, WHITE);
	buffer_textout_ex(normal, "Publisher (and Organisation): Joseph Bennie (APRA)", 50, 330, WHITE);
	
	buffer_textout_ex(normal, "Game Over:", 430, 270, WHITE);
	buffer_textout_ex(normal, "Track Name: Not a Happy Ending", 450, 285, WHITE);
	buffer_textout_ex(normal, "Track Length: 1:24", 450, 300, WHITE);
	buffer_textout_ex(normal, "Composer (and Organisation): Ron Sorbo (BMI)", 450, 315, WHITE);
	buffer_textout_ex(normal, "Publisher (and Organisation): Ames Rush (BMI)", 450, 330, WHITE);

	buffer_textout_ex(normal, "Crimson Theme", 30, 350, WHITE);
	buffer_textout_ex(normal, "Track Name: Mars, the Bringer of War", 50, 365, WHITE);
	buffer_textout_ex(normal, "Track Length: 4:40", 50, 380, WHITE);
	buffer_textout_ex(normal, "Composer (and Organisation): Gustav Holst", 50, 395, WHITE);
	buffer_textout_ex(normal, "Publisher (and Organisation): USAF Band", 50, 410, WHITE);

	buffer_textout_ex(normal, "Cobalt Theme", 430, 350, WHITE);
	buffer_textout_ex(normal, "Track Name: Prodigious Urban Spawn", 450, 365, WHITE);
	buffer_textout_ex(normal, "Track Length: 1:21", 450, 380, WHITE);
	buffer_textout_ex(normal, "Composer (and Organisation): Dan Schmidt (SOCAN)", 450, 395, WHITE);
	buffer_textout_ex(normal, "Publisher (and Organisation): Dan Schmidt (SOCAN)", 450, 410, WHITE);

	buffer_textout_ex(normal, "Pine Theme", 30, 430, WHITE);
	buffer_textout_ex(normal, "Track Name: Cold Wars", 50, 445, WHITE);
	buffer_textout_ex(normal, "Track Length: 0:50", 50, 460, WHITE);
	buffer_textout_ex(normal, "Composer (and Organisation): Patrick Smith (ASCAP)", 50, 475, WHITE);
	buffer_textout_ex(normal, "Publisher (and Organisation): Patrick Smith (ASCAP)", 50, 490, WHITE);
	
	done.check();
	if (done.clicked())
	{
		first = true;
		return MENU;
	}
	
	draw_mouse();
	return CREDITS;
}
