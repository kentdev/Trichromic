#include "empire.h"

_udphandler::_udphandler()
{
	tosend = NULL;
	length = 0;
	opened = false;
}

_udphandler::~_udphandler()
{
	if (tosend)
	{
		free(tosend);
		tosend = NULL;
	}
	close();
}

bool _udphandler::open()
{
	if (opened)
	{
		empdebug("UDP socket was already opened");
		return true;
	}
	
	if (udp.openSocket(41138))
	{
		empdebug("UDP socket opened");
		opened = true;
		return true;
	}
	return false;
}

void _udphandler::close()
{
	if (opened)
	{
		udp.closeSocket();
		empdebug("UDP socket closed");
	}
	opened = false;
}

bool _udphandler::update()
{
	int i, j;
	
	if (tosend)
	{
		if (!opened)
		{
			empdebug("UDP socket not opened");
			return false;
		}
		
		i = udp.sendData(global.netlobby, 41138, tosend, length);
		if (i < 0)
		{
			empdebug(udp.getErrMsg());
			return false;
		}
		
		j = 0;
		while (j < i)
		{
			tosend[j] = tosend[j + i];
			j++;
		}
		length -= i;
		if (length > 0) tosend = (unsigned char *)realloc(tosend, sizeof(char) * length);
		else
		{
			free(tosend);
			tosend = NULL;
		}
	}
	return true;
}

void _udphandler::send(unsigned char *newdata, int len)
{
	if (tosend)
	{
		tosend = (unsigned char *)realloc(tosend, sizeof(char) * (length + len));
	}
	else tosend = (unsigned char *)malloc(sizeof(char) * len);
	
	memcpy(tosend + length, newdata, len);
	length += len;
}
