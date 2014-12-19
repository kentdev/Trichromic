#ifndef _logic_h
#define _logic_h

#define NOTHING_HAPPENING   0
#define AI_TURN             1
#define IN_BATTLE           2
#define UNIT_SELECTED       3
#define UNIT_MOVING         4
#define SAVING_GAME         5
#define IN_GENERAL_MENU     6
#define IN_UNIT_BUILD_MENU  8
#define MECHANIC_CAPPING    9
// (MECHANIC_CAPPING is a mechanic capturing an enemy UNIT, not a building)
#define UNIT_UNLOADING      10
#define UNIT_TELEPORTING    11
#define TELEPORT_ANIMATION  12
//#define NET_BATTLE          13
#define SCROLLING           14
#define ONE_TEAM_LEFT       15
#define BACK_TO_MAIN_MENU   16
#define WINNER              17
#define FADE_EXIT           18
#define TIME_UP_INDICATOR   19
//#define AI_MENU             20
#define EXPLODING           21
#define PLAYER_DEFEATED     22
#define IN_OPTIONS_MENU     23
#define NETPLAYER_TURN      24
#define NET_WAITING_FOR_RESPONSE 25
// NET_WAITING_FOR_RESPONSE is only for the client (when they have asked the server to move a unit or something)

#define START_TURN_SYNC     26
#define IN_HELP_SCREEN      27


typedef struct _unitexplosion
{
  int tx;
  int ty;
  float frame;
  int exists;
  bool notaunit_oilderrick;  //ugly hack to reuse explosion code to let people know when an oil derrick has been used up
} _unitexplosion;

extern vector<_unitexplosion> boom;
extern list<_particle> p_effects;

extern bool rotate_units(_unit *a, _unit *d, int rotationspeed);
extern bool battle_attack(_unit *a, _unit *d);

extern void draw_tiles(int scrollx, int scrolly, int editing, int x1, int y1, int w, int h, int keep_units);
extern void draw_tiles(int scrollx, int scrolly, int editing, int keep_units);
extern int do_battle(_unit *a, _unit *d);
extern int HQ_exists(int playernum);
extern int base_exists(int playernum);
extern int do_ai(int plyr);
extern int do_ai(int plyr, bool reset_everything);
extern int any_unit_here(int tx, int ty);
//extern int player_color(int c);
extern int do_gameoptions(bool ingame);
extern float total_damage(_unit *a, _unit *d);

extern string unit_name(int type);
extern int unit_type(string name);

extern int loaded_time;  //used to ensure that players can't get infinite time by constantly saving and reloading

extern int pturn;

extern vector<int> movelist;

void check_minimap(int &m);
void set_moves(int plyr, int unitnum, int tx, int ty);
void clear_moveselect();
void draw_path(_unit *u);
void draw_unitstats(_unit *u, int x, int y);
void transform_buildings(int plyr, int plyr2);
void repair_units(int plyr);
void scroll_to_hq(int plyr);
void play_moving_sound(int utype);
void init_explode(int x, int y);
void init_deadoilderrick(int tx, int ty);
void do_power();
void capture_building(_unit* u);
void merge_units(_unit *merger, int tx, int ty);
void check_scrolling();
void check_scrolling(int forcex, int forcey);
void debug_savegame();
void destroy_theplayers_units();
void time_up();
void logic_netchat(string message = "");
//void ai_menu();
void update_cities();
void update_burning();
void do_firedamage(_unit *u, int damage);
void broadcast_unithealth(_unit *u);
void get_unithealth(DyBytes *data);
void broadcast_tiledata();
void get_tiledata(DyBytes *data);
void draw_net_icon(int x, int y);
void draw_wait_icon(int x, int y);
void mechanic_capture(_unit *capper, int captx, int capty);
void unload_unit(_unit *u, int unum, int tx, int ty);
void start_teleport_animation(_unit *u, int tx, int ty);
void teleport_animation(_unit *initunit, int tx, int ty);
void client_netbattle();
void draw_shieldrange(_unit *u);
void do_surrender(int from, int to);
void convert_theplayers_units(int from, int to);
bool one_team_left();
bool player_defeated(int plyr);
bool move_overlap(_unit *u);
bool can_merge(_unit *a, _unit *b);
bool type_can_attack(_unit *a, _unit *d);
bool unit_unloadable_here(int ox, int oy, int tx, int ty, int type);
bool check_explodes();
bool sync_burning();
bool help_screen();
int do_explodes();
int scroll_to_location(int x, int y);
int attackable_unit_here(_unit *attacker, int tx, int ty);
int attackable_unit_here(_unit *attacker, int tx, int ty, bool ignore_teams);
int unit_here_thorough(int plyr, int tx, int ty, int ignore);
int moves_needed(int mtype, int tx, int ty, int plyr);
int get_ltype(int tx, int ty);
int do_moveselect(int plyr, int u, int tx, int ty);
int list_distance(_unit *u);
int do_unitmenu(int menutype, int tx, int ty, int pturn);
int money_producing_buildings_owned(int plyr);
int do_unloadmenu(_unit *u);
int rounded_health(float h);
int unit_exists(int plyr);
int do_savemenu();
int secure_base(int plyr);
int reconnect();
string do_generalmenu();
string do_actionmenu(_unit *u);

void init_logic();
void battle();
void windelay();
void save_game();
void ai();
void netplayer();
void nothing_happening();
void unit_moving();
void unit_selected();
void mechanic_capping();
void unit_unloading();
void unit_teleporting();
void build_menu();
void general_menu();

void show_movable_tiles(_unit *u, float alpha);
void draw_attackables(vector<_loc> locations);
void show_unload_locations(vector<_loc> locations);
vector<_loc> find_attackables(_unit *u);
vector<_loc> find_unload_locations(_unit *u, int type);

void do_sidebar(int p, _unit *u, bool drawonly = false, bool reset = false);

#endif
