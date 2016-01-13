#pragma once

#include <ESP8266WiFi.h>
#include "RtuMaster.h"

#define MODBUSIP_PORT 	  502
#define MODBUSIP_MAXFRAME 200
#define MODBUSIP_TIMEOUT   10
#define TCP_BUFFER_SIZE 260

class TcpSlave
{
private:
	byte _MBAP[7];
	byte _sendbuffer[TCP_BUFFER_SIZE];
	WiFiClient _client;
	byte *_frame;
	byte  _len;
	RtuMaster _rtuMaster;

public:
	TcpSlave();
	~TcpSlave();
	void config(const char* ssid, const char* password, long baudRate);
	void task();
};

