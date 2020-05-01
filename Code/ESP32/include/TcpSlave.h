#pragma once

#include <Arduino.h>
#include <WiFiServer.h>
#include "RtuMaster.h"

#define MODBUS_DEFAULT_PORT 502
#define MODBUSIP_MAXFRAME 200
#define MODBUSIP_MAX_CLIENTS 4
#define MODBUSIP_MAX_READMS 100
#define MODBUSIP_MAX_RESPMS 2000

namespace ModbusAdapter
{

class TcpSlave
{
	protected:
	typedef union MBAP_t {
		struct {
			uint16_t transactionId;
			uint16_t protocolId;
			uint16_t length;
			uint8_t	 unitId;
		};
		uint8_t  raw[7];
	}MBAP_u;

	uint8_t*  _frame = nullptr;
	uint16_t  _len = 0;
	RtuMaster _rtuMaster;
	int8_t _clientIndex = -1;
	uint16_t _slavePort = 0;
	WiFiServer* _pServer;
	WiFiClient* _pClients[MODBUSIP_MAX_CLIENTS];
	RtuReplyState _rtuState;
	void cleanup();
	int8_t getFreeClient();
	void exceptionResponse(FunctionCode fn, ResultCode excode);

public:
	TcpSlave();
	void cbResponse(ResultCode res, void *data, uint16_t len);
	void init(long baudRate, long tcpPort, uint8_t mosbusAddress);
	void close();
	void run();
};
}
