#ifndef _empire
#define _empire

const unsigned char REVISION_MAJOR = 1;
const unsigned char REVISION_MINOR = 0;

#include <time.h>
#include <string>
#include <vector>
#include <fstream>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <allegro.h>
	#ifdef ALLEGRO_WINDOWS
		#include <winalleg.h>
	#endif
#include <fmod.h>
#include <OpenLayer.hpp>
//#include <jpgalleg.h>
using namespace std;

#define MAX_UNITS 30

#include "structs.h"
#include "units.h"
#include "ai_types.h"
#include "button.h"
#include "player.h"
#include "map.h"
#include "minimap.h"
#include "ranges.h"
#include "ai_path.h"
#include "ai.h"
#include "particle.h"
#include "sound.h"
#include "draw.h"
#include "listbox.h"
#include "fileselect.h"
#include "textline.h"
#include "networking.h"

// Building types
#define HQ      0
#define CITY    1
#define BASE    2
#define AIRPORT 3
#define PORT    4

#define QUIT      -1
#define MENU       0
#define LOGIC      1
#define MAPEDIT    2
#define PLAYEREDIT 3
#define GAMEOVER   4
#define GAMEINTRO  5
#define HELPSCREEN 6
#define NETWORK    7
#define TRANSITION 8
#define CREDITS    9

#define MENU_FIRST          10
#define MENU_LOAD           11
#define MENU_OPTIONS        12
#define MENU_GAME_OPTIONS   13
#define MENU_GAME_LOAD      14
//#define MENU_INGAME_OPTIONS 15

// Mouse types
#define MOUSE_NORMAL 0
#define MOUSE_TARGET 1

// Player controller types
#define HUMAN 0
#define AI    1
#define NET   2
#define NETAI 3

// Attack types
#define RANGED 0
#define DIRECT 1

// AI types, as well as unit basetypes
#define LAND   0
#define AIR    1
#define SEA    2
#define CUSTOM 3

#define BLACK makecol(0, 0, 0)
#define GREY makecol(128, 128, 128)
#define LGREY makecol(192, 192, 192)
#define DGREY makecol(64, 64, 64)
#define VDGREY makecol(32, 32, 32)
#define RED makecol(255, 0, 0)
#define GREEN makecol(0, 255, 0)
#define DGREEN makecol(0, 150, 0)
#define MGREEN makecol(0, 200, 0)
#define BLUE makecol(0, 0, 255)
#define YELLOW makecol(255, 255, 0)
#define DYELLOW makecol(150, 150, 0)
#define WHITE makecol(255, 255, 255)
#define ORANGE makecol(255, 128, 0)
#define BROWN makecol(150, 100, 50)
#define DBROWN makecol(100, 70, 40)
#define TRANS makecol(255, 0, 255)

#define RIGHT 1
#define DOWN  2
#define UP    3
#define LEFT  4

// Movement types
#define M_INFANTRY 0
#define M_TIRES    1
#define M_TREAD    2
#define M_AIR      3
#define M_MECH     4
#define M_SEA      5
#define M_LANDER   6

// Damage types
#define D_BULLET  0
#define D_FIRE    1
#define D_EXPLODE 2

#define SHIELD_RADIUS 3

extern int NUM_UNITS;

void play_sound(int soundtype);
extern int mouse_clicked();
extern int mouse_rclicked();
extern int player_color(int army, int tintnum);
extern void draw_mouse(int frame = 0);
extern void play_song(string path);
extern void empdebug(string text);
extern void empdebug(int i);
extern void empdebug(float f);

extern _globalsettings global;
extern _logic_variables var;

extern ol::Bitmap *transition;

extern bitmapdat land, river, sea, shoal, buildings, special;
extern bitmapdat mouse;
extern bitmapdat glb;
extern bitmapdat explode1, nukeboom;
extern bitmapdat statbmp, sidebarbmp;
extern bitmapdat units[MAX_UNITS], bigunits;
extern vector<animtiles> animated_tiles;
extern vector<string> maplist;
extern _armystats armystats[3];

extern ol::TextRenderer normal;
extern ol::TextRenderer med;
extern ol::TextRenderer big;

extern _player player[4];
extern _character *currentcharacter[4];
extern vector<_character *> character;
extern void save_map_and_players(ofstream *savefile);

extern volatile int counter;

extern _map worldmap;
extern _tile tile;
extern _minimap minimap;
extern _ingameoptions gameoptions;
extern FMOD_SOUND *musicfile;
extern FMOD_SYSTEM *fmodmain;

extern int pturn;
extern int numplayers;
extern int mouse_type;
//extern int first_mission_character;
extern string title_music;
extern string ingame_music[3];
extern string victory_music;
extern string battlemenu_music;

#endif
