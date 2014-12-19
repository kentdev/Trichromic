#ifndef _AI_CHECK_H
#define _AI_CHECK_H

#define action_none    -1
#define action_move     0
#define action_attack   1
//#define action_capture  2
#define action_load     3
#define action_unload1  4
#define action_unload2  5
#define action_standby  6
#define action_merge    7
#define action_teleport 8

extern void update_cities();
extern void apply_player_bonuses(float& basic, _unit *a, _unit *d);
extern void play_moving_sound(int utype);
extern void transform_buildings(int plyr, int plyr2);
extern void do_power();
extern void capture_building(_unit *u);
extern void merge_units(_unit *merger, int tx, int ty);
extern void show_movable_tiles(_unit *u, float alpha = 0.66);
extern bool type_can_attack(_unit *a, _unit *d);
extern bool check_explodes();
extern int basic_damage(_unit *a, _unit *d);
extern int defbonus(int tx, int ty);
extern int unit_here_thorough(int plyr, int tx, int ty, int ignore_n);
extern int do_battle(_unit *a, _unit *d);
extern int rounded_health(float h);
extern int get_tilenum(int mapdat);
extern int rounded_health(float h);
extern int scroll_to_location(int x, int y);
extern int moves_needed(int mtype, int tx, int ty, int plyr);

extern string unit_name(int type);

bool enemy_threat(int plyr);
bool enemy_within_radius(int plyr, int center_x, int center_y, int r);
bool can_get_to_aitarget(_unit *a, _aitarget d);
void act(_unit *u, _action a);
void make_new_units(int plyr);
void set_best_path(_unit *u, int tx, int ty);
void ai_teleport(_unit *u, _loc dest);
int get_damage_cost(_unit *a, _unit *d);
int get_return_damage_cost(_unit *a, _unit *d);
int any_unit_here(int tx, int ty);
int next_to_enemy(int plyr, int tx, int ty);
int check_can_build(int plyr, int price[], int num[]);
int intmax(int a, int b);
int intmin(int a, int b);
int can_unload_from_here(int x, int y, int mountains);
int do_ai(int pturn);
int do_ai(int pturn, bool reset_everything);
_actionset ponder_action(_unit *u);
_aitarget best_target();
_aitarget aitarget_here(_unit *u, int tx, int ty);
_loc closest_teleport(_unit *u);
_loc capturable_building(_unit *u);
_loc open_build_loc(int plyr, int u);

extern _action reserve_action;
extern _action action[3];

extern int in_battle, atk, def;
extern vector<_unit*> lander;
extern vector<_unit*> need_transport;
extern vector<_loc> base;
extern vector<_aitarget> targets;
extern _loc capital;
extern vector<_aitarget> list_targets(_unit *u);
extern vector<_connectedlocs> beaches;
extern vector<_loc> teleports;

void check_HQ_park(_unit *u);
void check_capturing(_unit *u);
void check_attackable_units(_unit *u);
void check_attackable_units(_unit *u, _loc l);
void check_low_health(_unit *u);
void check_closest_capturable(_unit *u);
void check_closest_attackable(_unit *u);
void check_APC_options(_unit *u);
void check_tcopter_options(_unit *u);
void check_lander_options(_unit *u);
void check_blocking_base(_unit *u);
void check_convenient_APC(_unit *u);
void check_available_copter(_unit *u);
void check_enemy_bases(_unit *u);
void check_available_lander(_unit *u);
void check_unload(_unit *u, _loc targetbeach);
void check_teleports(_unit *u);

#endif
