#pragma once
#include "Arduino.h"
#include "Enumerations.h"

#define InterFrameDelay 2
#define MODBUSRTU_TIMEOUT 2000

namespace ModbusAdapter
{
class RtuMaster
{
	byte _unitId; // configured unit ID from setup
	Stream* _port;
	int16_t _rtsPin = -1;
	void* _callBackFunction = nullptr;
	uint8_t _requestFrame[16];
	uint8_t _requestFrameLen;
public:
	RtuMaster();
	void Init(long baudRate, uint8_t mosbussAddress);
	void Transfer(byte* frame, uint16_t len, void* cb);
	void Run();

private:
	uint16_t crc16(uint8_t address, uint8_t* frame, uint8_t pdulen);
};


}