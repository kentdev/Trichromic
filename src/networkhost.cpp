#include "empire.h"

extern void get_connected_beaches();
extern void draw_minimap(int x, int y, string file);
extern void load_army_settings();
extern void load_unit_settings();

void send_lobbydata(UDPSock *socket, unsigned char players);
void send_inidata(int thread);

void broadcast_unreadyall(bool *readystatus);

void send_playerdata(int thread, int placement, bool *isai);
bool load_client_playerdata(int p, DyBytes *data);

DyBytes menudata();
void send_menudata(int thread);
void broadcast_menudata();

void do_netai_options(_button *b, bool *isai, bool *pready, int *colors, _button *ccycle, _button *pcycle, _button *tcycle);
void remove_player(int j, bool *pready, int *colors, bool *isai, _button *ccyc, _button *pcyc, _button *tcyc);
void assign_tintcolor(int p, int *colors, bool *isai);
int assign_new_client(int i, bool *isai);

int net_hostgame()
{
	static bool first = true, changemap = false, pmaploaded[3], pready[3], udpOK, isai[3];
	static int oldsel, prevmap, colors[4], last_lobby_update, baddata[3];
	static _button team, plyrcycle[4], teamcycle[4], fundsperbuilding, showdamage, startingcash, cashturns,
									back, mapchange, mapok, mapcancel, start, techlevel, colorcycle[4], boot[3], ai[3];
	static _listbox chatbox, mapbox;
	static _textline chattext;
	static UDPSock udp;
	bool notnumplayers = true;
	bool allready = false;
	bool pcycleclicked[4], pcyclerclicked[4], tcycleclicked[4], tcyclerclicked[4], ccycleclicked[4], ccyclerclicked[4];
	char c[70];
	int i, j, k;
	string str, s;
	DyBytes data;
	
	if (first)
	{
		play_song(battlemenu_music);
		load_army_settings();  //this machine may have been a network client previously
		load_unit_settings();  //so make sure the unit and army settings are the local ones
		
		if (!unlisted_game)
		{
			udpOK = udphandler.open();//udp.openSocket(41138);
		} else udpOK = false;
		
		last_lobby_update = 250;
		
		gameoptions.funding = 4000;
		gameoptions.show_damage = true;
		gameoptions.time = -999;
		oldsel = -1;
		changemap = false;
		
		i = 0;
		while (i < 4)
		{
			currentcharacter[i] = character[0];
			player[i].number = 0;
			player[i].team = i + 1;
			player[i].tintcolor = player_color(0, i);
			colors[i] = i;
			
			plyrcycle[i].init(44, 42 + 116 * i, 80, 80, "", character[0]->picture);
			teamcycle[i].init(150, 70 + 116 * i, 40, 40, "", glb.bmp(13 + player[i].team));
			colorcycle[i].init(275, 28 + 116 * i, 40, 40, "", NULL, 2, player[i].tintcolor);
			i++;
		}
		
		mapchange.init(348, 529, 170, 40, "Change map");
		mapbox.init(348, 410, 170, 160);
		mapbox.setsel(0);
		prevmap = 0;
		i = 0;
		while (i < signed(maplist.size()))
		{
			mapbox.additem(maplist[i].c_str());
			
			if (maplist[i] == global.lastmap)
			{
				mapbox.setsel(i);
				prevmap = i;
			}
			i++;
		}

		mapok.init(348, 580, 80, 16, "OK");
		mapcancel.init(438, 580, 80, 16, "Cancel");
		changemap = false;
		
		back.init(10, 575, 100, 20, "Back");
		start.init(695, 570, 100, 25, "Start!", 1, BLACK);
		
		startingcash.init(348, 410, 170, 20, "Starting Cash: 0");
		fundsperbuilding.init(348, 431, 170, 20, "Funds per Building: 4000");
		showdamage.init(348, 452, 170, 20, "Show Attacking Damage: Yes");
		techlevel.init(348, 473, 170, 20, "Tech Level: 3");
		var.techlevel = 3;
		cashturns.init(348, 494, 170, 20, "Derricks Last: 10 Turns");
		var.cashturns = 10;
		
		net.clientthread[0] = -1;
		net.clientthread[1] = -1;
		net.clientthread[2] = -1;
		player[0].name = global.netname;
		player[1].name = "";
		player[2].name = "";
		player[3].name = "";
		pmaploaded[0] = false;
		pmaploaded[1] = false;
		pmaploaded[2] = false;
		pready[0] = false;
		pready[1] = false;
		pready[2] = false;
		baddata[0] = 0;
		baddata[1] = 0;
		baddata[2] = 0;
		chatbox.init(348, 39, 420, 321);
		chattext.init(348, 362, 279, 13, true);
		
		boot[0].init(275, 208, 65, 20, "Kick!");
		boot[1].init(275, 208 + 116, 65, 20, "Kick!");
		boot[2].init(275, 208 + 232, 65, 20, "Kick!");
		
		isai[0] = false;
		isai[1] = false;
		isai[2] = false;
		ai[0].init(275, 208, 65, 20, "Toggle AI");
		ai[1].init(275, 208 + 116, 65, 20, "Toggle AI");
		ai[2].init(275, 208 + 232, 65, 20, "Toggle AI");
		
		first = false;
	}
	
	i = 0;
	while (i < 4)
	{
		if (plyrcycle[i].clicked()) pcycleclicked[i] = true;
		else pcycleclicked[i] = false;
		
		if (plyrcycle[i].rclicked()) pcyclerclicked[i] = true;
		else pcyclerclicked[i] = false;
		
		if (teamcycle[i].clicked()) tcycleclicked[i] = true;
		else tcycleclicked[i] = false;
		
		if (teamcycle[i].rclicked()) tcyclerclicked[i] = true;
		else tcyclerclicked[i] = false;
		
		if (colorcycle[i].clicked()) ccycleclicked[i] = true;
		else ccycleclicked[i] = false;
		
		if (colorcycle[i].rclicked()) ccyclerclicked[i] = true;
		else ccyclerclicked[i] = false;
		
		i++;
	}
	
	glb.draw(36, 0, 0);
	back.check();
	chatbox.check();
	str = chattext.check();
	buffer_textout_ex(normal, global.netname, 145, 34, BLACK);
	
	if (oldsel != mapbox.getsel())
	{
		s = mapbox.selected();
		s.erase(0, 5); //get rid of the (2P) / (3P) / (4P)
		sprintf(c, "maps/%s.map", s.c_str());
		load_map(mapbox.selected(), c, false);
		minimap.create();
		minimap.a = 1.0;
		oldsel = mapbox.getsel();
	}
	
	buffer_textout_ex(normal, worldmap.get_name(), 620, 384, BLACK);
	sprintf(c, "%d Players", numplayers);
	buffer_textout_ex(normal, c, 620, 397, BLACK);
	minimap.draw(false, 619, 412, false, true);
	
	buffer_rect(start.x - 3, start.y - 3, start.x + start.w + 3, start.y + start.h + 3, BLACK);
	buffer_rect(start.x, start.y, start.x + start.w, start.y + start.h, BLACK);
	
	if (changemap)
	{
		mapbox.check();
		mapok.check();
		mapcancel.check();
		if (mapok.clicked())
		{
			prevmap = mapbox.getsel();
			changemap = false;
			pmaploaded[0] = false;
			pmaploaded[1] = false;
			pmaploaded[2] = false;
			pready[0] = false;
			pready[1] = false;
			pready[2] = false;
			broadcast_unreadyall(&pready[0]);
			broadcast_mapdata();
		}
		if (mapcancel.clicked())
		{
			mapbox.setsel(prevmap);
			changemap = false;
		}
	}
	else
	{
		notnumplayers = false;
		i = 1;
		if (clientconnected(0) || isai[0]) i++;
		if (clientconnected(1) || isai[1]) i++;
		if (clientconnected(2) || isai[2]) i++;
		if (i < numplayers)
		{
			buffer_textout_ex(normal, "Not enough players for this map.", 348, 515, RED);
			notnumplayers = true;
		}
		else if (i > numplayers)
		{
			notnumplayers = true;
			buffer_textout_ex(normal, "Too many players for this map.", 348, 515, RED);
		}
		
		allready = true;
		i = 0;
		while (i < 3)
		{
			if (clientconnected(i) && !pready[i]) allready = false;
			i++;
		}
		
		if (!notnumplayers && allready)
		{
			start.check();
		}
		
		mapchange.check();
		startingcash.check();
		fundsperbuilding.check();
		showdamage.check();
		//timelimit.check();
		techlevel.check();
		cashturns.check();
		if (mapchange.clicked())
		{
			changemap = true;
		}
	}
	
	do_netai_options(ai, isai, pready, colors, colorcycle, plyrcycle, teamcycle);
	
	do_hostchat(&chatbox, str);
	
	i = 0;
	while (i < net.engine->getMaxConnections())
	{
		if (net.engine->isConnected(i))
		{
			if ((net.engine->isNewClient(i)) || (global.returned_from_netgame))
			{  //either a new client, or we've just returned from a netgame, so resend all the data
				j = assign_new_client(i, isai);
				if (j == -1)
				{  //no player number to assign this new connection to
					net.engine->disconnectThread(i);
				}
				else  //if this thread wasn't immediately disconnected
				{
					empdebug("New client");
					empdebug(j);
					empdebug("has connected on thread");
					empdebug(i);
					
					baddata[j] = 0;
					player[j + 1].name = "";
					pmaploaded[i] = false;
					net.data[j].clear();
					net.address[j + 1] = net.engine->getRemoteAddr(i);
					broadcast_unreadyall(&pready[0]);
					
					player[j + 1].playing = 1;
					player[j + 1].everplaying = 1;
					player[j + 1].number = 0;
					player[j + 1].controller = NET;
					player[j + 1].team = j + 2;
					colors[j + 1] = 0;
					assign_tintcolor(j + 1, colors, isai);
					
					currentcharacter[j + 1] = character[player[j + 1].number];
					
					data.clear();
					data.addByte(NET_ASK_PLAYERDATA);
					send_datablob(i, &data);
					
					send_inidata(i);  //send the hosts's ini file settings
					send_menudata(i); //and the current game settings
					send_mapdata(i);  //and the map
				}
			}
		}
		else  //if the thread isn't connected, check if a client was using that thread
		{
			j = 0;
			while (j < 3)
			{
				if (net.clientthread[j] == i)
				{
					empdebug("Client");
					empdebug(j);
					empdebug("on thread");
					empdebug(net.clientthread[j]);
					empdebug("is no longer connected.");
					
					clear_clientdata(j);
					net.clientthread[j] = -1;
					sprintf(c, "%s has left the game (Player %d disconnected)", player[j + 1].name.c_str(), j + 2);
					broadcast_chat_message(c);
					chatbox.additem(c);
					chatbox.scroll_to_end();
					
					data.clear();
					data.addByte(NET_PLAYER_UPDATE);
					broadcast_datablob(&data);
					
					broadcast_unreadyall(&pready[0]);
					remove_player(j, pready, colors, isai, colorcycle, plyrcycle, teamcycle);
				}  //end of "if the client on this thread disconnected"
				j++;
			}
		}
		i++;
	}
	if (global.returned_from_netgame)
	{
		net.engine->enableTCPListen(41136);
		net.chatengine->enableTCPListen(41137);
		global.returned_from_netgame = false;
	}
	
	j = 0;
	while (j < 3)
	{
		if (clientconnected(j) || isai[j])
		{  //if this client is connected
			if (player[j + 1].name != "")
			{
				buffer_rectfill(36, 36 + (116 * (j + 1)), 127, 127 + (116 * (j + 1)), WHITE);
				if (currentcharacter[j + 1] != NULL)
				{
					buffer_draw_sprite(currentcharacter[j + 1]->picture, 44, 157 + (116 * j));
					buffer_rectfill(275, 28 + (116 * (j + 1)), 315, 68 + (116 * (j + 1)), player[j + 1].tintcolor);
				}
				else buffer_draw_sprite(glb.bmp(51), 44, 157 + 116 * j);
				
				buffer_draw_sprite(glb.bmp(13 + player[j + 1].team), 150, 185 + (116 * j));
				
				buffer_textout_ex(normal, player[j + 1].name, 145, 149 + (116 * j), BLACK);
				if (pready[j])
				{
					buffer_draw_sprite(glb.bmp(28), 275, 188 + (116 * j));
				}
			}
			else if (!isai[j])
			{
				buffer_rectrounded(29, 27 + (116 * (j + 1)), 270, 133 + (116 * (j + 1)), BLACK, 0.7, 3.0);
				buffer_textout_ex(big, "Getting Player Data...", 51, 65 + (116 * (j + 1)), WHITE);
			}
			
			if (!isai[j]) boot[j].check();
			if (!isai[j] && boot[j].clicked())
			{
				net.engine->disconnectThread(net.clientthread[j]);
				net.data[net.clientthread[j]].clear();
				net.clientthread[j] = -1;
				sprintf(c, "%s has been shown the door (Player %d kicked)", player[j + 1].name.c_str(), j + 2);
				broadcast_chat_message(c);
				chatbox.additem(c);
				chatbox.scroll_to_end();
			}
			else
			{
				data = get_client_datablob(j);
				if (data.size() > 0)
				{
					i = data.getByte(0);
					data.removeFront(1);
					
					empdebug("Host received data");
					empdebug(i);
					
					switch(i)
					{
						case NET_PLAYERDATA:
							data.decompress();
							if (!load_client_playerdata(j + 1, &data))
							{  //if this client gave us bad data
								baddata[j]++;
								if (baddata[j] > 2)
								{  //three strikes and you're kicked
									net.engine->disconnectThread(net.clientthread[j]);
								}
							}
							
							data.clear();
							data.addByte(NET_PLAYER_UPDATE);
							broadcast_datablob(&data);
							
							k = 0;
							while (k < 3)
							{
								if (clientconnected(k))
								{
									send_playerdata(net.clientthread[k], k + 1, isai);
								}
								k++;
							}
							break;
						case NET_CHARACTER:
							data.removeFront(1);  //remove playernum
							
							i = j + 1;
							player[i].number++;
							if (unsigned(player[i].number) >= character.size()) player[i].number = -1;
							if (player[i].number != -1) currentcharacter[i] = character[player[i].number];
							else currentcharacter[i] = NULL;
							
							broadcast_unreadyall(&pready[0]);
							data.clear();
							data.addByte(NET_CHARACTER);
							data.addByte(i);
							data.addByte(player[i].number);
							broadcast_datablob(&data);
							
							if (currentcharacter[i] == NULL)
							{
								colors[i] = -1;
								player[i].tintcolor = WHITE;
							}
							else
							{
								colors[i] = 0;
								assign_tintcolor(i, colors, isai);
							}
							data.clear();
							data.addByte(NET_PLAYERTINT);
							data.addByte(i);
							data.appendVariable(player[i].tintcolor);
							broadcast_datablob(&data);
							
							minimap.create();  //update the minimap to reflect the new player color
							minimap.a = 1.0;
							
							break;
						case NET_CLIENT_TINTCYCLE:
							i = j + 1;
							
							if (currentcharacter[i] != NULL)
							{  //ignore if this client has a random character
								colors[i]++;
								assign_tintcolor(i, colors, isai);
								
								data.clear();
								data.addByte(NET_PLAYERTINT);
								data.addByte(i);
								data.appendVariable(player[i].tintcolor);
								broadcast_datablob(&data);
								
								minimap.create();  //update the minimap to reflect the new player color
								minimap.a = 1.0;
							}
							break;
						case NET_PLAYERTEAM:
							data.removeFront(1);  //remove playernum
							
							player[j + 1].team++;
							if (player[j + 1].team > 4) player[j + 1].team = 1;
							
							broadcast_unreadyall(&pready[0]);
							data.clear();
							data.addByte(NET_PLAYERTEAM);
							data.addByte(j + 1);
							data.addByte(player[j + 1].team);
							broadcast_datablob(&data);
							break;
						case NET_MAPLOADED:
							pmaploaded[j] = true;
							break;
						case NET_CLIENTREADY:
							pready[j] = data.getByte(0);
							data.clear();
							data.addByte(NET_CLIENTREADY);
							data.addByte(j + 1);
							data.addByte(pready[j]);
							broadcast_datablob(&data);
							break;
					}
				}
			}
		}
		j++;
	}
	
	i = 0;
	while (i < 4)
	{
		if ((i == 0) || isai[i - 1])
		{
			buffer_rectfill(36, 36 + 116 * i, 127, 127 + 116 * i, WHITE);
			plyrcycle[i].check();
			buffer_rect(38, 36 + 116 * i, 128, 126 + 116 * i, BLACK);
			buffer_textout_ex(med, "Team", 156, 55 + 116 * i, WHITE);
			buffer_textout_ex(med, "Team", 158, 55 + 116 * i, WHITE);
			buffer_textout_ex(med, "Team", 156, 57 + 116 * i, WHITE);
			buffer_textout_ex(med, "Team", 158, 57 + 116 * i, WHITE);
			buffer_textout_ex(med, "Team", 157, 56 + 116 * i, BLACK);
			teamcycle[i].check();
			if (currentcharacter[i] != NULL) colorcycle[i].check();
		}
		
		if (((i == 0) || isai[i - 1]) && (pcycleclicked[i] || pcyclerclicked[i]))
		{
			if (pcycleclicked[i]) player[i].number++;
			if (pcyclerclicked[i]) player[i].number--;
			
			if (player[i].number >= signed(character.size())) player[i].number = -1;
			if (player[i].number < -1) player[i].number = character.size() - 1;
			
			if (player[i].number != -1)
			{
				currentcharacter[i] = character[player[i].number];
				plyrcycle[i].init(44, 41 + 116 * i, 80, 80, "", currentcharacter[i]->picture);
			}
			else
			{
				currentcharacter[i] = NULL;
				plyrcycle[i].init(44, 41 + 116 * i, 80, 80, "", glb.bmp(51));
			}
			
			broadcast_unreadyall(&pready[i]);
			
			data.clear();
			data.addByte(NET_CHARACTER);
			data.addByte(i);
			data.addByte(player[i].number);
			broadcast_datablob(&data);
			
			if (currentcharacter[i] != NULL)
			{
				colors[i] = 0;
				assign_tintcolor(i, colors, isai);
			}
			else player[i].tintcolor = WHITE;
			colorcycle[i].init(275, 28 + 116 * i, 40, 40, "", NULL, 2, player[i].tintcolor);
			
			minimap.create();  //update the minimap to reflect the new player color
			minimap.a = 1.0;
			
			data.clear();
			data.addByte(NET_PLAYERTINT);
			data.addByte(i);
			data.appendVariable(player[i].tintcolor);
			broadcast_datablob(&data);
		}
		
		if (((i == 0) || isai[i - 1]) && (tcycleclicked[i] || tcyclerclicked[i]))
		{
			if (tcycleclicked[i]) player[i].team++;
			if (tcyclerclicked[i]) player[i].team--;
			
			if (player[i].team > 4) player[i].team = 1;
			if (player[i].team < 1) player[i].team = 4;
			
			teamcycle[i].init(150, 70 + 116 * i, 40, 40, "", glb.bmp(13 + player[i].team));
			broadcast_unreadyall(&pready[i]);
			
			data.clear();
			data.addByte(NET_PLAYERTEAM);
			data.addByte(i);
			data.addByte(player[i].team);
			broadcast_datablob(&data);
		}
		
		if (((i == 0) || isai[i - 1]) && ((currentcharacter[i] != NULL) && (ccycleclicked[i] || ccyclerclicked[i])))
		{
			if (ccycleclicked[i]) colors[i]++;
			if (ccyclerclicked[i]) colors[i]--;
			
			if (colors[i] < 0) colors[i] = 3;
			
			assign_tintcolor(i, colors, isai);
			colorcycle[i].init(275, 28 + 116 * i, 40, 40, "", NULL, 2, player[i].tintcolor);
			
			minimap.create();  //update the minimap to reflect the new player color
			minimap.a = 1.0;
			
			data.clear();
			data.addByte(NET_PLAYERTINT);
			data.addByte(i);
			data.appendVariable(player[i].tintcolor);
			broadcast_datablob(&data);
		}
		i++;
	}
	
	if (!changemap)
	{
		if (startingcash.clicked())
		{
			player[0].cash += 5000;
			if (player[0].cash > 50000) player[0].cash = 0;
			i = 1;
			while (i < 4)
			{
				player[i].cash = player[0].cash;
				i++;
			}
			sprintf(c, "Starting Cash: %d", player[0].cash);
			startingcash.init(348, 410, 170, 20, c);
			broadcast_unreadyall(&pready[0]);
			
			data.clear();
			data.addByte(NET_STARTINGCASH);
			data.addByte(player[0].cash / 1000);
			broadcast_datablob(&data);
		}
		if (startingcash.rclicked())
		{
			player[0].cash -= 5000;
			if (player[0].cash < 0) player[0].cash = 50000;
			i = 1;
			while (i < 4)
			{
				player[i].cash = player[0].cash;
				i++;
			}
			sprintf(c, "Starting Cash: %d", player[0].cash);
			startingcash.init(348, 410, 170, 20, c);
			broadcast_unreadyall(&pready[0]);
			
			data.clear();
			data.addByte(NET_STARTINGCASH);
			data.addByte(player[0].cash / 1000);
			broadcast_datablob(&data);
		}
		
		if (fundsperbuilding.clicked())
		{
			gameoptions.funding += 1000;
			if (gameoptions.funding > 10000)
			{
				gameoptions.funding = 1000;
			}
			sprintf(c, "Funds per Building: %d", gameoptions.funding);
			fundsperbuilding.init(348, 431, 170, 20, c);
			broadcast_unreadyall(&pready[0]);
			
			data.clear();
			data.addByte(NET_FUNDSPERBUILDING);
			data.addByte(gameoptions.funding / 1000);
			broadcast_datablob(&data);
		}
		if (fundsperbuilding.rclicked())
		{
			gameoptions.funding -= 1000;
			if (gameoptions.funding < 1000) gameoptions.funding = 10000;
			
			sprintf(c, "Funds per Building: %d", gameoptions.funding);
			fundsperbuilding.init(348, 431, 170, 20, c);
			broadcast_unreadyall(&pready[0]);
			
			data.clear();
			data.addByte(NET_FUNDSPERBUILDING);
			data.addByte(gameoptions.funding / 1000);
			broadcast_datablob(&data);
		}
		
		if (showdamage.clicked() || showdamage.rclicked())
		{
			if (gameoptions.show_damage)
			{
				gameoptions.show_damage = false;
				showdamage.init(348, 452, 170, 20, "Show Attacking Damage: No");
			}
			else
			{
				gameoptions.show_damage = true;
				showdamage.init(348, 452, 170, 20, "Show Attacking Damage: Yes");
			}
			broadcast_unreadyall(&pready[0]);
			
			data.clear();
			data.addByte(NET_SHOWDAMAGE);
			data.addByte(gameoptions.show_damage);
			broadcast_datablob(&data);
		}
		
		if (techlevel.clicked())
		{
			var.techlevel++;
			if (var.techlevel > 3) var.techlevel = 1;
			sprintf(c, "Tech Level: %d", var.techlevel);
			techlevel.init(348, 473, 170, 20, c);
			broadcast_unreadyall(&pready[0]);
			
			data.clear();
			data.addByte(NET_TECHLEVEL);
			data.addByte(var.techlevel);
			broadcast_datablob(&data);
		}
		if (techlevel.rclicked())
		{
			var.techlevel--;
			if (var.techlevel < 1) var.techlevel = 3;
			
			sprintf(c, "Tech Level: %d", var.techlevel);
			techlevel.init(348, 473, 170, 20, c);
			broadcast_unreadyall(&pready[0]);
			
			data.clear();
			data.addByte(NET_TECHLEVEL);
			data.addByte(var.techlevel);
			broadcast_datablob(&data);
		}
		
		if (cashturns.clicked())
		{  //cycle the number of turns oil derricks last
			var.cashturns += 5;
			if (var.cashturns > 30) var.cashturns = 0;
			
			if (var.cashturns == 0)
			{
				cashturns.init(348, 494, 170, 20, "Derricks Last: Forever");
			}
			else
			{
				sprintf(c, "Derricks Last: %d Turns", var.cashturns);
				cashturns.init(348, 494, 170, 20, c);
			}
			broadcast_unreadyall(&pready[0]);
			
			data.clear();
			data.addByte(NET_CASHTURNS);
			data.addByte(var.cashturns);
			broadcast_datablob(&data);
		}
		if (cashturns.rclicked())
		{  //cycle the number of turns oil derricks last
			var.cashturns -= 5;
			if (var.cashturns < 0) var.cashturns = 30;
			
			if (var.cashturns == 0)
			{
				cashturns.init(348, 494, 170, 20, "Derricks Last: Forever");
			}
			else
			{
				sprintf(c, "Derricks Last: %d Turns", var.cashturns);
				cashturns.init(348, 494, 170, 20, c);
			}
			broadcast_unreadyall(&pready[0]);
			
			data.clear();
			data.addByte(NET_CASHTURNS);
			data.addByte(var.cashturns);
			broadcast_datablob(&data);
		}
		
		if (!notnumplayers && allready && start.clicked())
		{  //if the start button was clicked and everything's ready to go
			global.lastmap = mapbox.selected();
			
			net.engine->disableTCPListen();
			net.chatengine->disableTCPListen();
			if (udpOK) udp.closeSocket();
			
			player[0].controller = HUMAN;
			player[0].name = global.netname;
			
			if (currentcharacter[0] == NULL)
			{
				player[0].number = rand()%3;
				currentcharacter[0] = character[player[0].number];
				
				colors[0] = 0;
				assign_tintcolor(0, colors, isai);
				
				data.clear();
				data.addByte(NET_CHARACTER);
				data.addByte(0);
				data.addByte(player[0].number);
				broadcast_datablob(&data);
				data.clear();
				data.addByte(NET_PLAYERTINT);
				data.addByte(0);
				data.appendVariable(player[0].tintcolor);
				broadcast_datablob(&data);
			}
			
			i = 0;
			while (i < 3)
			{
				if (clientconnected(i))
				{
					player[i + 1].controller = NET;
					
					if (currentcharacter[i + 1] == NULL)
					{
						player[i + 1].number = rand()%character.size();
						currentcharacter[i + 1] = character[player[i + 1].number];
						colors[i + 1]++;
						
						assign_tintcolor(i + 1, colors, isai);
						
						data.clear();
						data.addByte(NET_CHARACTER);
						data.addByte(i + 1);
						data.addByte(player[i + 1].number);
						broadcast_datablob(&data);
						data.clear();
						data.addByte(NET_PLAYERTINT);
						data.addByte(i + 1);
						data.appendVariable(player[i + 1].tintcolor);
						broadcast_datablob(&data);
					}
				}
				else if (isai[i])
				{
					player[i + 1].controller = AI;
				}
				
				i++;
			}
			create_sidebars();
			data.clear();
			data.addByte(NET_GO);
			broadcast_datablob(&data);
			global.very_first_turn = 1;
			pturn = 0;
			first = true;
			return NET_PLAY;
		}
	}
	
	if (back.clicked())
	{
		if (udpOK) udp.closeSocket();
		net.engine->disableTCPListen();
		i = 0;
		while (i < net.engine->getMaxConnections())
		{
			if (net.engine->isConnected(i))
			{
				net.engine->disconnectThread(i);
			}
			if (net.chatengine->isConnected(i))
			{
				net.chatengine->disconnectThread(i);
			}
			i++;
		}
		first = true;
		return NET_FIRST;
	}
	
	if (!unlisted_game)
	{
		udpOK = udphandler.update();
		if (udpOK)
		{
			last_lobby_update++;
			if (last_lobby_update > 250)
			{  //update lobby info every 5 seconds
				i = 0;
				if (clientconnected(0)) i++;
				if (clientconnected(1)) i++;
				if (clientconnected(2)) i++;
				if (i < 3)
				{  //don't send lobby data if the room's full
					send_lobbydata(&udp, i + 1);
				}
				last_lobby_update = 0;
			}
		}
		else
		{
			//udpOK = udp.openSocket(41138);
//			empdebug(udp.getErrMsg());
			buffer_textout_ex(normal, "Error connecting to netplay lobby - clients won't see your game in their menu", 5, 5, RED);
		}
	}
	
	draw_mouse();
	
	return NET_HOST;
}

bool do_hostchat(_listbox *chatbox, string str)
{  //returns true if a new item has been added
	static DyBytes incoming[MAX_CONNECTIONS];
	bool result = false;
	int i, j, k;
	string chatstr;
	
	if ((str != "/NOTHING YET/") && (str != ""))
	{
		chatstr = global.netname;
		chatstr += ": ";
		chatstr += str;
		chatbox->additem(chatstr.c_str());
		chatbox->scroll_to_end();
		
		net.chatdata.addByte(chatstr.length() % 256);
		if (chatstr.length() > 255) net.chatdata.addByte(chatstr.length() - 255);
		else net.chatdata.addByte(0);  //as in networkclient, ugly way of specifying string length (from 0 to 2 * 255)
		
		j = 0;
		while (unsigned(j) < chatstr.length())
		{
			net.chatdata.addByte(chatstr[j]);
			j++;
		}
		
		result = true;
		i = 0;
		while (i < net.chatengine->getMaxConnections())
		{
			if (net.chatengine->isConnected(i))
			{
				net.chatengine->sendData(i, net.chatdata);
			}
			i++;
		}
		net.chatdata.clear();
	}
	
	i = 0;
	while (i < net.chatengine->getMaxConnections())
	{
		if (net.chatengine->isConnected(i))
		{
			net.chatengine->getData(i, net.chatdata);
			if (net.chatdata.size() > 0)
			{
				incoming[i].append(net.chatdata);
				
				if (incoming[i].size() > 2)
				{
					j = incoming[i].getByte(0) + incoming[i].getByte(1);
					
					while (incoming[i].size() >= unsigned(j))
					{
						incoming[i].removeFront(2);
						
						chatstr = "";
						k = 0;
						while (k < j)
						{
							chatstr += incoming[i].getByte(k);
							k++;
						}
						incoming[i].removeFront(j);
						
						net.chatdata.clear();
						net.chatdata.addByte(j % 256);
						if (j > 255) net.chatdata.addByte(j - 255);
						else net.chatdata.addByte(0);
						k = 0;
						while (k < j)
						{
							net.chatdata.addByte(chatstr[k]);
							k++;
						}
						
						j = 0;
						while (j < net.chatengine->getMaxConnections())
						{
							if (net.chatengine->isConnected(j))
							{
								net.chatengine->sendData(j, net.chatdata);
							}
							j++;
						}
						chatbox->additem(chatstr.c_str());
						chatbox->scroll_to_end();
						result = true;
						
						if (incoming[i].size() > 2) j = incoming[i].getByte(0) + incoming[i].getByte(1);
					}
				}
				net.chatdata.clear();
			}
		}
		i++;
	}
	return result;
}

void broadcast_chat_message(string mesg)
{
	int i = 0;
	DyBytes data;
	data.addString(mesg);
	
	while (i < net.chatengine->getMaxConnections())
	{
		if (net.chatengine->isConnected(i))
		{
			net.chatengine->sendData(i, data);
		}
		i++;
	}
}

void broadcast_unreadyall(bool *readystatus)
{
	int i = 0;
	DyBytes data;
	
	while (i < 3)
	{
		readystatus[i] = false;
		i++;
	}
	
	data.clear();
	data.addByte(NET_UNREADYALL);
	broadcast_datablob(&data);
}

void send_inidata(int thread)
{
	int i = 0, j, k;
	DyBytes data, alldata;
	alldata.clear();
	data.clear();
	
	while (i < 3)
	{
		data.appendVariable(armystats[i].pricefraction);
		data.appendVariable(armystats[i].movemod);
		data.appendVariable(armystats[i].rangemaxmod);
		j = 0;
		while (j < 3)
		{
			data.appendVariable(armystats[i].attackfraction[j]);
			data.appendVariable(armystats[i].resistfraction[j]);
			j++;
		}
		i++;
	}
	
	data.addByte(NUM_UNITS);
	i = 0;
	while (i < NUM_UNITS)
	{
		j = unitstats[i].name.length();
		data.appendVariable(j);
		k = 0;
		while (k < j)
		{
			data.addByte(unitstats[i].name[k]);
			k++;
		}
		j = unitstats[i].description.length();
		data.appendVariable(j);
		k = 0;
		while (k < j)
		{
			data.addByte(unitstats[i].description[k]);
			k++;
		}
		
		data.appendVariable(unitstats[i]._move);
		data.appendVariable(unitstats[i].movetype);
		data.appendVariable(unitstats[i].attacktype);
		data.appendVariable(unitstats[i].basetype);
		data.appendVariable(unitstats[i]._rangemin);
		data.appendVariable(unitstats[i]._rangemax);
		data.appendVariable(unitstats[i]._price);
		
		data.appendVariable(unitstats[i].loadcapacity);
		data.appendVariable(unitstats[i].loadtype);
		data.appendVariable(unitstats[i].canhit);
		data.appendVariable(unitstats[i].attackparticle);
		data.appendVariable(unitstats[i].attacksound);
		data.appendVariable(unitstats[i].movesound);
		data.appendVariable(unitstats[i].moveparticle);
		data.appendVariable(unitstats[i].cancapture);
		data.appendVariable(unitstats[i].isarmy[0]);
		data.appendVariable(unitstats[i].isarmy[1]);
		data.appendVariable(unitstats[i].isarmy[2]);
		data.appendVariable(unitstats[i].techlevel);
		data.appendVariable(unitstats[i].attacksburn);
		//don't transfer unitstats[i].healthdrawhack, that's something that depends on the client's pngs
		data.appendVariable(unitstats[i].blastradius);
		data.appendVariable(unitstats[i].attackdelay);
		
		j = 0;
		while (j < 3)
		{
			data.appendVariable(unitstats[i]._attack[j]);
			data.appendVariable(unitstats[i]._resist[j]);
			j++;
		}
		i++;
	}
	data.compress(9);
	
	alldata.addByte(NET_INIDATA);
	alldata.append(&data);
	send_datablob(thread, &alldata);
}

void send_lobbydata(UDPSock *socket, unsigned char players)
{
	unsigned char data[256];
	unsigned char i, j = 3;
	//int r, s;
	
	//data format is type, gameid, game-specific data length, then game-specific data after that
	
	data[0] = 0;  //indicates that it's sending host data
	data[1] = gameid;
	
	data[j] = REVISION_MAJOR;
	j++;
	data[j] = REVISION_MINOR;
	j++;
	data[j] = players;
	j++;
	data[j] = strlen(global.netname.c_str());
	j++;
	data[j] = strlen(worldmap.get_name().c_str());
	j++;
	i = 0;
	while (i < global.netname.length())
	{
		data[j] = global.netname[i];
		i++;
		j++;
	}
	i = 0;
	while (i < worldmap.get_name().length())
	{
		data[j] = worldmap.get_name()[i];
		i++;
		j++;
	}
	data[2] = j - 3;
	
	udphandler.send(data, (int)j);
}

void send_playerdata(int thread, int placement, bool *isai)
{
	unsigned char c;
	int i;
	unsigned int j, k;
	string s;
	DyBytes data;
	
	data.clear();
	data.addByte(placement);
	
	i = 0;
	while (i < 4)
	{
		if ((i == 0) || ((clientconnected(i - 1)) && (player[i].name != "")) || isai[i - 1])
		{
			j = player[i].name.length();
			data.addByte(j);
			k = 0;
			while (k < j)
			{
				data.addByte(player[i].name[k]);
				k++;
			}
			data.addByte(player[i].number);
			if (player[i].controller == HUMAN) data.addByte(NET);
			else if (player[i].controller == AI) data.addByte(NETAI);
			else data.addByte(player[i].controller);
			data.addByte(player[i].team);
			data.appendVariable(player[i].tintcolor);
			
			if ((i != 0) && !isai[i - 1])
			{
				s = net.address[i];
				j = s.length();
				data.addByte(j);
				k = 0;
				while (k < j)
				{
					c = s[k];
					data.addByte(c);
					k++;
				}
			}
			else data.addByte(0);
		}
		else
		{
			data.addByte(0);
		}
		i++;
	}
	data.compress(9);
	data.insertByte(NET_PLAYERDATA);
	
	send_datablob(thread, &data);
}

DyBytes menudata()
{
	DyBytes data;
	data.clear();
	
	data.addByte(var.cashturns);
	data.addByte(player[0].cash / 1000);
	data.addByte(var.techlevel);
	data.addByte(gameoptions.funding / 1000);
	data.addByte(gameoptions.show_damage);
	data.compress(9);
	data.insertByte(NET_MENUDATA);
	return data;
}

void send_menudata(int thread)
{
	DyBytes data = menudata();
	send_datablob(thread, &data);
}

void broadcast_menudata()
{
	DyBytes data = menudata();
	broadcast_datablob(&data);	
}

bool load_client_playerdata(int p, DyBytes *data)
{
	unsigned int namelen, i;
	char str[50];
	
	namelen = data->getByte(0);
	data->removeFront(1);
	if (namelen > data->size()) return false;
	if (namelen >= 50) return false;
	
	i = 0;
	while (i < namelen)
	{
		str[i] = data->getByte(0);
		data->removeFront(1);
		i++;
	}
	str[i] = '\0';
	player[p].name = str;
	
	return true;
}

void do_netai_options(_button *b, bool *isai, bool *pready, int *colors, _button *ccyc, _button *pcyc, _button *tcyc)
{
	int i = 0, j;
	while (i < 3)
	{
		if (!clientconnected(i))
		{
			if ((i == 0) || isai[i - 1] || clientconnected(i - 1))
			{
				b[i].check();
				if (b[i].clicked())
				{
					isai[i] = !isai[i];
					
					if (isai[i])
					{
						player[i + 1].number = 0;
						player[i + 1].team = i + 2;
						player[i + 1].name = "AI";
						player[i + 1].controller = AI;
						player[i + 1].playing = 1;
						player[i + 1].everplaying = 1;
						colors[i + 1] = 0;
						assign_tintcolor(i + 1, colors, isai);
						currentcharacter[i + 1] = character[player[i + 1].number];
						
						pcyc[i + 1].init(pcyc[i + 1].x, pcyc[i + 1].y, 80, 80, "", currentcharacter[i + 1]->picture);
						tcyc[i + 1].init(tcyc[i + 1].x, tcyc[i + 1].y, 40, 40, "", glb.bmp(13 + player[i + 1].team));
						ccyc[i + 1].init(ccyc[i + 1].x, ccyc[i + 1].y, 40, 40, "", NULL, 2, player[i + 1].tintcolor);
						
						j = 0;
						while (j < 3)
						{
							if (clientconnected(j))
							{
								send_playerdata(net.clientthread[j], j + 1, isai);
							}
							j++;
						}
					}
					else
					{
						remove_player(i, pready, colors, isai, ccyc, pcyc, tcyc);
					}
				}
			}
		}
		
		if (isai[i]) glb.draw(55, 200, 186 + 116 * i);
		
		i++;
	}
}

void remove_player(int j, bool *pready, int *colors, bool *isai, _button *ccyc, _button *pcyc, _button *tcyc)
{
	int k;
	
	net.clientthread[j] = -1;
	player[j + 1].name = "";
	player[j + 1].playing = 0;
	player[j + 1].everplaying = 0;
	pready[j] = false;
	player[j + 1].team = j + 1;
	player[j + 1].number = 0;
	player[j + 1].tintcolor = player_color(0, 0);
	currentcharacter[j + 1] = NULL;
	
	k = j;
	while (k < 2)
	{  //if the guy who was removed was in the middle, move everyone below him up one to fill the gap
		if ((clientconnected(k + 1)) || (isai[k + 1]))
		{
			if (clientconnected(k + 1)) net.clientthread[k] = net.clientthread[k + 1];
			else isai[k] = isai[k + 1];
			pready[k] = pready[k + 1];
			player[k + 1].name = player[k + 2].name;
			player[k + 1].team = player[k + 2].team;
			player[k + 1].number = player[k + 2].number;
			player[k + 1].controller = player[k + 2].controller;
			player[k + 1].tintcolor = player[k + 2].tintcolor;
			currentcharacter[k + 1] = currentcharacter[k + 2];
			colors[k + 1] = colors[k + 2];
			
			net.clientthread[k + 1] = -1;
			isai[k + 1] = false;
			pready[k + 1] = false;
			player[k + 2].name = "";
			player[k + 2].team = 1;
			player[k + 2].number = 0;
			player[k + 2].controller = HUMAN;
			player[k + 2].tintcolor = player_color(0, 0);
			colors[k + 2] = 0;
			currentcharacter[k + 2] = NULL;
			
			if (isai[k])
			{
				pcyc[k + 1].init(pcyc[k + 1].x, pcyc[k + 1].y, 80, 80, "", currentcharacter[k + 1]->picture);
				tcyc[k + 1].init(tcyc[k + 1].x, tcyc[k + 1].y, 40, 40, "", glb.bmp(13 + player[k + 1].team));
				ccyc[k + 1].init(ccyc[k + 1].x, ccyc[k + 1].y, 40, 40, "", NULL, 2, player[k + 1].tintcolor);
			}
		}
		k++;
	}
	k = 0;
	while (k < 3)
	{
		if (clientconnected(k))
		{
			send_playerdata(net.clientthread[k], k + 1, isai);
		}
		k++;
	}
}

void assign_tintcolor(int p, int *colors, bool *isai)
{
	int k;
	
	if (colors[p] > 3) colors[p] = 0;
	player[p].tintcolor = player_color(player[p].number, colors[p]);
	
	k = 0;
	while (k < 4)
	{
		if ( (p != k) && ((k == 0) || clientconnected(k - 1) || isai[k - 1]) )
		{
			if (player[k].tintcolor == player[p].tintcolor)
			{
				colors[p]++;
				if (colors[p] > 3) colors[p] = 0;
				k = -1;
				player[p].tintcolor = player_color(player[p].number, colors[p]);
			}
		}
		k++;
	}
}

int assign_new_client(int i, bool *isai)
{
	int j = 0;
	bool assigned = false;
	
	while ((j < 3) && (!assigned))
	{
		if (!isai[j] && !clientconnected(j))
		{
			net.clientthread[j] = i;
			assigned = true;
		}
		else j++;
	}
	if (assigned) return j;
	return -1;
}
