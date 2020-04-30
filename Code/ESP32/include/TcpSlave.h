#pragma once

#include <Arduino.h>
#include <WiFiServer.h>
#include "RtuMaster.h"

#define MODBUS_DEFAULT_PORT 502
#define MODBUSIP_MAXFRAME 250 // max 125 holding registers

namespace ModbusAdapter
{
class TcpSlave
{
private:
	uint8_t _MBAP[7];
	WiFiClient _client;
	RtuMaster _rtuMaster;

public:
	TcpSlave();
	void init(long baudRate, long tcpPort, uint8_t mosbusAddress);
	void close();
	void run();
};
}
