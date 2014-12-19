#ifndef _map_h
#define _map_h

const short MAP_TILE_SIZE = 32;
const short XTILES        = (800 / MAP_TILE_SIZE) - 5;  //5 tiles needed for the sidebar
const short YTILES        = 600 / MAP_TILE_SIZE;
const short MAP_MAX_L     = 50;
const short MAP_MAX_H     = 50;

typedef struct animtiles
{
  short set, num, framenum;
  float frameinc;
} animtiles;

class _tile
{
  short unit;
  short set, number;
  short step;  //this should never go above 999
  short temp;  //neither should this
  short animated;
  short owner;
  short oldset, oldnum;
	float animframe;
	float burnframe;
  public:
		bool burning;
		short turnsused;  //for cities (or "oil derricks" as they're being called now), to make cash limited
		short burnedfor;
    void reset();
    void change_type(int new_set, int new_number);
    void change_owner(int plyr);
    void change_to_city();
    void draw(int x, int y);
	  void draw_unit(int x, int y, _unit *other_unit_here);
    void set_unit(int plyr, int number);
    void set_temp(short t);
    void set_step(short s);
		bool is_building();
    bool is_building(int type);
    bool is_unit_producing();
    bool is_HQ();
    bool owned_by(int plyr);
    bool same_type(int t);
    bool same_type(int tset, int tnum);
    short get_temp();
    short get_step();
    int unit_here();
    int get_owner();
    int get_set();
    int get_number();
    int building_type();
    short x, y;
};

class _map
{
  string name;
  int pathmap_limit;
  void create_pathmap(int plyr, int mtype, int startx, int starty, bool limited);
  void pathmap_recursive(int plyr, int mtype, int ox, int oy, int moves, bool limited);
  public:
    _tile tile[MAP_MAX_L][MAP_MAX_H];
    int l, h, scroll_x, scroll_y;
    int offset_x, offset_y;
    void reset();
    string get_name();
    void set_name(string new_name);
    void draw_tiles(bool showunits = true);
    void draw_tiles(int gridx, int gridy, int numx, int numy, bool showunits = true);
    void draw_tiles(int gridx, int gridy, int numx, int numy, _unit *other_unit_here, bool showunits = true);
    void create_pathmap(int plyr, int mtype, int startx, int starty);
    void create_limited_pathmap(int plyr, int mtype, int startx, int starty, int limit);
    void backup_pathmap();
    void restore_pathmap();
		bool in_bounds(int x, int y);
};

void load_map(string mapname, const char *path, bool reset_players = true);

#endif
