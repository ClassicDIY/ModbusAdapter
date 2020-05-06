#pragma once

#include <Arduino.h>
#include <vector>
#include <WiFiServer.h>
#include "RtuMaster.h"

#define MODBUS_DEFAULT_PORT 502
#define MODBUSIP_MAXFRAME 200
#define MODBUSIP_MAX_CLIENTS 4
#define MODBUSIP_MAX_READMS 100
#define MODBUSIP_MAX_RESPMS 500
#define TRANSACTION_LIFESPAN 60000

namespace ModbusAdapter
{

typedef struct Transaction {
	uint32_t	timestamp;
	uint8_t*	_requestFrame = nullptr;
	uint16_t	_requestFrameLen = 0;
	uint8_t*	_responseFrame = nullptr;
	uint16_t	_responseFrameLen = 0;
}TransactionT;

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
	std::vector<Transaction> _trans;
	uint8_t*  _frame = nullptr;
	uint16_t  _len = 0;
	RtuMaster _rtuMaster;
	ResultCode _rtuResultCode;
	int8_t _clientIndex = -1;
	uint16_t _slavePort = 0;
	WiFiServer* _pServer;
	WiFiClient* _pClients[MODBUSIP_MAX_CLIENTS];
	void cleanup();
	int8_t getFreeClient();
	void exceptionResponse(FunctionCode fn, ResultCode excode);
	Transaction* searchTransaction(uint8_t* frame);
	boolean SupportedFunction(uint8_t fcode);

public:
	TcpSlave();
	void cbResponse(ResultCode res, uint8_t* requestFrame, uint8_t requestFrameLen, uint8_t *data, uint16_t len);
	void init(long baudRate, long tcpPort, uint8_t mosbusAddress);
	void close();
	void run();
};
}
