#ifndef _player_h
#define _player_h

typedef struct _uselessstats
{
	int raised;
	int spent;
	int produced;
	int killed;
	int lost;
	void reset();
} _uselessstats;

typedef struct _player
{
	int number;
	int playing;
	int everplaying;
	int cash;
	int controller;
	int team;
	int tookturn;
	int tintcolor;
	_unit unit[50];
	string name;
	void reset();
	void clear_units();
	void ready_units();
	void sort_units();
	void mergesort_units(int min, int max);
	void reapply_unitstats();
	bool defeated(int pnum);
	int create_unit(int type, int x, int y, int player);
	int units_in_play();
	_uselessstats stats;
	
	ol::Bitmap *sidebar1, *sidebar2;
} _player;

class _character
{
	char *name;
	char *description;
public:
	ol::Bitmap *picture;
	void setname(const char *n);
	void setdescription(const char *d);
	void create(const char *n, const char *d, ol::Bitmap *pic);
  void destroy();
	char *getname();
	char *getdescription();

	_character();
  ~_character();
};

#endif
