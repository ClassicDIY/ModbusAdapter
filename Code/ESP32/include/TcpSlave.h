#pragma once

#include <Arduino.h>
#include <WiFiServer.h>
#include "RtuMaster.h"

#define MODBUS_DEFAULT_PORT 502
#define MODBUSIP_MAXFRAME 256
#define TCP_BUFFER_SIZE 1024

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
	void init(long baudRate, long tcpPort, byte mosbusAddress);
	void close();
	void run();
};

