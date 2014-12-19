#ifndef _NETWORKING_H
#define _NETWORKING_H

#include <DyConnect.h>

#define NET_FIRST 0
#define NET_JOIN  1
#define NET_HOST  2
#define NET_PLAY  3
#define NET_QUIT  4

#define NET_PLAYERTINT       4
#define NET_PLAYERTEAM       5
#define NET_ASK_PLAYERDATA   6
#define NET_FUNDSPERBUILDING 7
#define NET_SHOWDAMAGE       8
#define NET_TIMELIMIT        9
#define NET_MAPCHANGE        10
#define NET_MAPDATA          11
#define NET_MAPLOADED        12
#define NET_CLIENTREADY      13
#define NET_CHARACTER        14
#define NET_STARTINGCASH     15
#define NET_TECHLEVEL        16
#define NET_CASHTURNS        17
#define NET_UNREADYALL       18
#define NET_GO               19
#define NET_CLIENT_TINTCYCLE 20

#define NET_UPDATINGTILES    21
#define NET_UNITHEALTH       22
#define NET_FINISHEDSYNC     23
#define NET_INIDATA          24
#define NET_PLAYERGONE       25
#define NET_NEWTURN          26
#define NET_NEWUNIT          27
#define NET_PLAYERCASH       28
#define NET_MOVEUNIT         29
#define NET_STANDBY          30
#define NET_DEPLOYPORTAL     31
#define NET_MECHANIC_CAPTURE 32
#define NET_UNLOAD           33
#define NET_TELEPORT         34
#define NET_BATTLEDATA       35
//#define NET_ATTACK           35
//#define NET_COUNTERATTACK    36
//#define NET_TREEBURN         37
//#define NET_BATTLEOVER       38
#define NET_OHNOYOUDONT      39

#define NET_CLIENT_ENDTURN   40
#define NET_CLIENT_NEWUNIT   41
#define NET_CLIENT_MOVEUNIT  42
#define NET_CLIENT_STANDBY   43
#define NET_CLIENT_DEPLOY    44
#define NET_CLIENT_MECHCAP   45
#define NET_CLIENT_UNLOAD    46
#define NET_CLIENT_TELEPORT  47
#define NET_CLIENT_ATTACK    48
#define NET_CLIENT_SURRENDER 49

#define NET_STATS            50
#define NET_PLAYERDATA       51
#define NET_MENUDATA         52
#define NET_PLAYER_UPDATE    53
#define NET_PLAYER_SURRENDER 54
#define NET_RESUMEGAME       55
#define NET_NEWCONTROLLER    56

#define gameid 0

#define MAX_CONNECTIONS 6

typedef struct _netstuff
{
	bool server;
	int thread, chatthread, clientthread[3], client_pnum, hostnum;
	string address[4];
	DyBytes data[3], chatdata;
	NetworkEngine *engine;  //for the main stuff: user and movement data
	NetworkEngine *chatengine;  //for chatting between players
	
	PollingConnect poll, chatpoll;
	
	_netstuff();
	~_netstuff();
	
	bool init(bool is_server);
	void reset();
} _netstuff;

typedef struct udphost
{
	unsigned char major, minor;
	short num;
	unsigned int since_update;
	string name, mapname, address;
	bool extract_data(string addr, unsigned char *data);
} udphost;

typedef struct _udphandler
{
	unsigned char *tosend;
	unsigned int length;
	
	void close();
	void send(unsigned char *newdata, int len);
	bool open();
	bool update();
	
	bool opened;
	UDPSock udp;
	
	_udphandler();
	~_udphandler();
} _udphandler;

extern bool unlisted_game;

extern _netstuff net;
extern _udphandler udphandler;

extern void create_sidebars();
extern void description(_character *c, int x, int y, bool centeredtext);

bool init_networking();
void stop_networking();

void clear_clientdata(int client);
void send_datablob(int thread, DyBytes *data);
void broadcast_datablob(DyBytes *data);
DyBytes get_datablob(int thread, DyBytes *data);
DyBytes get_client_datablob(int client);
DyBytes get_host_datablob();

void load_netplayer(int number, DyBytes *data);

DyBytes mapdata();
void send_mapdata(int thread);
void broadcast_mapdata();
void load_mapdata(DyBytes *data);

bool clientconnected(int client);
int network();
int net_choosemode(short err = 0);
int net_joingame();
int net_hostgame();

bool do_hostchat(_listbox *chatbox, string str);
bool do_clientchat(_listbox *chatbox, string str);
void broadcast_chat_message(string mesg);

#endif
