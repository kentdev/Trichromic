#ifndef _unit_h
#define _unit_h

typedef struct unitprops
{
  short _move;
  short movetype;
  short attacktype;
  short basetype;
	short _rangemin;
	short _rangemax;
	short loadcapacity;
	short loadtype;
	short canhit;
	short attackparticle;
	short attacksound;
	short movesound;
	short moveparticle;
	short _attack[3], _resist[3];
	short techlevel;
	short blastradius;
	short attackdelay;
	bool isarmy[3];
	bool cancapture;
	bool attacksburn;
	bool healthdrawhack;  //whether this unit has different bitmaps depending on health
	int _price;
	string name;
	string description;
} unitprops;

extern unitprops unitstats[MAX_UNITS];

typedef struct _subload
{
  short loaded;
  short type;
  float health;
} _subload;

typedef struct _loadedunit
{
  short loaded;
  short type;
  float health;
  _subload subload[2];
} _loadedunit;

class _unit
{
  float frame;
  short x;
  short y;
  vector<int> mlist;
  short moving;
  short destx;
  short desty;
  void do_move();
	public:
    _unit();
		bool canmove;
		bool shielding;
    short otherunit;
    short type;
    short color;  // what player it belongs to (0, 1, 2, 3), not actual color (makecol())
    short number;
    short ready;
    float health;
    short exists;
    short tilex;
    short tiley;
	  short direction;
	  short attackdelay;
		short move, rangemin, rangemax, attack[3], resist[3];
		int price;
	  int rotation;
    void create(int utype, int tx, int ty, int tcolor, int tnum);
    void draw();
    void draw(bool increment_frame);
    void set_moves(vector<int> list);
    vector<int> get_movelist();
    void kill();
    void set_tiles(int tx, int ty);
    void do_movement();
		void reposition(int offset_x, int offset_y);
    bool can_load_unit(_unit *u);
    bool can_unload_unit(int n);
    bool is_moving();
    int load_unit(_unit *u);
		int units_loaded();
    _loadedunit load[2];
};

#endif
