#include "empire.h"

extern void text_box(int x, int y, int w, int h, char *ret, int leng, int spaces, string title);
extern int unit_here_thorough(int plyr, int tx, int ty, int ignore_n);
extern int attackable_unit_here(_unit *attacker, int tx, int ty);
extern int attackable_unit_here(_unit *attacker, int tx, int ty, bool ignore_teams);
extern int any_unit_here(int tx, int ty);
extern int unit_price(int type, int army);

extern string unit_name(int type);
extern int unit_type(string name);

int random_buildpos(bool *buildarray);

int do_unitmenu(int type, int tx, int ty, int pturn)
{
	const int x = 160, y = 70;
  static bool first = true;
  static bool ok[MAX_UNITS];  //each ok[] corresponds to a unit type
  static _button items[15];  //maximum number of buttons possible at once is 15
  static _button pos[9];
  static int lastitem, unittype, buildpos;
  static bool limit, buildable[9];
	bool drewbuildpos;
  char tempchar[30];
  int i, j, bx, by;
  DyBytes data;
	
  if (first)
  {
    limit = false;
		
		i = 0;
		while (i < 9)
		{
			bx = tx + (i % 3) - 1;
			by = ty + (i / 3) - 1;
			buildable[i] = true;
			if ((bx < 0) || (bx >= worldmap.l)) buildable[i] = false;
			if ((by < 0) || (by >= worldmap.h)) buildable[i] = false;
			if (any_unit_here(bx, by) != -1) buildable[i] = false;
			i++;
		}
		buildpos = random_buildpos(&buildable[0]);
		
		i = 0;
		while (i < NUM_UNITS)
		{
			ok[i] = false;
			switch(type)
			{
				case BASE:
					if (unitstats[i].basetype == LAND) ok[i] = true;
					break;
				case AIRPORT:
					if (unitstats[i].basetype == AIR) ok[i] = true;
					break;
				case PORT:
					if (unitstats[i].basetype == SEA) ok[i] = true;
					break;
			}
			if (!unitstats[i].isarmy[player[pturn].number])
			{  //if the player's army can't build this unit
				ok[i] = false;
			}
			if (unitstats[i].techlevel > var.techlevel)
			{
				ok[i] = false;
			}
      i++;
    }
    if (player[pturn].units_in_play() >= 50)  //if the player cannot make more units
    {
      i = 0;
      while (i < NUM_UNITS)
      {
        ok[i] = false;
        i++;
      }
      buffer_textout_ex(normal, "Unit Limit Reached", 50, 290, BLACK);
      limit = true;
    }
    i = 0;
    j = 0;
    while (i < NUM_UNITS)
    {
      if (ok[i])
      {
        items[j].init(x + 30, y + 78 + (20 * j), 100, 15, unit_name(i));
        j++;
      }
      i++;
    }
    items[j].init(x + 8, y + 10, 22, 22, "");  //close button
    lastitem = j;
    position_mouse(220, 195);
    
    j = 0;
    while (j < 3)
    {
    	i = 0;
    	while (i < 3)
    	{
    		pos[i + j * 3].init(x + 80 + 19 * i, y + 10 + 19 * j, 18, 18);
    		i++;
    	}
    	j++;
    }
    
    first = false;
  }
	buffer_draw_sprite(glb.bmp(35), x, y, 0.9);

	if (!limit)
	{
		i = 0;
		while (i < 9)
		{
			if (buildable[i])
			{
				pos[i].check();
				if (pos[i].clicked())
				{
					buildpos = i;
				}
			}
			else
			{
				buffer_rectfill(x + 80 + (i % 3) * 19, y + 10 + (i / 3) * 19, x + 98 + (i % 3) * 19, y + 28 + (i / 3) * 19, makecol(128, 0, 0), 0.75);
			}
			i++;
		}
	}
	drewbuildpos = false;
	
  j = 0;
  i = 0;
	while ((i < lastitem) && (!limit))
	{
		items[i].check();
		if (items[i].mouseover() == 1)
		{
			j = 1;
			unittype = unit_type(items[i].get_text());
			buffer_textout_centre_ex(med, unit_name(unittype), x + 225, y + 35, BLACK, 0.7);
			sprintf(tempchar, "Price: %-1d", unit_price(unittype, player[pturn].number));
			if (unit_price(unittype, player[pturn].number) <= player[pturn].cash)
			{
				buffer_textout_centre_ex(normal, tempchar, x + 225, y + 55, BLACK);
			}
			else
			{
				buffer_textout_centre_ex(normal, tempchar, x + 225, y + 55, RED);
			}
			bigunits.draw(unittype, x + 175, y + 95);
			multiline_textout_ex(normal, unitstats[unittype].description, x + 160, y + 215, 130, BLACK);
			
			bx = tx + (buildpos % 3) - 1;
			by = ty + (buildpos / 3) - 1;
			
			if ((any_unit_here(bx, by) == -1) && (bx >= 0) && (bx < worldmap.l) && (by >= 0) && (by < worldmap.h) &&
					(moves_needed(unitstats[unittype].movetype, bx, by, pturn) < 99))
			{  //if the selected unit can be built on the selected tile
				buffer_rectfill(x + 80 + (buildpos % 3) * 19, y + 10 + (buildpos / 3) * 19, x + 98 + (buildpos % 3) * 19, y + 28 + (buildpos / 3) * 19, GREEN, 0.75);
			}
			else
			{
				buffer_rectfill(x + 80 + (buildpos % 3) * 19, y + 10 + (buildpos / 3) * 19, x + 98 + (buildpos % 3) * 19, y + 28 + (buildpos / 3) * 19, RED, 0.75);
			}
			drewbuildpos = true;
		}
		if (items[i].clicked())
		{
			if (player[pturn].cash >= unit_price(unittype, player[pturn].number))
			{
				bx = tx + (buildpos % 3) - 1;
				by = ty + (buildpos / 3) - 1;
				if (moves_needed(unitstats[unittype].movetype, bx, by, pturn) < 99)
				{
					if ((bx >= 0) && (bx < worldmap.l) && (by >= 0) && (by < worldmap.h) && (any_unit_here(bx, by) == -1))
					{
						if (!global.netgame)
						{
							player[pturn].cash -= unit_price(unittype, player[pturn].number);
							player[pturn].create_unit(unittype, bx, by, pturn);
							
							player[pturn].stats.produced++;
							player[pturn].stats.spent += unit_price(unittype, player[pturn].number);
						}
						else
						{
							if (!net.server)
							{  //if it's not the network host, it just sends a request for a new unit here
								data.clear();
								data.addByte(NET_CLIENT_NEWUNIT);
								data.addByte(unittype);
								data.addByte(bx);
								data.addByte(by);
								send_datablob(net.thread, &data);
							}
							else
							{
								player[pturn].cash -= unit_price(unittype, player[pturn].number);
								player[pturn].create_unit(unittype, bx, by, pturn);
								
								player[pturn].stats.spent += unit_price(unittype, player[pturn].number);
								player[pturn].stats.produced++;
								
								data.clear();
								data.addByte(NET_PLAYERCASH);
								data.addByte(pturn);
								data.appendVariable(player[pturn].cash);
								broadcast_datablob(&data);
								
								data.clear();
								data.addByte(NET_NEWUNIT);
								data.addByte(pturn);
								data.addByte(unittype);
								data.addByte(bx);
								data.addByte(by);
								broadcast_datablob(&data);
							}
						}
						position_mouse(var.oldmx, var.oldmy);
						first = true;
						return 1;
					}
				}
      }
    }
    i++;
  }
	if ((!limit) && (!drewbuildpos))
	{
		buffer_rectfill(x + 80 + (buildpos % 3) * 19, y + 10 + (buildpos / 3) * 19, x + 98 + (buildpos % 3) * 19, y + 28 + (buildpos / 3) * 19, WHITE, 0.75);
	}
  
  if (limit)
  {  //the 50-unit limit has been hit
    buffer_textout_ex(normal, "Unit Limit Reached", x + 160, y + 215, RED);
  }
  items[lastitem].check();
  if (items[lastitem].clicked())  //cancel button
  {
    //set_mouse_range(0, 0, 639, 479);
    position_mouse(var.oldmx, var.oldmy);
    first = true;
    return -1;
  }
  return 0;
}

string do_generalmenu()
{
	static bool first = true, surrendering = false;
	static _button quit, save, option, cancel, help, surrender[6];
	int i, col;
	char c[40];
	
	if (first)
	{
		save.init(270, 160, 100, 20, "Save Game");
		option.init(270, 200, 100, 20, "Game Options");
		help.init(270, 240, 100, 20, "Help");
		surrender[0].init(270, 280, 100, 20, "Surrender");
		quit.init(270, 320, 100, 20, "Quit to Menu");
		cancel.init(270, 380, 100, 20, "Return to Game");
		
		surrendering = false;
		first = false;
	}
	
	if (!surrendering)
	{
		buffer_rectrounded(220, 110, 420, 430, GREY, 0.85, 10.0);
		buffer_textout_centre_ex(big, "Options", 320, 120, BLACK, 0.9);
		
		if (!global.netgame)
		{
			save.check();
		}
		
		option.check();
		quit.check();
		help.check();
		cancel.check();
		if (player[pturn].controller == HUMAN) surrender[0].check();
		else surrender[0].draw_disabled();
		
		if (quit.clicked())
		{
			return "Quit";
		}
		if (option.clicked())
		{
			return "Options";
		}
		if ((player[pturn].controller == HUMAN) && (surrender[0].clicked()))
		{
			i = 0;
			while (i < numplayers)
			{
				if ((i != pturn) && (player[i].playing == 1))
				{
					if (global.netgame) sprintf(c, "Surrender to %s", player[i].name.c_str());
					else sprintf(c, "Surrender to Player %d", i + 1);
					surrender[i].init(210, 160 + (30 * i), 220, 20, c);
				}
				i++;
			}
			surrender[i].init(210, 160 + (30 * i), 220, 20, "Surrender");
			if (!global.netgame) surrender[i + 1].init(210, 210 + (30 * i), 220, 20, "Give Control to AI");
			cancel.init(250, 380, 140, 20, "Don't Surrender");
			surrendering = true;
		}
		if (help.clicked())
		{
			return "Help";
		}
		if ((!global.netgame) && (save.clicked()))
		{
			return "Save";
		}
		if ((cancel.clicked()) || (mouse_rclicked() == 1))
		{
			first = true;
			return "Cancel";
		}
	}
	else  //surrendering
	{
		buffer_rectrounded(180, 110, 460, 430, GREY, 0.85, 10.0);
		buffer_textout_centre_ex(big, "Surrender", 320, 120, BLACK, 0.9);
		
		i = 0;
		while (i < numplayers)
		{
			if ((i != pturn) && (player[i].playing == 1))
			{
				col = player[i].tintcolor;
				buffer_rectrounded(surrender[i].x, surrender[i].y, surrender[i].x + surrender[i].w, surrender[i].y + surrender[i].h, col, 1.0, 2.0);
				surrender[i].check();
				
				if (surrender[i].clicked())
				{
					global.surrenderval = i;
					first = true;
					return "Surrender";
				}
			}
			i++;
		}
		buffer_rectrounded(surrender[i].x, surrender[i].y, surrender[i].x + surrender[i].w, surrender[i].y + surrender[i].h, WHITE, 0.3, 2.0);
		surrender[i].check();
		if (surrender[i].clicked())
		{
			global.surrenderval = -1;
			first = true;
			return "Surrender";
		}
		if (!global.netgame)
		{
			surrender[i + 1].check();
			if (surrender[i + 1].clicked())
			{
				player[pturn].controller = AI;
				first = true;
				return "Cancel";
			}
		}
		
		cancel.check();
		if (cancel.clicked())
		{
			surrender[0].init(270, 280, 100, 20, "Surrender");
			cancel.init(270, 380, 100, 20, "Return to Game");
			surrendering = false;
		}
	}
	
	return "-";
}

int do_unloadmenu(_unit *u)
{
  static _button b[3];
  static int first = 1;
  static int oldmx, oldmy;
  if (first == 1)
  {
    oldmx = mouse_x;
    oldmy = mouse_y;
    if (u->load[0].loaded == 1)
    {
      b[0].init(502, 2, 135, 28, unit_name(u->load[0].type).c_str());
    }
    else
    {
      b[0].init(502, 2, 135, 28, "-");
    }
    if (u->load[1].loaded == 1)
    {
      b[1].init(502, 32, 135, 28, unit_name(u->load[1].type).c_str());
    }
    else
    {
      b[1].init(502, 32, 135, 28, "-");
    }
    b[2].init(502, 62, 135, 28, "Cancel");
    position_mouse(570, 15);
    //set_mouse_range(500, 0, 640, 92);
    first = 0;
  }
  buffer_menuback(500, 0, 639, 92, DGREY, GREY, LGREY, 0.8);
  
  b[0].check();
  b[1].check();
  b[2].check();
  if ((b[0].clicked()) && (b[0].get_text() != "-"))
  {
    //set_mouse_range(0, 0, 639, 479);
    position_mouse(oldmx, oldmy);
    first = 1;
    return 0;
  }
  if ((b[1].clicked()) && (b[1].get_text() != "-"))
  {
    //set_mouse_range(0, 0, 639, 479);
    position_mouse(oldmx, oldmy);
    first = 1;
    return 1;
  }
  if (b[2].clicked())
  {
    //set_mouse_range(0, 0, 639, 479);
    position_mouse(oldmx, oldmy);
    first = 1;
    return 2;
  }
  return -1;
}

int do_savemenu()
{
  static bool first = true;
  static _button slot[10];
  int i;
  char path[20];
  char text[20];
  char name[20] = "";

  if (first)
  {
    buffer_textout_centre_ex(normal, "Choose a save slot", 320, 93, BLACK);
    position_mouse(320, 245);
    //set_mouse_range(239, 84, 401, 391);
    i = 0;
    while (i < 9)
    {
      sprintf(path, "saves/game%-1d.sav", i);
      if (exists(path) != 0)
			{
				ifstream n(path);
				n >> text;
				slot[i].init(250, 140 + (i * 25), 140, 20, text);
				n.close();
			}
			else
			{
				slot[i].init(250, 140 + (i * 25), 140, 20, "Empty");
			}
      i++;
    }
    slot[9].init(250, 390, 140, 20, "Cancel");
    first = false;
  }
  //buffer_menuback(239, 84, 401, 391, DGREY, GREY, LGREY, 0.8);
  buffer_rectrounded(220, 100, 420, 440, GREY, 0.85, 10.0);
  buffer_textout_centre_ex(normal, "Choose a save slot", 320, 123, BLACK);
  
  i = 0;
  while (i < 10)
  {
    slot[i].check();
    if (slot[i].clicked())
    {
      if (i == 9)  //Cancel was clicked
      {
        first = 1;
        //set_mouse_range(0, 0, 639, 479);
        return 0;
      }
      else
      {
        if (slot[i].get_text() != "Empty")
        {
          sprintf(name, slot[i].get_text().c_str());
        }
        text_box(240, 200, 161, 35, name, 15, 0, "Name your saved game");
        if (strcmp(name, "_CANCEL_") == 0)
        {  //if ESC was pressed in text_box, don't save
          first = 1;
          //set_mouse_range(0, 0, 639, 479);
          return 0;
        }
        sprintf(path, "saves/game%-1d.sav", i);
        ofstream savefile(path);
        if (!(savefile.bad()))
        {
          savefile << name << "\n" << worldmap.l << "\n" << worldmap.h << "\n";

					save_map_and_players(&savefile);
					
					savefile << "pturn " << pturn << "\n" << gameoptions.funding << "\n";
					savefile << gameoptions.show_damage << "\n" << gameoptions.time << "\n";
					savefile << var.time << "\n" << var.techlevel << "\n" << var.cashturns << "\n";
					savefile << worldmap.get_name().c_str();
					
          savefile.close();
        }
        first = 1;
        //set_mouse_range(0, 0, 639, 479);
        counter = 0;  //the saveprocess might take a while, so reset the counter
        return 0;
      }
    }
    i++;
  }
  return 1;  //returns 1 if it isn't finished
}

int random_buildpos(bool *buildarray)
{
	int i;
	vector<int> positions;
	
	i = 0;
	while (i < 9)
	{
		if (buildarray[i])
		{
			if (i != 4)
			{
				positions.push_back(i);
			}
		}
		i++;
	}
	
	if (positions.size() > 0)
	{
		return positions[rand()%positions.size()];
	}
	return 4;
}
