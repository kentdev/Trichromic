#ifndef _BATTLESHOT_H
#define _BATTLESHOT_H

#define B_BULLET     0
#define B_CANNON     1
#define B_BALLISTIC  2  //includes shots from artillery, battleships, etc.
#define B_BALTRAIL   3  //like BALLISTIC, but with a long trail
#define B_BOMB       4
#define B_TORPEDO    5
#define B_ROCKET     6
#define B_NUKE       7  //nuclear missile and associated explosion
#define B_SNIPER     8

void create_battleshot(int type, int ox, int oy, int dx, int dy, int delay, int sound);
bool do_battleshots();
vector<_loc> bulletpoints(_unit *a, _unit *d);

typedef struct _battleshot
{
	bool exists, playedsound;
	short type, delay, time, striplen;
	int sound;
	float x, y, xv, yv, xa, ya, dx, dy;
	ol::LineStrip strip;
	ol::Rgba start, end;
} _battleshot;

#endif
