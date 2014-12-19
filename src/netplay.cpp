#include "empire.h"
#include "logic.h"

extern void get_battledata(DyBytes *data);
extern int unit_price(int type, int army);
bool valid_client_movepath(_unit *u, vector<int> movelist);

void netplayer()
{
	static bool scroll = false;
	static int scrollx, scrolly;
	
	bool err = false;
	int i, j, k, x, y, z;
	DyBytes data;
	_unit *v, *w;
	vector<int> movelist;
	
	if (scroll)
	{
		if (scroll_to_location(scrollx, scrolly) == 1) scroll = false;
		else return;
	}  //if it's in the process of scrolling somewhere, don't read in any new data yet
	
	if (net.server)
	{
		j = 0;
		while (j < 3)
		{
			if (clientconnected(j))
			{
				data = get_client_datablob(j);
				
				//The server gets data from all clients, but only acts on the ones
				//from the client who's turn it is.  This prevents data from other
				//clients (who shouldn't be sending anything when it's not their
				//turn anyway) from building up a backlog of old data.
				
				if ((j == pturn - 1) && (data.size() > 0))
				{
					i = data.getByte(0);
					data.removeFront(1);
					empdebug("Host received data");
					empdebug(i);
					empdebug("from client");
					empdebug(j);
					switch(i)
					{
						case NET_CLIENT_ENDTURN:
							pturn++;
							while (player[pturn].playing != 1)
							{
								pturn++;
								if (pturn >= 4) pturn = 0;
							}
							var.first = true;
							var.unitselected = NULL;
							var.logicstate = NOTHING_HAPPENING;
							
							data.clear();
							data.addByte(NET_NEWTURN);
							data.addByte(pturn);
							broadcast_datablob(&data);
							break;
						case NET_CLIENT_NEWUNIT:
							if (player[pturn].units_in_play() < 50)
							{
								k = data.getByte(0);
								x = data.getByte(1);
								y = data.getByte(2);
								data.removeFront(3);
								
								if (player[pturn].cash >= unit_price(k, player[pturn].number))
								{
									if ((k >= 0) && (k < NUM_UNITS) && (moves_needed(unitstats[k].movetype, x, y, pturn) < 99))
									{
										if ((x >= 0) && (x < worldmap.l) && (y >= 0) && (y < worldmap.h))
										{
											if (any_unit_here(x, y) == -1)
											{
												player[pturn].cash -= unit_price(k, player[pturn].number);
												player[pturn].create_unit(k, x, y, pturn);
												
												player[pturn].stats.spent += unit_price(k, player[pturn].number);
												player[pturn].stats.produced++;
												
												update_cities();
												
												data.clear();
												data.addByte(NET_PLAYERCASH);
												data.addByte(pturn);
												data.appendVariable(player[pturn].cash);
												broadcast_datablob(&data);
												
												data.clear();
												data.addByte(NET_NEWUNIT);
												data.addByte(pturn);
												data.addByte(k);
												data.addByte(x);
												data.addByte(y);
												broadcast_datablob(&data);
											}
											else
											{
												empdebug("ERROR: Client asked to create a unit, but there's a unit in that spot!");
												err = true;
											}
										}
										else
										{
											empdebug("ERROR: Client asked to create a unit, but gave off-map coordinates!");
											err = true;
										}
									}
									else
									{
										empdebug("ERROR: Client asked to create a unit, but the unit couldn't be built!");
										err = true;
									}
								}
								else
								{
									empdebug("ERROR: Client asked to create a unit, but didn't have enough cash!");
									err = true;
								}
							}
							else
							{
								empdebug("ERROR: Client asked to create a unit, but it has too many units in play!");
								err = true;
							}
							
							if (err)
							{  //the client will be waiting for a response after sending the "create unit" request
								data.clear();
								data.addByte(NET_OHNOYOUDONT);  //so make sure the client knows its request has been DENIED
								send_datablob(net.clientthread[j], &data);
							}
							break;
						case NET_CLIENT_MOVEUNIT:
							x = data.getByte(0);
							y = data.getByte(1);
							data.removeFront(2);
							i = any_unit_here(x, y);
							if ((i != -1) && (i / 100 == pturn))
							{
								v = &player[i / 100].unit[i % 100];
								if ((v->exists == 1) && (v->canmove))
								{
									worldmap.create_limited_pathmap(pturn, unitstats[v->type].movetype, v->tilex, v->tiley, v->move + 1);
									k = data.getByte(0);
									data.removeFront(1);
									movelist.clear();
									x = 0;
									while (x < k)
									{
										movelist.push_back(data.getByte(x));
										x++;
									}
									if (valid_client_movepath(v, movelist))
									{
										var.unitselected = v;  //necessary for the unit_moving function
										play_moving_sound(v->type);
										v->set_moves(movelist);
										v->canmove = false;
										var.logicstate = UNIT_MOVING;
										
										scroll = true;
										scrollx = v->tilex;
										scrolly = v->tiley;
										
										data.clear();
										data.addByte(NET_MOVEUNIT);
										data.addByte(v->tilex);
										data.addByte(v->tiley);
										data.addByte(movelist.size());
										x = 0;
										while ((unsigned)x < movelist.size())
										{
											data.addByte(movelist[x]);
											x++;
										}
										broadcast_datablob(&data);
									}
									else
									{
										empdebug("ERROR: Client wanted to move a unit, but the movelist was invalid!");
										err = true;
									}
								}
								else
								{
									empdebug("ERROR: Client wanted to move a unit, but the unit can't move or doesn't exist!");
									err = true;
								}
							}
							else
							{
								empdebug("ERROR: Client wanted to move a unit, but the unit wasn't there or belongs to someone else!");
								err = true;
							}
							break;
						case NET_CLIENT_STANDBY:
							i = data.getByte(1);
							if ((i >= 0) && (i < 50))
							{
								if (player[pturn].unit[i].exists == 1)
								{
									player[pturn].unit[i].ready = 0;
									
									data.clear();
									data.addByte(NET_STANDBY);
									data.addByte(pturn);
									data.addByte(i);
									broadcast_datablob(&data);
								}
							}  //standby doesn't really do anything, so even if the unit can't go into standby, don't bother with the errors
							break;
						case NET_CLIENT_DEPLOY:
							i = data.getByte(0);
							if ((i >= 0) && (i < 50))
							{
								v = &player[pturn].unit[i];
								if (unitstats[v->type].name == "Deployable Portal")
								{
									if (worldmap.tile[v->tilex][v->tiley].get_set() == 0)
									{  //if it's a land tile
										k = get_ltype(v->tilex, v->tiley);
										if ((k == 1) || (k == 6))
										{
											v->exists = 0;
											v->health = 0;
											worldmap.tile[v->tilex][v->tiley].set_unit(-1, -1);
											worldmap.tile[v->tilex][v->tiley].change_type(6, 0);
											var.unitselected = NULL;
											var.logicstate = NOTHING_HAPPENING;
											create_particle(v->tilex * MAP_TILE_SIZE + MAP_TILE_SIZE / 2, v->tiley * MAP_TILE_SIZE + MAP_TILE_SIZE / 2, P_SHOCKWAVE);
											
											scroll = true;
											scrollx = v->tilex;
											scrolly = v->tiley;
											
											data.clear();
											data.addByte(NET_DEPLOYPORTAL);
											data.addByte(pturn);
											data.addByte(i);
											broadcast_datablob(&data);
										}
									}
								}
								else
								{
									empdebug("ERROR: Client wanted to deploy a portal, but the unit wasn't a Deployable Portal!");
									err = true;
								}
							}
							else
							{
								empdebug("ERROR: Client wanted to deploy a portal, but gave an out-of-range index!");
								err = true;
							}
							break;
						case NET_CLIENT_MECHCAP:
							i = data.getByte(0);
							if ((i >= 0) && (i < 50))
							{
								v = &player[pturn].unit[i];
								x = data.getByte(1);
								y = data.getByte(2);
								if ((x > 0) && (x < worldmap.l) && (y > 0) && (y < worldmap.h))
								{
									k = any_unit_here(x, y);
									if ((k != -1) && (player[k / 100].team != player[pturn].team))
									{
										i = player[k / 100].unit[k % 100].type;
										if ((unitstats[i].movetype != M_INFANTRY) && (unitstats[i].movetype != M_MECH))
										{
											if (unitstats[i].basetype != AIR)
											{
												mechanic_capture(v, x, y);
												
												scroll = true;
												scrollx = v->tilex;
												scrolly = v->tiley;
												
												data.clear();
												data.addByte(NET_MECHANIC_CAPTURE);
												data.addByte(pturn);
												data.addByte(v->number);
												data.addByte(x);
												data.addByte(y);
												broadcast_datablob(&data);
											}
											else
											{
												empdebug("ERROR: Client wanted to do a mechanic capture, but the enemy unit was an air unit!");
												err = true;
											}
										}
										else
										{
											empdebug("ERROR: Client wanted to do a mechanic capture, but the enemy unit was an infantry type!");
											err = true;
										}
									}
									else
									{
										empdebug("ERROR: Client wanted to do a mechanic capture, but there wasn't an enemy unit there!");
										err = true;
									}
								}
								else
								{
									empdebug("ERROR: Client wanted to do a mechanic capture, but gave off-map coordinates!");
									err = true;
								}
							}
							else
							{
								empdebug("ERROR: Client wanted to do a mechanic capture, but gave an out-of-range index!");
								err = true;
							}
							break;
						case NET_CLIENT_UNLOAD:
							i = data.getByte(0);
							j = data.getByte(1);
							z = any_unit_here(i, j);
							if ((z != -1) || (z / 100 != pturn))
							{
								v = &player[z / 100].unit[z % 100];
								k = data.getByte(2);
								if ((k == 0) || (k == 1))
								{
									x = data.getByte(3);
									y = data.getByte(4);
									if (unit_unloadable_here(v->tilex, v->tiley, x, y, v->load[k].type))
									{
										unload_unit(v, k, x, y);
										
										scroll = true;
										scrollx = v->tilex;
										scrolly = v->tiley;
										
										data.clear();
										data.addByte(NET_UNLOAD);
										data.addByte(v->tilex);
										data.addByte(v->tiley);
										data.addByte(k);
										data.addByte(x);
										data.addByte(y);
										broadcast_datablob(&data);
									}
									else
									{
										empdebug("ERROR: Client wanted to unload a unit, but the unit wasn't unloadable here!");
										err = true;
									}
								}
								else
								{
									empdebug("ERROR: Client wanted to unload a unit, but gave a bad unload number!");
									err = true;
								}
							}
							else
							{
								empdebug("ERROR: Client wanted to unload a unit, but gave a tile with no unit or a unit it doesn't control!");
								err = true;
							}
							break;
						case NET_CLIENT_TELEPORT:
							i = data.getByte(0);
							if ((i >= 0) && (i < 50))
							{
								v = &player[pturn].unit[i];
								x = data.getByte(1);
								y = data.getByte(2);
								if ((x >= 0) && (x < worldmap.l) && (y >= 0) && (y < worldmap.h))
								{
									if ((worldmap.tile[x][y].same_type(6, 0)) && (worldmap.tile[v->tilex][v->tiley].same_type(6, 0)))
									{
										if (unitstats[v->type].basetype == LAND)
										{
											start_teleport_animation(v, x, y);
											
											data.clear();
											data.addByte(NET_TELEPORT);
											data.addByte(pturn);
											data.addByte(i);
											data.addByte(x);
											data.addByte(y);
											broadcast_datablob(&data);
										}
										else
										{
											empdebug("ERROR: Client wanted to teleport a unit, but it wasn't a land unit!");
											err = true;
										}
									}
									else
									{
										empdebug("ERROR: Client wanted to teleport a unit, but the tile isn't a teleporter!");
										err = true;
									}
								}
								else
								{
									empdebug("ERROR: Client wanted to teleport a unit, but gave off-map coordinates!");
									err = true;
								}
							}
							else
							{
								empdebug("ERROR: Client wanted to teleport a unit, but gave an out-of-range index!");
								err = true;
							}
							break;
						case NET_CLIENT_ATTACK:
							x = data.getByte(0);
							y = data.getByte(1);
							data.removeFront(2);
							z = any_unit_here(x, y);
							if ((z != -1) && (z / 100 == pturn))
							{
								v = &player[z / 100].unit[z % 100];
								if ((v->exists == 1) && (v->ready == 1))
								{
									x = data.getByte(0);
									y = data.getByte(1);
									data.removeFront(2);
									z = any_unit_here(x, y);
									if ((z != -1) && (z / 100 != pturn))
									{
										w = &player[z / 100].unit[z % 100];
										if (w->exists == 1)
										{
											if (player[v->color].team != player[w->color].team)
											{
												k = attackable_unit_here(v, w->tilex, w->tiley, true);
												if (k != -1)
												{  //scrolling to the battle is done in the battle function
													var.logicstate = IN_BATTLE;
													var.attacker = (v->color * 100) + v->number;
													var.defender = (w->color * 100) + w->number;
													
													//sending battle data is done in the actual battle function
												}
												else
												{
													empdebug("ERROR: Client wanted to attack, but the attacker can't hit the defender!");
													err = true;
												}
											}
											else
											{
												empdebug("ERROR: Client wanted to attack, but the defender is on the same team!");
												err = true;
											}
										}
										else
										{
											empdebug("ERROR: Client wanted to attack, but the defender doesn't exist!");
											err = true;
										}
									}
									else
									{
										empdebug("ERROR: Client wanted to attack, but gave a tile with no defender!");
										err = true;
									}
								}
								else
								{
									empdebug("ERROR: Client wanted to attack, but the attacker isn't ready or doesn't exist!");
									err = true;
								}
							}
							else
							{
								empdebug("ERROR: Client wanted to attack, but gave a tile with no unit or a unit it doesn't control!");
								err = true;
							}
							break;
						case NET_CLIENT_SURRENDER:
							x = (signed char)data.getByte(0);
							if (player[pturn].playing == 1)
							{
								if ((x >= -1) && (x < 4))
								{
									if (player[x].playing == 1)
									{
										do_surrender(pturn, x);
										
										data.clear();
										data.addByte(NET_PLAYER_SURRENDER);
										data.addByte(pturn);
										data.addByte(x);
										broadcast_datablob(&data);
									}
									else
									{
										empdebug("Error: Client wanted to surrender to a player that isn't playing!");
										err = true;
									}
								}
								else
								{
									empdebug("ERROR: The client wanted to surrender, but gave an out-of-range index!");
									err = true;
								}
							}
							else
							{
								empdebug("ERROR: The client wanted to surrender, but is no longer playing!");
								err = true;
							}
							break;
					}
					if (err)
					{  //the client will be waiting for a response after sending the request
						data.clear();
						data.addByte(NET_OHNOYOUDONT);  //so make sure the client knows its request has been DENIED
						send_datablob(net.clientthread[j], &data);
					}
				}  //end of "if j == pturn - 1 && data.size > 0"
			}  //end of "if clientconnected(j)"
			j++;
		}
	}
	else
	{
		data = get_host_datablob();
		if (data.size() > 0)
		{
			i = data.getByte(0);
			data.removeFront(1);
			empdebug("Received data:");
			empdebug(i);
			
			if (var.logicstate == NET_WAITING_FOR_RESPONSE)
			{  //if this client was waiting for a response from the server, here it is
				var.logicstate = NOTHING_HAPPENING;
			}
			
			switch(i)
			{
				case NET_PLAYERGONE:
					j = data.getByte(0);
					if (player[j].playing == 1)
					{
						player[j].playing = 0;
						transform_buildings(j, -1);
						var.logicstate = PLAYER_DEFEATED;
						var.theplayer = j;
					}
					break;
				case NET_NEWTURN:
					do_sidebar(0, NULL, false, true);  //reset sidebar
					j = data.getByte(0);
					pturn = j;
					var.first = true;
					var.unitselected = NULL;
					var.logicstate = NOTHING_HAPPENING;
					break;
				case NET_PLAYERCASH:
					j = data.getByte(0);
					data.extractVariable(1, player[j].cash);
					break;
				case NET_NEWUNIT:
					j = data.getByte(0);
					k = data.getByte(1);
					x = data.getByte(2);
					y = data.getByte(3);
					z = player[j].create_unit(k, x, y, j);
					
					player[j].stats.produced++;
					player[j].stats.spent += player[j].unit[z].price;
					
					update_cities();
					break;
				case NET_MOVEUNIT:
					movelist.clear();
					x = data.getByte(0);
					y = data.getByte(1);
					data.removeFront(2);
					k = any_unit_here(x, y);
					v = &player[k / 100].unit[k % 100];
					k = data.getByte(0);
					data.removeFront(1);
					j = 0;
					while (j < k)
					{
						movelist.push_back(data.getByte(j));
						j++;
					}
					
					scroll = true;
					scrollx = v->tilex;
					scrolly = v->tiley;
					
					var.unitselected = v;  //necessary for the unit_moving function
					v->set_moves(movelist);
					v->canmove = false;
					var.logicstate = UNIT_MOVING;
					play_moving_sound(v->type);
					break;
				case NET_STANDBY:
					j = data.getByte(0);
					k = data.getByte(1);
					player[j].unit[k].ready = 0;
					break;
				case NET_DEPLOYPORTAL:
					j = data.getByte(0);
					k = data.getByte(1);
					v = &player[j].unit[k];
					
					scroll = true;
					scrollx = v->tilex;
					scrolly = v->tiley;
					
					v->exists = 0;
					v->health = 0;
					worldmap.tile[v->tilex][v->tiley].set_unit(-1, -1);
					worldmap.tile[v->tilex][v->tiley].change_type(6, 0);
					create_particle(v->tilex * MAP_TILE_SIZE + MAP_TILE_SIZE / 2, v->tiley * MAP_TILE_SIZE + MAP_TILE_SIZE / 2, P_SHOCKWAVE);
					break;
				case NET_MECHANIC_CAPTURE:
					j = data.getByte(0);
					k = data.getByte(1);
					x = data.getByte(2);
					y = data.getByte(3);
					
					scroll = true;
					scrollx = player[j].unit[k].tilex;
					scrolly = player[j].unit[k].tiley;
					
					mechanic_capture(&player[j].unit[k], x, y);
					break;
				case NET_UNLOAD:
					j = data.getByte(0);
					k = data.getByte(1);
					z = any_unit_here(j, k);
					v = &player[z / 100].unit[z % 100];
					j = data.getByte(2);
					x = data.getByte(3);
					y = data.getByte(4);
					unload_unit(v, j, x, y);
					
					scroll = true;
					scrollx = v->tilex;
					scrolly = v->tiley;
					
					if (player[j].controller == HUMAN)
					{  //if it was this client that asked to unload a unit
						var.logicstate = UNIT_SELECTED;
					}
					break;
				case NET_TELEPORT:
					j = data.getByte(0);
					k = data.getByte(1);
					x = data.getByte(2);
					y = data.getByte(3);
					start_teleport_animation(&player[j].unit[k], x, y);
					break;
				case NET_BATTLEDATA:
					var.logicstate = IN_BATTLE;
					get_battledata(&data);
					empdebug("Entering battle");
					break;
				case NET_PLAYER_SURRENDER:
					x = (signed char)data.getByte(0);
					y = (signed char)data.getByte(1);
					do_surrender(x, y);
					break;
				case NET_OHNOYOUDONT:
					empdebug("ERROR!  Tried to do something, but the server wouldn't allow it!");
					break;
			}
		}
	}
}

bool valid_client_movepath(_unit *u, vector<int> movelist)
{
	unsigned int i = 0;
	int x = u->tilex, y = u->tiley, z;
	
	while (i < movelist.size())
	{
		switch(movelist[i])
		{
			case UP:
				y--;
				break;
			case DOWN:
				y++;
				break;
			case LEFT:
				x--;
				break;
			case RIGHT:
				x++;
				break;
		}
		i++;
	}
	
	z = any_unit_here(x, y);
	if (z == -1) return true;
	if (can_merge(u, &player[z / 100].unit[z % 100])) return true;
	if (player[z / 100].unit[z % 100].can_load_unit(u)) return true;
	
	return false;
}

void logic_netchat(string message)
{
	static bool first = true, holdingenter;
	static int chattimer;
	static float chatalpha;
	static _listbox chatbox;
	static _textline chattext;
	string chatstr;
	
	if (first)
	{
		chatbox.init(0, YTILES * MAP_TILE_SIZE - 135, 620, 121);
		chattext.init(0, YTILES * MAP_TILE_SIZE - 13, 479, 13, true);
		chatalpha = 0.0;
		chattimer = 0;
		holdingenter = false;
		first = false;
	}
	
	if (message != "")
	{
		chatbox.additem(message.c_str());
		chatbox.scroll_to_end();
		chatalpha = 1.0;
		return;
	}
	
	if (var.typingchat)
	{
		chatbox.draw(0.7, true, true);
		chatstr = chattext.check(0.7);
	}
	else
	{
		if (holdingenter)
		{
			if (!key[KEY_ENTER])
			{
				holdingenter = false;
			}
		}
		else
		{
			if (key[KEY_ENTER]) var.typingchat = true;
		}
		chattext.draw(0.2);
		clear_keybuf();
		chatstr = "/NOTHING YET/";
	}
	
	if (chatstr != "/NOTHING YET/")
	{
		chatalpha = 1.0;
		chattimer = 0;
		var.typingchat = false;
		holdingenter = true;
	}
	else
	{
		if (chattimer > 150)
		{
			chatalpha -= 0.005;
			if (chatalpha < 0) chatalpha = 0;
		}
		else chattimer++;
	}
	
	chatbox.draw(chatalpha, true, true);
	if (net.server)
	{
		if (do_hostchat(&chatbox, chatstr))
		{
			chatalpha = 1.0;
			chattimer = 0;
		}
	}
	else
	{
		if (do_clientchat(&chatbox, chatstr))
		{
			chatalpha = 1.0;
			chattimer = 0;
		}
	}
}

int reconnect()
{
	static bool first = true, nohost, pollingerror, isclient, waiting, nobodyleft, allconnected;
	static int timer = 0;
	static _button cancel;
	int i, j, k;
	DyBytes data;
	
	if (first)
	{
		pollingerror = false;
		nohost = false;
		waiting = false;
		first = false;
		nobodyleft = false;
		timer = 0;
		
		net.hostnum++;
		
		while (player[net.hostnum].controller == NETAI) net.hostnum++;
		if (net.hostnum >= 4)
		{
			nohost = true;
		}
		else if (player[net.hostnum].controller == HUMAN)
		{  //if the player designated to be the next host is a human player on this machine
			j = 0;
			i = 0;
			while (i < 4)
			{
				if (player[i].playing == 1)
				{
					j++;
					if ((player[i].controller == NETAI) || (i < net.hostnum)) player[i].controller = AI;
				}
				i++;
			}
			if (j == 0)
			{  //no other players besides the new host are left in the game
				nobodyleft = true;
				cancel.init(240, 450, 180, 40, "Return to Netplay Menu");
			}
			else cancel.init(240, 450, 180, 40, "Stop Waiting");
			allconnected = false;
			
			net.server = true;
			net.engine->disconnectThread(net.thread);
			net.chatengine->disconnectThread(net.chatthread);
			net.engine->enableTCPListen(41136);
			net.chatengine->enableTCPListen(41137);
			
			net.clientthread[0] = -1;
			net.clientthread[1] = -1;
			net.clientthread[2] = -1;
		}
		else
		{
			cancel.init(240, 450, 180, 40, "Exit to Netplay Menu");
			if (net.address[net.hostnum] != "")
			{
				net.engine->disconnectThread(net.thread);
				net.chatengine->disconnectThread(net.chatthread);
				net.poll.connectToHost(net.address[net.hostnum], 41136, net.engine);
				net.chatpoll.connectToHost(net.address[net.hostnum], 41137, net.chatengine);
				net.thread = -1;
				net.chatthread = -1;
			}
			else
			{  //next designated host doesn't have an address - game over
				nohost = true;
			}
		}
	}
	
	timer++;
	
	buffer_rectfill(0, 0, 800, 600, BLACK, 0.65);
	cancel.check();
	
	buffer_textout_centre_ex(big, "Lost connection to host", 320, 100, WHITE);
	if (net.server)
	{
		buffer_textout_centre_ex(big, "You are the new host.", 320, 200, WHITE);
		
		if (nobodyleft)
			buffer_textout_centre_ex(big, "No other players left.  Can't continue the game.", 320, 250, WHITE);
		else
		{
			buffer_textout_centre_ex(big, "Waiting for the other clients to connect...", 320, 250, WHITE);
			
			allconnected = true;
			i = 0;
			while (i < 4)
			{
				if ((player[i].playing == 1) && (player[i].controller == NET))
				{
					buffer_textout_right_ex(big, (player[i].name + ": ") + net.address[i], 310, 280 + (20 * i), WHITE);
					if (clientconnected(i - 1)) buffer_textout_ex(big, "Connected", 330, 280 + (20 * i), GREEN);
					else
					{
						buffer_textout_ex(big, "Not Connected Yet", 330, 280 + (20 * i), WHITE);
						allconnected = false;
					}
				}
				else if ((player[i].playing == 1) && (player[i].controller != HUMAN))
				{
					buffer_textout_right_ex(big, player[i].name, 310, 280 + (20 * i), WHITE);
					buffer_textout_ex(big, "Controlled by AI", 330, 280 + (20 * i), WHITE);
				}
				i++;
			}
			
			if (allconnected)
			{
				buffer_textout_centre_ex(big, "All Players Connected / Transferred to AI", 320, 500, WHITE);
				if (cancel.get_text() != "Continue Game") cancel.init(240, 450, 180, 40, "Continue Game");
			}
			else buffer_textout_centre_ex(med, "Hitting \"Stop Waiting\" will replace any not-yet-reconnected players with AI", 320, 500, WHITE);
			
			i = 0;
			while (i < net.engine->getMaxConnections())
			{
				if (net.engine->isConnected(i))
				{
					if (net.engine->isNewClient(i))
					{
						isclient = false;
						j = net.hostnum + 1;
						while (j < 4)
						{
							if (net.engine->getRemoteAddr(i) == net.address[j])
							{
								isclient = true;
								k = 0;
								while (clientconnected(k)) k++;
								if (k < 3)
									net.clientthread[k] = i;
								else
									net.engine->disconnectThread(i);
							}
							j++;
						}
						if (!isclient)
						{  //if whoever just connected wasn't one of the clients in the game
							net.engine->disconnectThread(i);
						}
					}
				}
				i++;
			}
		}
		
		if (cancel.clicked())
		{
			net.engine->disableTCPListen();
			net.chatengine->disableTCPListen();
			if (nobodyleft)
			{
				net.thread = -1;
				net.chatthread = -1;
				net.clientthread[0] = -1;
				net.clientthread[1] = -1;
				net.clientthread[2] = -1;
				first = true;
				return -1;
			}
			else
			{
				i = net.hostnum + 1;
				while (i < 4)
				{
					if (player[i].playing == 1)
					{  //make sure everyone has the right controller data
						//replace players that haven't connected with AI
						if ((i > 0) && (!clientconnected(i - 1))) player[i].controller = AI;
						
						data.clear();
						data.addByte(NET_NEWCONTROLLER);
						data.addByte(i);
						
						if (player[i].controller == HUMAN) data.addByte(NET);
						else if (player[i].controller == AI) data.addByte(NETAI);
						else data.addByte(NET);
						broadcast_datablob(&data);
					}
					i++;
				}
				data.clear();
				data.addByte(NET_RESUMEGAME);
				broadcast_datablob(&data);
				first = true;
				return 1;
			}
		}
	}
	else  //not the new host
	{
		if (nohost)
		{
			buffer_textout_centre_ex(big, "Error: There is no new host.  Can't continue the game.", 320, 200, WHITE);
			
			if (cancel.clicked())
			{
				first = true;
				return -1;
			}
		}
		else if (pollingerror)
		{
			buffer_textout_centre_ex(big, "Error connecting to the new host.  Can't continue the game.", 320, 200, WHITE);
			if (cancel.clicked())
			{
				first = true;
				return -1;
			}
		}
		else
		{
			waiting = false;
			
			if (net.thread != -1)
			{
				waiting = true;
				i = net.poll.checkConnection();
				if (i >= 0)
				{
					net.thread = i;
				}
				else if (i != DY_BUSYUNREACHABLE)
				{
					net.poll.cancel();
					net.chatpoll.cancel();
					pollingerror = true;
				}
			}
			if (net.chatthread != -1)
			{
				waiting = true;
				i = net.chatpoll.checkConnection();
				if (i >= 0)
				{
					net.chatthread = i;
				}
				else if (i != DY_BUSYUNREACHABLE)
				{
					net.poll.cancel();
					net.chatpoll.cancel();
					pollingerror = true;
				}
			}
			
			if (waiting) buffer_textout_centre_ex(big, "Attempting to connect to the new host...", 320, 200, WHITE);
			else
			{
				buffer_textout_centre_ex(big, "Connected to new host!", 320, 200, WHITE);
				buffer_textout_centre_ex(big, "Game will continue when the host stops waiting for others...", 320, 220, WHITE);
				
				data = get_host_datablob();
				if (data.size() > 0)
				{
					if (data.getByte(0) == NET_NEWCONTROLLER)
					{
						if (data.getByte(1) != net.client_pnum)
						{  //our player's controller is always human, so ignore anything to the contrary
							player[data.getByte(1)].controller = data.getByte(2);
						}
					}
					else if (data.getByte(0) == NET_RESUMEGAME)
					{
						first = true;
						return 1;
					}
				}
			}
			
			if (cancel.clicked())
			{
				if (net.thread != -1)
				{
					net.engine->disconnectThread(net.thread);
					net.thread = -1;
				}
				if (net.chatthread != -1)
				{
					net.engine->disconnectThread(net.chatthread);
					net.chatthread = -1;
				}
				first = true;
				return -1;
			}
			
		}
	}
	
	return 0;
}
