#ifndef _particle_h
#define _particle_h

#define P_FLAME      0
#define P_DUST       1
#define P_SMOKE      2
#define P_AIRTRAIL   3
#define P_SEATRAIL   4
#define P_HORIZONTAL 5
#define P_FADETEXT   6
#define P_SHOCKWAVE  7
#define P_TELEPORT   8
#define P_SHOTHIT    9
#define P_CASH       10
#define P_FIRE       11
#define P_NUKEBOOM   12
#define P_REPAIR     13
#define P_QUADSPIRAL 14
#define P_LOGO       15

typedef struct _particle
{
  short life;
  short type;
  short exists;
	//short delay;
	int color;
	float x, y, xv, yv, xa, ya;
  string text;
  void create(int x, int y, int t, int c, string text = "");
  void draw();
  void move();
} _particle;

extern void create_particle(int x, int y, int type);
extern void create_particle(int x, int y, int type, int color, string text = "");
extern void do_particles();
extern void clear_particles();

#endif
