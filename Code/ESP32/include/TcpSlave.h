#pragma once

#include <Arduino.h>
#include <WiFiServer.h>
#include "RtuMaster.h"

#define MODBUSIP_PORT 	  502
#define MODBUSIP_MAXFRAME 200
#define MODBUSIP_TIMEOUT   10
#define TCP_BUFFER_SIZE 300

class TcpSlave
{
private:
	byte _MBAP[7];
	byte _sendbuffer[TCP_BUFFER_SIZE];
	WiFiClient _client;
	byte *_frame;
	byte  _len;
	unsigned int _timeToReceive;
	RtuMaster _rtuMaster;

public:
	TcpSlave();
	void init(long baudRate, unsigned char id);
	void run();
};

