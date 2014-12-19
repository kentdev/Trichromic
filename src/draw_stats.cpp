#include "empire.h"

void draw_unitstats(_unit *u, int x, int y)
{
	char c[20];
	int h = 45;
	int i;
	unitprops *p = &unitstats[u->type];
	
	h += unitstats[u->type].loadcapacity * 15;
	
	buffer_rectfill(x, y, x + 110, y + h, BLACK, 0.3);
	buffer_rect(x, y, x + 110, y + h, WHITE);
	buffer_textout_ex(normal, p->name.c_str(), x + 5, y + 5, WHITE);
	sprintf(c, "Health: %.2f", u->health);
	buffer_textout_ex(normal, c, x + 5, y + 20, WHITE);
	
	i = 0;
	while (i < unitstats[u->type].loadcapacity)
	{
		if (u->load[i].loaded == 1)
		{
			buffer_textout_ex(normal, unit_name(u->load[i].type), x + 5, y + 35 + (15 * i), WHITE);
		}
		else
		{
			buffer_textout_ex(normal, "-", x + 5, y + 35 + (15 * i), WHITE);
		}
		i++;
	}
}
