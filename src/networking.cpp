#include "empire.h"

_netstuff net;
_udphandler udphandler;
_character *netcharacter[4];
ol::Bitmap *netbmp[4];

bool unlisted_game;

extern void refresh_maplist();
extern void get_teleports();

_netstuff::_netstuff()
{
	engine = NULL;
	chatengine = NULL;
}

_netstuff::~_netstuff()
{
	reset();
}

bool _netstuff::init(bool is_server)
{
	reset();
	engine = new NetworkEngine();
	chatengine = new NetworkEngine();
	engine->setTimeout(0);
	engine->limitReceiveRate(30);
	engine->setMaxConnections(MAX_CONNECTIONS);
	chatengine->setTimeout(0);
	chatengine->limitReceiveRate(30);
	chatengine->setMaxConnections(MAX_CONNECTIONS);
	server = is_server;
	if ((!engine->initialize()) || (!chatengine->initialize()))
	{
		return false;
	}
	return true;
}

void _netstuff::reset()
{
	if (engine != NULL)
	{
		engine->disableTCPListen();
		delete engine;
		engine = NULL;
	}
	if (chatengine != NULL)
	{
		chatengine->disableTCPListen();
		delete chatengine;
		chatengine = NULL;
	}
	data[0].clear();
	data[1].clear();
	data[2].clear();
	chatdata.clear();
}

bool init_networking()
{
	if (!DyConnect::startup())
	{
		return false;
	}
	return true;
}

void stop_networking()
{
	net.reset();
	DyConnect::shutdown();
}


bool udphost::extract_data(string addr, unsigned char *data)
{  //data starts at 1, 0 holds the total length of the data
	unsigned char namelen, mapnamelen, addrlen, offset;
	char *str;
	
	addrlen = data[1];
	str = (char *)malloc(sizeof(char) * addrlen);
	memcpy(str, &data[2], addrlen);
	address = str;
	free(str);
	
	major = data[addrlen + 3];
	minor = data[addrlen + 4];
	offset = addrlen + 4;
	
	num = data[offset + 1];
	namelen = data[offset + 2];
	mapnamelen = data[offset + 3];
	
	if (namelen + mapnamelen > data[offset]) return false;
	
	str = (char *)malloc(sizeof(char) * (namelen + 1));
	memcpy(str, &data[offset + 4], namelen);
	str[namelen] = '\0';
	name = str;
	free(str);
	
	str = (char *)malloc(sizeof(char) * (mapnamelen + 1));
	memcpy(str, &data[offset + 4 + namelen], mapnamelen);
	str[mapnamelen] = '\0';
	mapname = str;
	free(str);
	
	since_update = 0;
	
	return true;
}

//==============================================================================
//==============================================================================

int network()
{
	static bool first = true;
	static int netstatus = NET_FIRST;
	
	if (first)
	{
		netstatus = NET_FIRST;
		first = false;
	}
	
	switch (netstatus)
	{
		case NET_FIRST:
			netstatus = net_choosemode();
			break;
		case NET_JOIN:
			netstatus = net_joingame();
			break;
		case NET_HOST:
			netstatus = net_hostgame();
			break;
		case NET_PLAY:
			//first = true;
			global.very_first_turn = 1;
			global.netgame = true;
			get_teleports();
			clear_particles();
			
			if (net.server) netstatus = NET_HOST;
			else netstatus = NET_JOIN;
			
			return LOGIC;
			break;
		case NET_QUIT:
			first = true;
			return MENU;
			break;
	}
	
	return NETWORK;
}

int net_choosemode(short err)
{
	static bool veryfirst = true, first = true, udpOK = false, connecting = false;
	static short error;
	static int since_update;
	static _button back, bclient, bserver, bserver_unlisted;
	static _textline text;
	static _listbox hostlist;
	static unsigned char data[2];
	static vector<udphost> hosts;
	bool found, waiting;
	int p, len;
	unsigned int i;
	string a;
	unsigned char buffer[256];
	char hosttext[100];
	udphost temphost;
	vector<udphost>::iterator it;
	
	if (veryfirst)
	{
		text.init(320, 260, 290, 13, false);
		veryfirst = false;
	}  //it only iniitializes the text box once, so that whatever text it held is not erased later
	if (first)
	{
		back.init(10, 575, 100, 20, "Back");
		bclient.init(230, 210, 380, 40, "Join a Network Game");
		bserver.init(230, 310, 185, 40, "Host a Network Game");
		bserver_unlisted.init(425, 310, 185, 40, "Host an Unlisted Network Game");
		
		hostlist.init(220, 15, 400, 170);
		error = err;
		first = false;
		connecting = false;
		net.thread = DY_BUSYUNREACHABLE;
		net.chatthread = DY_BUSYUNREACHABLE;
		udpOK = udphandler.open();//udp.openSocket(41138);
		since_update = 100;
		data[0] = 1;
		data[1] = gameid;
		hosts.clear();
		play_song(title_music);
	}
	
	glb.draw(54, 0, 0);
	
	sprintf(hosttext, "Version %u.%02u", REVISION_MAJOR, REVISION_MINOR);
	buffer_textout_ex(normal, hosttext, 3, 3, WHITE);
	
	buffer_rectrounded(220, 200, 620, 290, BLACK, 0.4, 5.0);
	buffer_textout_ex(normal, "Host IP Address:", 230, 260, WHITE);
	buffer_rectrounded(220, 300, 620, 360, BLACK, 0.4, 5.0);
	
	if (connecting)
	{
		buffer_textout_ex(normal, "Connecting...", 220, 185, GREEN);
		back.draw_disabled();
		bclient.check();
		bserver.draw_disabled();
		waiting = false;
		
		if (bclient.clicked())
		{
			net.poll.cancel();
			net.chatpoll.cancel();
			connecting = false;
			bclient.init(230, 210, 380, 40, "Join a Network Game");
		}
		else
		{
			if (net.thread == -1)
			{
				p = net.poll.checkConnection();
				if (p >= 0)
				{
					net.thread = p;
				}
				else if (p != DY_BUSYUNREACHABLE)
				{
					net.poll.cancel();
					net.chatpoll.cancel();
					connecting = false;
					bclient.init(230, 210, 380, 40, "Join a Network Game");
					error = -1;
					waiting = true;
				}
				else waiting = true;
			}
			if (net.chatthread == -1)
			{
				p = net.chatpoll.checkConnection();
				if (p >= 0)
				{
					net.chatthread = p;
				}
				else if (p != DY_BUSYUNREACHABLE)
				{
					net.poll.cancel();
					net.chatpoll.cancel();
					connecting = false;
					bclient.init(230, 210, 380, 40, "Join a Network Game");
					error = -1;
					waiting = true;
				}
				else waiting = true;
			}
			if (!waiting)
			{
				if (udpOK) udphandler.close();//udp.closeSocket();
				connecting = false;
				first = true;
				return NET_JOIN;
			}
		}
		
		draw_mouse();
		return NET_FIRST;
	}
	
	udpOK = udphandler.update();
	since_update++;
	if (since_update > 100)
	{
		if (udpOK)
		{
			since_update = 0;
			//if (udp.sendData(global.netlobby, 41138, data, 2) < 0) udpOK = false;
			//counter = 0;
			udphandler.send(data, 2);
		}
		else
		{
			udpOK = udphandler.open();//udp.openSocket(41138);
		}
	}
	if (!udpOK) buffer_textout_ex(normal, "Couldn't open socket on port 41138", 220, 2, RED);
	
	if (udpOK)
	{
		if (hosts.size() > 0) buffer_textout_ex(normal, "Attempting to play against people with a different version number may lead to bad things happening", 220, 588, LGREY);
		
		len = udphandler.udp.recvData(a, p, buffer, 256);
		if (len > 0)
		{
			empdebug("Received UDP data");
			if ((unsigned char)len == buffer[0])
			{
				if (temphost.extract_data(a, buffer))
				{
					found = false;
					i = 0;
					while ((i < hosts.size()) && !found)
					{
						if (temphost.name == hosts[i].name)
						{
							hosts[i].mapname = temphost.mapname;
							hosts[i].num = temphost.num;
							hosts[i].since_update = 0;
							
							hostlist.clear();
							i = 0;
							while (i < hosts.size())
							{
								sprintf(hosttext, "Version %u.%02u     %s     %d players     Map: %s", hosts[i].major, hosts[i].minor, hosts[i].name.c_str(), hosts[i].num, hosts[i].mapname.c_str());
								hostlist.additem(hosttext);
								i++;
							}
							
							found = true;
						}
						i++;
					}
					if (!found)
					{
						hosts.push_back(temphost);
						sprintf(hosttext, "Version %u.%u     %s     %d players     Map: %s", hosts[i].major, hosts[i].minor, hosts[i].name.c_str(), hosts[i].num, hosts[i].mapname.c_str());
						hostlist.additem(hosttext);
					}
				}
				else empdebug("Received UDP data that couldn't be read correctly, ignoring");
			}
			else empdebug("Received UDP data that didn't match its stated size, ignoring");
		}
		
		it = hosts.begin();
		while (it != hosts.end())
		{
			(*it).since_update++;
			if ((*it).since_update > 300)
			{
				it = hosts.erase(it);
				hostlist.clear();
				
				i = 0;
				while (i < hosts.size())
				{
					sprintf(hosttext, "%s: %d players, map: %s", hosts[i].name.c_str(), hosts[i].num, hosts[i].mapname.c_str());
					hostlist.additem(hosttext);
					i++;
				}
			}
			else it++;
		}
	}
	hostlist.check();
	p = hostlist.getsel();
	if (p != -1)
	{
		text.set_text(hosts[p].address);
		hostlist.setsel(-1);
	}
	
	back.check();
	
	bclient.check();
	bserver.check();
	bserver_unlisted.check();
	text.check();
	switch(error)
	{
		case 1:
			buffer_textout_ex(normal, "Error connecting to host", 220, 185, RED);
			break;
		case 2:
			buffer_textout_ex(normal, "Error listening on port 41136/41137, try again in a few seconds", 220, 185, RED);
			break;
		case 3:
			buffer_textout_ex(normal, "Disconnected from host", 220, 185, RED);
			break;
		case 4:
			buffer_textout_ex(normal, "You don't have any maps!", 220, 185, RED);
			break;
		case 5:
			buffer_textout_ex(normal, "Error initializing networking", 220, 185, RED);
			break;
		case 6:
			buffer_textout_ex(normal, "The host has a different number of units, crashing would be inevitable!", 220, 185, RED);
			break;
	}
	
	if (back.clicked())
	{
		if (udpOK) udphandler.close();//udp.closeSocket();
		first = true;
		return NET_QUIT;
	}
	
	if (bclient.clicked())
	{
		if (!net.init(false)) error = 5;
		else
		{
			net.address[0] = text.current_text();
			buffer_textout_ex(normal, "Connecting...", 220, 185, GREEN);
			bclient.init(230, 210, 380, 40, "Cancel");
			
			connecting = true;
			net.poll.connectToHost(net.address[0], 41136, net.engine);
			net.chatpoll.connectToHost(net.address[0], 41137, net.chatengine);
		}
	}
	if (bserver.clicked())
	{
		if (!net.init(true)) error = 5;
		else
		{
			unlisted_game = false;
			refresh_maplist();
			if (maplist.size() < 1)
			{
				error = 4;
			}
			else if ((!net.engine->enableTCPListen(41136)) || (!net.chatengine->enableTCPListen(41137)))
			{
				error = 2;
			}
			else
			{
				//if (udpOK) udphandler.close();//udp.closeSocket();
				first = true;
				return NET_HOST;
			}
		}
	}
	if (bserver_unlisted.clicked())
	{
		if (!net.init(true)) error = 5;
		else
		{
			unlisted_game = true;
			refresh_maplist();
			if (maplist.size() < 1)
			{
				error = 4;
			}
			else if ((!net.engine->enableTCPListen(41136)) || (!net.chatengine->enableTCPListen(41137)))
			{
				error = 2;
			}
			else
			{
				if (udpOK) udphandler.close();//udp.closeSocket();
				first = true;
				return NET_HOST;
			}
		}
	}
	
	draw_mouse();
	
	return NET_FIRST;
}

void send_datablob(int thread, DyBytes *data)
{
	unsigned long int spacer = 1000000000;
	char size[15];
	DyBytes send;
	
	while (spacer > data->size())
	{  //add leading zeroes
		spacer /= 10;
		send.addString("0");
	}
	sprintf(size, "%lu", data->size());
	send.addString(size);
	net.engine->sendData(thread, send);
	net.engine->sendData(thread, *data);
	send.clear();
}

void broadcast_datablob(DyBytes *data)
{
	int i = 0;
	while (i < net.engine->getMaxConnections())
	{
		if (net.engine->isConnected(i))
		{
			send_datablob(i, data);
		}
		i++;
	}
}

DyBytes get_client_datablob(int client)
{
	return get_datablob(net.clientthread[client], &net.data[client]);
}

DyBytes get_host_datablob()
{
	return get_datablob(net.thread, &net.data[0]);
}

DyBytes get_datablob(int thread, DyBytes *data)
{  //accumulates all data from "thread" into "data", and removes and returns the next completed data chunk
	static bool gotsize = false;
	static unsigned long int filesize = 0;
	stringstream converter;
	DyBytes size;
	DyBytes result;
	
	net.engine->getData(thread, *data);
	if (!gotsize)
	{
		if (data->size() > 10)
		{
			size.append(*data, 0, 10);
			data->removeFront(10);
			converter << size;
			converter >> filesize;
			gotsize = true;
		}
	}
	else
	{
		if (data->size() >= filesize)
		{  //this blob has been fully downloaded
			result.append(*data, 0, filesize);
			data->removeFront(filesize);
			gotsize = false;
			filesize = 0;
		}
	}
	return result;
}

void clear_clientdata(int client)
{
	net.engine->getData(net.clientthread[client], net.data[client]);
	net.data[client].clear();
}

DyBytes mapdata()
{
	int i, j, n;
	DyBytes data, mapdata;
	
	data.addByte(NET_MAPCHANGE);  //tell the connected players that the map has changed
	broadcast_datablob(&data);
	data.clear();
	
	mapdata.addByte(worldmap.l);  //then send the actual map data in a separate blob
	mapdata.addByte(worldmap.h);
	mapdata.addByte(numplayers);
	
	j = 0;
	while (j < worldmap.h)
	{
		i = 0;
		while (i < worldmap.l)
		{
			mapdata.addByte(worldmap.tile[i][j].get_set());
			mapdata.addByte(worldmap.tile[i][j].get_number());
			mapdata.addByte(worldmap.tile[i][j].get_owner());
			i++;
		}
		j++;
	}
	
	n = 0;
	i = 0;
	while (i < 4)
	{
		if (player[i].playing == 1)
		{
			n += player[i].units_in_play();
		}
		i++;
	}
	mapdata.addByte(n);
	
	i = 0;
	while (i < 4)
	{
		if (player[i].playing == 1)
		{
			j = 0;
			while (j < 50)
			{
				if (player[i].unit[j].exists == 1)
				{
					mapdata.addByte(player[i].unit[j].type);
					mapdata.addByte(player[i].unit[j].tilex);
					mapdata.addByte(player[i].unit[j].tiley);
					mapdata.addByte(i);
				}
				j++;
			}
		}
		i++;
	}
	mapdata.addString(worldmap.get_name());
	
	mapdata.compress(9);
	data.addByte(NET_MAPDATA);
	data.append(mapdata);
	return data;
}

void send_mapdata(int thread)
{
	DyBytes data = mapdata();
	send_datablob(thread, &data);
}

void broadcast_mapdata()
{
	DyBytes data = mapdata();
	broadcast_datablob(&data);
}

void load_mapdata(DyBytes *data)
{
	int i, j, t, x, y, p, o;
	
	data->decompress();
	worldmap.reset();
	player[0].clear_units();
	player[1].clear_units();
	player[2].clear_units();
	player[3].clear_units();
	
	worldmap.l = data->getByte(0);
	worldmap.h = data->getByte(1);
	numplayers = data->getByte(2);
	data->removeFront(3);
	
	i = 0;
	while (i < numplayers)
	{
		player[i].everplaying = 1;
		player[i].playing = 1;
		i++;
	}
	while (i < 4)
	{
		player[i].everplaying = 0;
		player[i].playing = 0;
		i++;
	}
	
	y = 0;
	while (y < worldmap.h)
	{
		x = 0;
		while (x < worldmap.l)
		{
			i = data->getByte(0);
			j = data->getByte(1);
			o = data->getByte(2);
			data->removeFront(3);
			worldmap.tile[x][y].change_type(i, j);
			//bytes are received as UNSIGNED, so -1 is interpreted as 255, which need correcting
			if ((o >= 0) && (o <= 3)) worldmap.tile[x][y].change_owner(o);
			else worldmap.tile[x][y].change_owner(-1);
			
			x++;
		}
		y++;
	}
	
	j = data->getByte(0);
	data->removeFront(1);
	i = 0;
	while (i < j)
	{
		t = data->getByte(0);
		x = data->getByte(1);
		y = data->getByte(2);
		p = data->getByte(3);
		data->removeFront(4);
		player[p].create_unit(t, x, y, p);
		i++;
	}
	worldmap.set_name(data->getAsString());
}

bool clientconnected(int client)
{
	if (client > 2) return false;
	return (net.clientthread[client] != -1);
}
