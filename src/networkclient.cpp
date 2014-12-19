#include "empire.h"

extern void draw_net_icon(int x, int y);
void load_inidata(DyBytes *data);
void load_playerdata(DyBytes *data);
void load_menudata(DyBytes *data);
void send_client_playerdata();
void list_gamestats(int x, int y);

bool error_different_numunits;

int net_joingame()
{
	static bool first = true, maploaded, playersloaded, playerwait, teamwait, tintwait, pready[4];
	static _button back, plyrcycle, teamcycle, colorcycle, ready;
	static _listbox chatbox;
	static _textline chattext;
	int i, n;
	char c[50];
	string str, datastr;
	DyBytes data;
	
	if (first)
	{
		play_song(battlemenu_music);
		
		error_different_numunits = false;
		back.init(10, 575, 100, 20, "Back");
		ready.init(675, 565, 100, 20, "Not Ready", 1, RED);
		chatbox.init(348, 39, 420, 321);
		chattext.init(348, 362, 279, 13, true);
		maploaded = false;
		playersloaded = false;
		playerwait = false;
		teamwait = false;
		tintwait = false;
		i = 0;
		while (i < 4)
		{
			player[i].name = "";
			pready[i] = false;
			player[i].team = i + 1;
			player[i].number = 0;
			player[i].tintcolor = player_color(0, i);
			currentcharacter[i] = character[0];
			i++;
		}
		
		net.address[0] = net.engine->getRemoteAddr(net.thread);
		net.hostnum = 0;
		
		gameoptions.time = -999;
		
		plyrcycle.init(-10, -10, 1, 1, "");
		teamcycle.init(-10, -10, 1, 1, "");
		colorcycle.init(-10, -10, 1, 1, "");
		net.client_pnum = -1;
		first = false;
		
		empdebug("Entered network client function");
	}
	
	if (global.returned_from_netgame)
	{  //if just back from a game, we need to wait for the host to get back
		if (playersloaded)
		{  //the host has begun re-sending all the data, so that means we're both in the netplay menu again
			global.returned_from_netgame = false;
		}
		draw_net_icon(760, 0);
	}
	
	data = get_host_datablob();
	if (data.size() > 0)
	{
		i = data.getByte(0);
		data.removeFront(1);
		empdebug("Client received data:");
		empdebug(i);
		switch(i)
		{
			case NET_ASK_PLAYERDATA:
				send_client_playerdata();
				break;
			case NET_PLAYER_UPDATE:
				playersloaded = false;
				break;
			case NET_PLAYERDATA:
				load_playerdata(&data);
				
				if (currentcharacter[net.client_pnum] != NULL)
					plyrcycle.init(44, 41 + (116 * net.client_pnum), 80, 80, "", currentcharacter[net.client_pnum]->picture);
				else
					plyrcycle.init(44, 41 + (116 * net.client_pnum), 80, 80, "", glb.bmp(51));
				
				teamcycle.init(150, 70 + (116 * net.client_pnum), 40, 40, "", glb.bmp(13 + player[net.client_pnum].team));
				colorcycle.init(275, 28 + (116 * net.client_pnum), 40, 40, "", NULL, 2, player[net.client_pnum].tintcolor);
				playersloaded = true;
				playerwait = false;
				teamwait = false;
				tintwait = false;
				break;
			case NET_MENUDATA:
				load_menudata(&data);
				break;
			case NET_CHARACTER:
				i = data.getByte(0);
				n = (signed char)data.getByte(1);
				player[i].number = n;
				if (n == -1) currentcharacter[i] = NULL;
				else currentcharacter[i] = character[n];
				if (i == net.client_pnum)
				{
					if (currentcharacter[i] != NULL)
						plyrcycle.init(44, 41 + (116 * i), 80, 80, "", currentcharacter[i]->picture);
					else
						plyrcycle.init(44, 41 + (116 * i), 80, 80, "", glb.bmp(51));
					playerwait = false;
				}
				break;
			case NET_PLAYERTEAM:
				i = data.getByte(0);
				n = data.getByte(1);
				player[i].team = n;
				if (i == net.client_pnum)
				{
					teamcycle.init(150, 70 + (116 * i), 40, 40, "", glb.bmp(13 + player[i].team));
					teamwait = false;
				}
				break;
			case NET_PLAYERTINT:
				i = data.getByte(0);
				data.removeFront(1);
				data.extractVariable(0, player[i].tintcolor);
				if (i == net.client_pnum)
				{
					colorcycle.init(275, 28 + (116 * i), 40, 40, "", NULL, 2, player[i].tintcolor);
					tintwait = false;
				}
				
				if (maploaded)
				{  //if the map is loaded, update it to reflect the new tint color
					minimap.create();
					minimap.a = 1.0;
				}
				break;
			case NET_FUNDSPERBUILDING:
				i = data.getByte(0);
				gameoptions.funding = i * 1000;
				break;
			case NET_SHOWDAMAGE:
				gameoptions.show_damage = (data.getByte(0) != 0);
				break;
			case NET_STARTINGCASH:
				n = data.getByte(0);
				i = 0;
				while (i < 4)
				{
					player[i].cash = n * 1000;
					i++;
				}
				break;
			case NET_TECHLEVEL:
				i = data.getByte(0);
				var.techlevel = i;
				break;
			case NET_CASHTURNS:
				i = data.getByte(0);
				var.cashturns = i;
				break;
			case NET_MAPCHANGE:
				maploaded = false;
				break;
			case NET_MAPDATA:
				load_mapdata(&data);
				minimap.create();
				minimap.a = 1.0;
				maploaded = true;
				data.clear();
				data.addByte(NET_MAPLOADED);
				send_datablob(net.thread, &data);
				break;
			case NET_CLIENTREADY:
				i = data.getByte(0);
				if (data.getByte(1) == 0)
				{
					pready[i] = false;
				}
				else
				{
					pready[i] = true;
				}
				break;
			case NET_UNREADYALL:
				pready[1] = false;
				pready[2] = false;
				pready[3] = false;
				ready.init(675, 565, 100, 20, "Not Ready", 1, RED);
				break;
			case NET_INIDATA:
				load_inidata(&data);
				if (error_different_numunits)
				{
					net_choosemode(6);
					first = true;
					if (net.chatengine->isConnected(net.chatthread))
					{
						net.chatengine->disconnectThread(net.chatthread);
					}
					clear_clientdata(0);
					return NET_FIRST;
				}
				break;
			case NET_GO:
				create_sidebars();
				player[net.client_pnum].controller = HUMAN;
				global.very_first_turn = 1;
				pturn = 0;
				first = true;
				return NET_PLAY;
				break;
		}
	}
	
	if (!net.engine->isConnected(net.thread))
	{
		first = true;
		if (net.chatengine->isConnected(net.chatthread))
		{
			net.chatengine->disconnectThread(net.chatthread);
		}
		clear_clientdata(0);
		net_choosemode(3);  //return an error to the previous screen
		empdebug("Error, disconnecting...");
		return NET_FIRST;
	}
	
	glb.draw(36, 0, 0);
	back.check();
	ready.check();
	chatbox.check();
	str = chattext.check();
	do_clientchat(&chatbox, str);
	
	if ((ready.clicked()) && maploaded && (net.client_pnum != -1))
	{
		if (pready[net.client_pnum])
		{
			pready[net.client_pnum] = false;
			ready.init(675, 565, 100, 20, "Not Ready", 1, RED);
			data.clear();
			data.addByte(NET_CLIENTREADY);
			data.addByte(0);
			send_datablob(net.thread, &data);
		}
		else
		{
			pready[net.client_pnum] = true;
			ready.init(675, 565, 100, 20, "Ready", 1, BLACK);
			data.clear();
			data.addByte(NET_CLIENTREADY);
			data.addByte(1);
			send_datablob(net.thread, &data);
		}
	}
	
	list_gamestats(345, 390);
	
	if (maploaded)
	{
		buffer_textout_ex(normal, worldmap.get_name(), 620, 384, BLACK);
		sprintf(c, "%d Players", numplayers);
		buffer_textout_ex(normal, c, 620, 397, BLACK);
		minimap.draw(false, 619, 412, false, true);
	}
	else
	{
		buffer_textout_ex(normal, "Loading map...", 620, 384, BLACK);
	}
	
	if (playersloaded)
	{
		i = 0;
		while (i < 4)
		{
			if ((player[i].name != "") || (i == net.client_pnum))
			{
				buffer_textout_ex(med, "Team", 156, 55 + 116 * i, WHITE);
				buffer_textout_ex(med, "Team", 158, 55 + 116 * i, WHITE);
				buffer_textout_ex(med, "Team", 156, 57 + 116 * i, WHITE);
				buffer_textout_ex(med, "Team", 158, 57 + 116 * i, WHITE);
				buffer_textout_ex(med, "Team", 157, 56 + 116 * i, BLACK);
				
				if (i != net.client_pnum)
				{
					buffer_draw_sprite(glb.bmp(13 + player[i].team), 150, 70 + (116 * i));
					buffer_rectfill(275, 28 + (116 * i), 315, 68 + (116 * i), player[i].tintcolor);
					buffer_textout_ex(normal, player[i].name, 145, 32 + (116 * i), BLACK);
				}
				else buffer_textout_ex(normal, global.netname, 145, 32 + (116 * i), BLACK);
				
				if (player[i].controller == NETAI) glb.draw(55, 200, 70 + 116 * i);
				
				buffer_rectfill(36, 36 + (116 * i), 127, 127 + (116 * i), WHITE);
				if (currentcharacter[i] != NULL)
				{
					if (i == net.client_pnum)
					{
						buffer_rect(27, 26 + (116 * i), 137, 136 + (116 * i), WHITE);
					}
					else buffer_draw_sprite(currentcharacter[i]->picture, 44, 42 + (116 * i));
				}
				else buffer_draw_sprite(glb.bmp(51), 44, 42 + (116 * i));
				
				if (pready[i])
				{
					buffer_draw_sprite(glb.bmp(28), 275, 72 + (116 * i));
				}
			}
			i++;
		}
		
		if (!playerwait)
		{
			plyrcycle.check();
			buffer_rect(38, 36 + (116 * net.client_pnum), 128, 126 + (116 * net.client_pnum), BLACK);
			
			if (plyrcycle.clicked())
			{
				data.clear();
				data.addByte(NET_CHARACTER);
				data.addByte(net.client_pnum);
				send_datablob(net.thread, &data);
				
				playerwait = true;
			}
		}
		else
		{
			plyrcycle.draw();
			buffer_rectrounded(plyrcycle.x, plyrcycle.y, plyrcycle.x + plyrcycle.w, plyrcycle.y + plyrcycle.h, BLACK, 0.7, 3.0);
		}
		
		if (!teamwait)
		{
			teamcycle.check();
			if (teamcycle.clicked())
			{
				data.clear();
				data.addByte(NET_PLAYERTEAM);
				data.addByte(net.client_pnum);
				data.addByte(player[net.client_pnum].team);
				send_datablob(net.thread, &data);
				
				teamwait = true;
			}
		}
		else
		{
			teamcycle.draw();
			buffer_rectrounded(teamcycle.x, teamcycle.y, teamcycle.x + teamcycle.w, teamcycle.y + teamcycle.h, BLACK, 0.7, 3.0);
		}
		
		if (!tintwait && (currentcharacter[net.client_pnum] != NULL))
		{
			colorcycle.check();
			if (colorcycle.clicked())
			{
				data.clear();
				data.addByte(NET_CLIENT_TINTCYCLE);
				send_datablob(net.thread, &data);
				
				tintwait = true;
			}
		}
		else if (currentcharacter[net.client_pnum] != NULL)
		{
			colorcycle.draw();
			buffer_rectrounded(colorcycle.x, colorcycle.y, colorcycle.x + colorcycle.w, colorcycle.y + colorcycle.h, BLACK, 0.7, 3.0);
		}
	}
	else  //if it hasn't loaded the initial player data yet
	{
		buffer_rectrounded(29, 27, 270, 481, BLACK, 0.7, 5.0);
		buffer_textout_ex(big, "Loading...", 101, 240, WHITE);
	}
	
	if (back.clicked())
	{
		empdebug("Back button clicked, returning to network main menu.");
		net.engine->disconnectThread(net.thread);
		net.chatengine->disconnectThread(net.chatthread);
		first = true;
		return NET_FIRST;
	}
	
	draw_mouse();
	
	return NET_JOIN;
}

bool do_clientchat(_listbox *chatbox, string str)
{  //returns true if it got a new item to display
	static DyBytes recv;
	bool result = false;
	string chatstr;
	unsigned int i, j;
	
	if ((str != "/NOTHING YET/") && (str != ""))
	{
		chatstr = global.netname;
		chatstr += ": ";
		chatstr += str;
		net.chatdata.addByte(chatstr.length() % 256);
		
		if (chatstr.length() > 255) net.chatdata.addByte(chatstr.length() - 255);
		else net.chatdata.addByte(0);  //ugly UGLY way of adding a length parameter (max length of 2 * 255)
		
		j = 0;
		while (j < chatstr.length())
		{
			net.chatdata.addByte(chatstr[j]);
			j++;
		}
		
		net.chatengine->sendData(net.chatthread, net.chatdata);
		net.chatdata.clear();
	}
	
	net.chatengine->getData(net.chatthread, net.chatdata);
	if (net.chatdata.size() > 0)
	{
		recv.append(net.chatdata);
		
		if (recv.size() > 2)
		{
			j = recv.getByte(0) + recv.getByte(1);
			
			while (recv.size() >= j)
			{
				recv.removeFront(2);
				chatstr = "";
				i = 0;
				while (i < j)
				{
					chatstr += recv.getByte(i);
					i++;
				}
				recv.removeFront(j);
				
				chatbox->additem(chatstr.c_str());
				chatbox->scroll_to_end();
				result = true;
				
				if (recv.size() > 2) j = recv.getByte(0) + recv.getByte(1);
			}
		}
		net.chatdata.clear();
	}
	return result;
}

void load_inidata(DyBytes *data)
{
	char *str;
	int i, j, k, n;
	
	data->decompress();
	
	i = 0;
	while (i < 3)
	{
		data->extractVariable(0, armystats[i].pricefraction);
		data->removeFront(sizeof(armystats[i].pricefraction));
		
		data->extractVariable(0, armystats[i].movemod);
		data->removeFront(sizeof(armystats[i].movemod));
		
		data->extractVariable(0, armystats[i].rangemaxmod);
		data->removeFront(sizeof(armystats[i].rangemaxmod));
		
		j = 0;
		while (j < 3)
		{
			data->extractVariable(0, armystats[i].attackfraction[j]);
			data->removeFront(sizeof(armystats[i].attackfraction[j]));
			
			data->extractVariable(0, armystats[i].resistfraction[j]);
			data->removeFront(sizeof(armystats[i].resistfraction[j]));
			j++;
		}
		i++;
	}
	
	n = data->getByte(0);
	data->removeFront(1);
	if (n != NUM_UNITS)
	{
		error_different_numunits = true;
		return;
	}
	
	i = 0;
	while (i < n)
	{
		data->extractVariable(0, j);
		data->removeFront(sizeof(int));
		str = (char *)malloc(sizeof(char) * (j + 1));
		k = 0;
		while (k < j)
		{
			str[k] = data->getByte(0);
			data->removeFront(1);
			k++;
		}
		str[k] = '\0';
		unitstats[i].name = str;
		free(str);
		
		data->extractVariable(0, j);
		data->removeFront(sizeof(int));
		str = (char *)malloc(sizeof(char) * (j + 1));
		k = 0;
		while (k < j)
		{
			str[k] = data->getByte(0);
			data->removeFront(1);
			k++;
		}
		str[k] = '\0';
		unitstats[i].description = str;
		free(str);
		
		data->extractVariable(0, unitstats[i]._move);
		data->removeFront(sizeof(unitstats[i]._move));
		
		data->extractVariable(0, unitstats[i].movetype);
		data->removeFront(sizeof(unitstats[i].movetype));
		
		data->extractVariable(0, unitstats[i].attacktype);
		data->removeFront(sizeof(unitstats[i].attacktype));
		
		data->extractVariable(0, unitstats[i].basetype);
		data->removeFront(sizeof(unitstats[i].basetype));
		
		data->extractVariable(0, unitstats[i]._rangemin);
		data->removeFront(sizeof(unitstats[i]._rangemin));
		
		data->extractVariable(0, unitstats[i]._rangemax);
		data->removeFront(sizeof(unitstats[i]._rangemax));
		
		data->extractVariable(0, unitstats[i]._price);
		data->removeFront(sizeof(unitstats[i]._price));
		
		data->extractVariable(0, unitstats[i].loadcapacity);
		data->removeFront(sizeof(unitstats[i].loadcapacity));
		
		data->extractVariable(0, unitstats[i].loadtype);
		data->removeFront(sizeof(unitstats[i].loadtype));
		
		data->extractVariable(0, unitstats[i].canhit);
		data->removeFront(sizeof(unitstats[i].canhit));
		
		data->extractVariable(0, unitstats[i].attackparticle);
		data->removeFront(sizeof(unitstats[i].attackparticle));
		
		data->extractVariable(0, unitstats[i].attacksound);
		data->removeFront(sizeof(unitstats[i].attacksound));
		
		data->extractVariable(0, unitstats[i].movesound);
		data->removeFront(sizeof(unitstats[i].movesound));
		
		data->extractVariable(0, unitstats[i].moveparticle);
		data->removeFront(sizeof(unitstats[i].moveparticle));
		
		data->extractVariable(0, unitstats[i].cancapture);
		data->removeFront(sizeof(unitstats[i].cancapture));
		
		data->extractVariable(0, unitstats[i].isarmy[0]);
		data->removeFront(sizeof(unitstats[i].isarmy[0]));
		
		data->extractVariable(0, unitstats[i].isarmy[1]);
		data->removeFront(sizeof(unitstats[i].isarmy[1]));
		
		data->extractVariable(0, unitstats[i].isarmy[2]);
		data->removeFront(sizeof(unitstats[i].isarmy[2]));
		
		data->extractVariable(0, unitstats[i].techlevel);
		data->removeFront(sizeof(unitstats[i].techlevel));
		
		data->extractVariable(0, unitstats[i].attacksburn);
		data->removeFront(sizeof(unitstats[i].attacksburn));
		
		data->extractVariable(0, unitstats[i].blastradius);
		data->removeFront(sizeof(unitstats[i].blastradius));
		
		data->extractVariable(0, unitstats[i].attackdelay);
		data->removeFront(sizeof(unitstats[i].attackdelay));
		
		j = 0;
		while (j < 3)
		{
			data->extractVariable(0, unitstats[i]._attack[j]);
			data->removeFront(sizeof(unitstats[i]._attack[j]));
			
			data->extractVariable(0, unitstats[i]._resist[j]);
			data->removeFront(sizeof(unitstats[i]._resist[j]));
			j++;
		}
		i++;
	}
}

void load_playerdata(DyBytes *data)
{
	int i, k, d;
	char str[50];
	
	data->decompress();
	
	net.client_pnum = data->getByte(0);
	data->removeFront(1);
	
	i = 0;
	while (i < 4)
	{
		player[i].name = "";
		player[i].playing = 0;
		player[i].everplaying = 0;
		if (i != 0) net.address[i] = "";
		
		d = data->getByte(0);
		data->removeFront(1);
		
		if (d != 0)
		{
			player[i].playing = 1;
			player[i].everplaying = 1;
			
			k = 0;
			while (k < d)
			{
				str[k] = data->getByte(k);
				k++;
			}
			str[k] = '\0';
			player[i].name = str;
			data->removeFront(d);
			
			player[i].number = (signed char)data->getByte(0);
			if (player[i].number == -1) currentcharacter[i] = NULL;
			else currentcharacter[i] = character[player[i].number];
			player[i].controller = data->getByte(1);
			player[i].team = data->getByte(2);
			data->removeFront(3);
			data->extractVariable(0, player[i].tintcolor);
			data->removeFront(sizeof(int));
			
			d = data->getByte(0);
			data->removeFront(1);
			k = 0;
			while (k < d)
			{
				str[k] = data->getByte(k);
				k++;
			}
			str[k] = '\0';
			net.address[i] = str;
			data->removeFront(d);
		}
		i++;
	}
	
	empdebug("net.address:");
	empdebug(net.address[0]);
	empdebug(net.address[1]);
	empdebug(net.address[2]);
	empdebug(net.address[3]);
}

void send_client_playerdata()
{
	unsigned int i;
	DyBytes data;
	data.clear();
	
	data.addByte(global.netname.length());
	i = 0;
	while (i < global.netname.length())
	{
		data.addByte(global.netname[i]);
		i++;
	}
	data.compress(9);
	data.insertByte(NET_PLAYERDATA);
	
	send_datablob(net.thread, &data);
}

void load_menudata(DyBytes *data)
{
	int i, c;
	
	data->decompress();
	var.cashturns = data->getByte(0);
	c = data->getByte(1);
	i = 0;
	while (i < 4)
	{
		player[i].cash = c * 1000;
		i++;
	}
	var.techlevel = data->getByte(2);
	gameoptions.funding = data->getByte(3) * 1000;
	gameoptions.show_damage = data->getByte(4);
}

void list_gamestats(int x, int y)
{
	char c[50];
	
	buffer_rectfill(x, y, x + 180, y + 110, WHITE, 0.6);
	buffer_rect(x, y, x + 180, y + 110, BLACK);
	sprintf(c, "Starting Cash: %d", player[0].cash);
	buffer_textout_ex(normal, c, x + 10, y + 8, BLACK);
	sprintf(c, "Funds per Building: %d", gameoptions.funding);
	buffer_textout_ex(normal, c, x + 10, y + 28, BLACK);
	if (gameoptions.show_damage > 0)
	{
		buffer_textout_ex(normal, "Show Attacking Damage: Yes", x + 10, y + 48, BLACK);
	}
	else
	{
		buffer_textout_ex(normal, "Show Attacking Damage: No", x + 10, y + 48, BLACK);
	}
	sprintf(c, "Tech Level: %d", var.techlevel);
	buffer_textout_ex(normal, c, x + 10, y + 68, BLACK);
	if (var.cashturns == 0)
	{
		buffer_textout_ex(normal, "Derricks Last: Forever", x + 10, y + 88, BLACK);
	}
	else
	{
		sprintf(c, "Derricks Last: %d Turns", var.cashturns);
		buffer_textout_ex(normal, c, x + 10, y + 88, BLACK);
	}
}
