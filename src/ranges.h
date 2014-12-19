#ifndef _RANGES_H
#define _RANGES_H

#include "empire.h"

typedef struct _target
{
  int player;
  int unit;
}_target;

void draw_attackrange(_unit *u);
int tile_distance(int ax, int ay, int bx, int by);
int enemy_in_range(_unit *u);

#endif
