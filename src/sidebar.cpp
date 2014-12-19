#include "empire.h"
#include "logic.h"

extern int money_producing_buildings_owned(int plyr);

void do_sidebar(int p, _unit *u, bool drawonly, bool reset)
{
	const short land = 1;
	const short air  = 2;
	const short sea  = 4;
	const int leftx = 800 - MAP_TILE_SIZE * 5;
	const int minimapy = 8;
	static bool first = true, spacefirst, spacesecond, stat_display;
	static _button endturn, option, unittab[2], unload[2], tele, standby, cap, cap2, deploy, talk, stats;
	static _unit *lastunit = NULL;
	static short tab = 0, spacedelay, spacedelay2 = 0, income_updatedelay = 0;
	static int income[4], currentplayer;
	bool cancap2 = false;
	int mm_mousex, mm_mousey, i, j, k, x, y;
	char c[40];
	DyBytes data;
	
	if (reset)
	{
		first = true;
		return;
	}
	
	if (first)
	{
		lastunit = NULL;
		tab = 0;
		unittab[0].init(leftx + 140, 170, 18, 80, "");
		unittab[1].init(leftx + 140, 270, 18, 80, "");
		endturn.init(leftx + 7, 583, 73, 15, "End Turn");
		option.init(leftx + 84, 583, 73, 15, "Options");
		standby.init(leftx + 5, 390, 150, 15, "Standby");
		cap.init(leftx + 5, 410, 150, 15, "Capture");
		tele.init(leftx + 5, 430, 150, 15, "Teleport");
		deploy.init(leftx + 5, 450, 150, 15, "Deploy");
		cap2.init(leftx + 5, 470, 150, 15, "Capture Unit");
		talk.init(580, 576, 60, 24, "", 6);
		stats.init(215, 576, 60, 24, "", 6);
		first = false;
		spacefirst = false;
		spacesecond = false;
		stat_display = false;
		spacedelay = 0;  //spacedelay2 isn't reset here, on purpose
		income_updatedelay = 0;
		currentplayer = -1;
	}

	sidebarbmp.draw(p * 3 + tab, leftx, 0);
	statbmp.draw(9 + tab, leftx, 0);
	sidebarbmp.draw(p * 3 + 2, 0, 576);
	if (global.netgame) statbmp.draw(11, 0, 576);
	else statbmp.draw(12, 0, 576);
	
	sprintf(c, "Player %d", p + 1);
	buffer_textout_ex(normal, c, 15, 584, WHITE);
	sprintf(c, "Funds: $%d", player[p].cash);
	buffer_textout_ex(normal, c, 100, 584, WHITE);
	
	if (currentplayer != p)
	{
		currentplayer = p;
		i = 0;
		while (i < 4)
		{
			if (player[i].playing == 1) income[i] = money_producing_buildings_owned(i) * gameoptions.funding;
			else income[i] = 0;
			i++;
		}
		income_updatedelay = 40;
	}
	if (income_updatedelay <= 0)
	{
		i = 0;
		while (i < 4)
		{
			if (player[i].playing == 1) income[i] = money_producing_buildings_owned(i) * gameoptions.funding;
			else income[i] = 0;
			i++;
		}
		income_updatedelay = 40;
	} else income_updatedelay--;
	sprintf(c, "Income: $%d", income[p]);
	buffer_textout_ex(normal, c, 285, 584, WHITE);
	
	minimap.draw(false, leftx + 5, minimapy);
	
	if (u)
	{
		if (lastunit != u)
		{
			lastunit = u;
			i = 0;
			while (i < 2)
			{
				if (u->load[i].loaded == 1)
				{
					sprintf(c, "Unload %s", unitstats[u->load[i].type].name.c_str());
					unload[i].init(leftx + 5, 450 + 20 * i, 150, 15, c);
				}
				else
				{
					unload[i].init(-10, -10, 1, 1, "");
				}
				i++;
			}
		}
		
		if (var.logicstate == UNIT_SELECTED)
		{
			standby.check();
			if (standby.clicked())
			{
				if (global.netgame)
				{
					if (net.server)
					{
						data.clear();
						data.addByte(NET_STANDBY);
						data.addByte(pturn);
						data.addByte(u->number);
						broadcast_datablob(&data);
					}
					else
					{
						data.clear();
						data.addByte(NET_CLIENT_STANDBY);
						data.addByte(pturn);
						data.addByte(u->number);
						send_datablob(net.thread, &data);
					}
				}
				u->ready = 0;  //usually the client should wait for a server response, but standby doesn't really do anything, so why bother?
				var.unitselected = NULL;
				var.logicstate = NOTHING_HAPPENING;
			}
			
			if (unitstats[u->type].name == "Deployable Portal")
			{
				if (worldmap.tile[u->tilex][u->tiley].get_set() == 0)
				{  //if it's a land tile
					i = get_ltype(u->tilex, u->tiley);
					if ((i == 1) || (i == 6))
					{  //if it's grass or a road
						deploy.check();
						if (deploy.clicked())
						{
							if (!global.netgame)
							{
								u->exists = 0;
								u->health = 0;
								worldmap.tile[u->tilex][u->tiley].set_unit(-1, -1);
								worldmap.tile[u->tilex][u->tiley].change_type(6, 0);
								var.unitselected = NULL;
								var.logicstate = NOTHING_HAPPENING;
								create_particle(u->tilex * MAP_TILE_SIZE + MAP_TILE_SIZE / 2, u->tiley * MAP_TILE_SIZE + MAP_TILE_SIZE / 2, P_SHOCKWAVE);
							}
							else
							{
								if (net.server)
								{
									u->exists = 0;
									u->health = 0;
									worldmap.tile[u->tilex][u->tiley].set_unit(-1, -1);
									worldmap.tile[u->tilex][u->tiley].change_type(6, 0);
									create_particle(u->tilex * MAP_TILE_SIZE + MAP_TILE_SIZE / 2, u->tiley * MAP_TILE_SIZE + MAP_TILE_SIZE / 2, P_SHOCKWAVE);
									
									var.unitselected = NULL;
									var.logicstate = NOTHING_HAPPENING;
									
									data.clear();
									data.addByte(NET_DEPLOYPORTAL);
									data.addByte(pturn);
									data.addByte(u->number);
									broadcast_datablob(&data);
								}
								else
								{
									data.clear();
									data.addByte(NET_CLIENT_DEPLOY);
									data.addByte(u->number);
									send_datablob(net.thread, &data);
									
									var.unitselected = NULL;
									var.logicstate = NET_WAITING_FOR_RESPONSE;
								}
							}
						}
					}
				}
			}
			
			if (unitstats[u->type].name == "Mechanic")
			{
				i = 0;
				while (i < 4)
				{
					x = u->tilex;
					y = u->tiley;
					if (i == 0) x++;
					if (i == 1) x--;
					if (i == 2) y++;
					if (i == 3) y--;
					
					if ((x > 0) && (x < worldmap.l) && (y > 0) && (y < worldmap.h))
					{
						j = any_unit_here(x, y);
						if ((j != -1) && (player[j / 100].team != player[pturn].team))
						{
							k = player[j / 100].unit[j % 100].type;
							if ((unitstats[k].movetype != M_INFANTRY) && (unitstats[k].movetype != M_MECH) && (unitstats[k].basetype != AIR))
							{
								cancap2 = true;
							}
						}
					}
					i++;
				}
				if (cancap2)
				{
					cap2.check();
					if (cap2.clicked())
					{
						var.logicstate = MECHANIC_CAPPING;
					}
				}
			}
			
			i = 0;
			while (i < 2)
			{
				if (u->can_unload_unit(i))
				{
					unload[i].check();
					if (unload[i].clicked())
					{
						var.logicstate = UNIT_UNLOADING;
						var.unloadnum = i;
					}
				}
				i++;
			}
			
			if ((worldmap.tile[u->tilex][u->tiley].same_type(6, 0)) && (unitstats[u->type].basetype == LAND))
			{  //if the tile the unit's on is a teleport tile
				tele.check();
				if (tele.clicked())
				{
					var.logicstate = UNIT_TELEPORTING;
				}
			}
		}
		
		i = 0;
		while (i < 2)
		{
			if (i != tab)
			{
				unittab[i].check();
				if (unittab[i].clicked())
				{
					tab = i;
				}
				i++;
			}
			i++;
		}
		if (tab == 0)
		{
			buffer_textout_centre_ex(med, unitstats[u->type].name, leftx + 69, 193, BLACK);
			i = 0;
			while (i < 3)
			{
				sprintf(c, "%d", u->attack[i]);
				buffer_textout_centre_ex(normal, c, leftx - 1 + 20 * (i + 1), 216, BLACK);
				buffer_textout_centre_ex(normal, c, leftx + 20 * (i + 1), 217, WHITE);
				sprintf(c, "%d", u->resist[i]);
				buffer_textout_centre_ex(normal, c, leftx + 79 + (20 * i), 216, BLACK);
				buffer_textout_centre_ex(normal, c, leftx + 80 + (20 * i), 217, WHITE);
				i++;
			}
			
			if (unitstats[u->type].canhit & land) statbmp.draw(3, leftx + 5, 255);
			if (unitstats[u->type].canhit & air) statbmp.draw(4, leftx + 5, 285);
			if (unitstats[u->type].canhit & sea) statbmp.draw(5, leftx + 5, 315);
			
			bigunits.draw(u->type, leftx + 30, 251);
		}
		else if (tab == 1)
		{
			buffer_rectfill(leftx + 11, 180, leftx + 127, 371, WHITE, 0.5);
			buffer_textout_ex(med, unit_name(u->type), leftx + 14, 184, BLACK);
			multiline_textout_ex(normal, unitstats[u->type].description, leftx + 14, 205, 112, BLACK);
		}
	}
	
	if (mouse_b & 1)
	{  //center the map on the area of the minimap that was clicked
		mm_mousex = (mouse_x - (leftx + 5) - ((150 - minimap.bmp->Width()) / 2)) / 3;
		mm_mousey = (mouse_y - minimapy - ((150 - minimap.bmp->Height()) / 2)) / 3;
		if ((mm_mousex >= 0) && (mm_mousex < worldmap.l))
		{
			if ((mm_mousey >= 0) && (mm_mousey < worldmap.h))
			{
				worldmap.scroll_x = mm_mousex - (XTILES / 2);
				worldmap.scroll_y = mm_mousey - (YTILES / 2);
				worldmap.offset_x = 0;
				worldmap.offset_y = 0;
				if (worldmap.scroll_x < 0) worldmap.scroll_x = 0;
				if (worldmap.scroll_x + XTILES >= worldmap.l) worldmap.scroll_x = worldmap.l - XTILES;
				if (worldmap.scroll_y < 0) worldmap.scroll_y = 0;
				if (worldmap.scroll_y + YTILES >= worldmap.h) worldmap.scroll_y = worldmap.h - YTILES;
			}
		}
	}
	
	if (global.netgame)
	{
		talk.check();
		if (talk.clicked()) var.typingchat = true;
	}
	stats.check();
	if (stats.clicked())
	{
		stat_display = !stat_display;
	}
	if (stat_display)
	{
		statbmp.draw(6 + player[p].number, 65, 376);
		i = 0;
		while (i < 4)
		{
			if (player[i].everplaying == 1)
			{
				if (player[i].playing == 1)
				{
					if (global.netgame) buffer_textout_centre_ex(med, player[i].name, 140, 431 + (25 * i), WHITE);
					else
					{
						sprintf(c, "Player %d", i + 1);
						buffer_textout_centre_ex(med, c, 140, 431 + (25 * i), WHITE);
					}
					sprintf(c, "%d", player[i].cash);
					buffer_textout_centre_ex(med, c, 260, 431 + (25 * i), WHITE);
					sprintf(c, "%d", income[i]);
					buffer_textout_centre_ex(med, c, 380, 431 + (25 * i), WHITE);
					
					if (i == p)
					{
						buffer_rect(88, 432 + (25 * i), 190, 448 + (25 * i), WHITE);
					}
				}
				else
				{
					if (global.netgame) buffer_textout_centre_ex(med, player[i].name, 140, 431 + (25 * i), GREY);
					else
					{
						sprintf(c, "Player %d", i + 1);
						buffer_textout_centre_ex(med, c, 140, 431 + (25 * i), GREY);
					}
					sprintf(c, "%d", player[i].cash);
					buffer_textout_centre_ex(med, c, 260, 431 + (25 * i), GREY);
					sprintf(c, "%d", income[i]);
					buffer_textout_centre_ex(med, c, 380, 431 + (25 * i), WHITE);
				}
			}
			i++;
		}
	}
	
	if (player[p].controller == HUMAN)
	{
		endturn.check();
	}
	option.check();
	
	if (!drawonly)
	{
		if (player[p].controller == HUMAN)
		{
			if (endturn.clicked())
			{
				var.unitselected = NULL;
				var.logicstate = NOTHING_HAPPENING;
				
				if ((!global.netgame) || (net.server))
				{  //only change the turn yourself if not a network client
					pturn++;
					while (player[pturn].playing != 1)
					{
						pturn++;
						if (pturn >= 4) pturn = 0;
					}
					first = true;
					var.first = true;
				}
				
				if (global.netgame)
				{
					if (net.server)
					{
						data.clear();
						data.addByte(NET_NEWTURN);
						data.addByte(pturn);
						broadcast_datablob(&data);
					}
					else
					{
						data.clear();
						data.addByte(NET_CLIENT_ENDTURN);
						send_datablob(net.thread, &data);
					}
				}
			}
			
			if (!spacefirst)  //all this makes the turn end if the space bar is double-tapped
			{
				if (spacedelay2 > 0)
				{
					spacedelay2--;
				}
				else
				{
					if (key[KEY_SPACE])
					{
						spacefirst = true;
					}
				}
			}
			else
			{
				if (!spacesecond)
				{
					if (!key[KEY_SPACE])
					{
						spacedelay = 35;
						spacesecond = true;
					}
				}
				else
				{
					if (spacedelay > 0)
					{
						spacedelay--;
						if (key[KEY_SPACE])
						{
							spacedelay2 = 35;
							
							if (!var.typingchat)
							{  //don't let spacebar end the turn when typing
								var.unitselected = NULL;
								var.logicstate = NOTHING_HAPPENING;
								
								if ((!global.netgame) || (net.server))
								{  //only change the turn yourself if not a network client
									pturn++;
									while (player[pturn].playing != 1)
									{
										pturn++;
										if (pturn >= 4) pturn = 0;
									}
									first = true;
									var.first = true;
								}
								
								if (global.netgame)
								{
									if (net.server)
									{
										data.clear();
										data.addByte(NET_NEWTURN);
										data.addByte(pturn);
										broadcast_datablob(&data);
									}
									else
									{
										data.clear();
										data.addByte(NET_CLIENT_ENDTURN);
										send_datablob(net.thread, &data);
									}
								}
							}
						}
					}
					else
					{
						spacefirst = false;
						spacesecond = false;
					}
				}
			}
		}
		
		if ((option.clicked()) || (key[KEY_ESC]))
		{
			var.logicstate = IN_GENERAL_MENU;
		}
	}
}
