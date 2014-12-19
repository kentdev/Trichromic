#ifndef _structs_h
#define _structs_h

#include "units.h"

typedef struct _loc
{
  int x;
  int y;
} _loc;

typedef struct _aipath
{
  int dir;
  int total_moves;
  _aipath()
  {
    dir = -1;
    total_moves = 0;
  }
} _aipath;

typedef struct _aitarget
{
  int type;
  float health;
  int damage_cost;
  int attacker_cost;
  _loc loc;
} _aitarget;

typedef struct _connectedlocs
{
  vector<_loc> loc;
} _connectedlocs;

typedef struct _globalsettings
{
	bool networking;
  bool sound;
	bool netgame;
	bool returned_from_netgame;
	short surrenderval;
  int gamestatus;
  int mspeed;
  int volume;
  int music_volume;
  int fullscreen;
  int particles;
  int maporder;
  int windowed_resolution;
  int current_resolution;
	int screen_width, screen_height;
  int loaded_game;
  int very_first_turn;
  int scroll_divider;
  int AI_scroll_divider;
  int battle_animation;
	string netname;
	string netlobby;
	string lastmap;
} _globalsettings;

typedef struct _init_variables
{
  int screenmode;
  int screen_x;
  int screen_y;
  int current_res;
} _init_variables;

typedef struct _logic_variables
{
	bool first;
	bool typingchat;
  int oldux, olduy;
  int attacker, defender;
  int unitx, unity;
  int unitmenu;
  int oldmx, oldmy;
  int unloadnum;
  int windelay;
  int logicstate;
  int unitmoving;
  int time;
  int theplayer;
	int techlevel;
	int cashturns;
	_unit *unitselected;
} _logic_variables;

typedef struct _ingameoptions
{
  bool show_damage;
  int funding;
  int time;
  void reset();
} _ingameoptions;

typedef struct _action
{
  int type;
  _loc loc;
	_loc destination;
} _action;

typedef struct _actionset
{
  _action action[3];
} _actionset;

typedef struct _buildorder
{
  int unit;
  float ratio;
} _buildorder;

typedef struct setunit  //units predeployed on the map, if any
{
  int x;
  int y;
  int player;
  int type;
  int exist;
} setunit;

typedef struct _maparrange
{
  string name;
  int players;
  int l, h;
  bool bad;
} _maparrange;

typedef struct _armystats
{
	int pricefraction;
	int attackfraction[3];
	int resistfraction[3];
	int movemod;
	int rangemaxmod;
} _armystats;

#endif
