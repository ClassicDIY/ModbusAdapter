#pragma once
#include "Arduino.h"
#include "Enumerations.h"

#define InterFrameDelay 2
#define MODBUSRTU_TIMEOUT 3000

namespace ModbusAdapter
{
class RtuMaster
{

	byte _slaveId; // unit ID sent to slave, cleared after
	byte _unitId; // configured unit ID from setup
	Stream* _port;
	int16_t _rtsPin = -1;
	uint32_t _timestamp = 0;
	uint8_t _sentFunctionCode = 0;
	void* _callBackFunction = nullptr;
	unsigned int _interFrameDelay;	// inter-frame delay in mS
	uint32_t _lastTimeStamp = 0;
	uint16_t _len = 0;

public:
	RtuMaster();
	void Init(long baudRate, uint8_t mosbussAddress);
	bool Transfer(byte* frame, uint16_t len, void* cb);
	void run();
    void reset();

private:
	bool rawSend(uint8_t slaveId, uint8_t* frame, uint8_t len);
	bool cleanup(); 	
	uint16_t crc16(uint8_t address, uint8_t* frame, uint8_t pdulen);
};
}