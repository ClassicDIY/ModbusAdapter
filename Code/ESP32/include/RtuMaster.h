#pragma once
#include "Arduino.h"
#include "Enumerations.h"

#define RTU_BUFFER_SIZE 264
#define RtuTimeout 1000
#define RtuRetryCount 10
#define InterFrameDelay 2
#define MODBUSRTU_TIMEOUT 2000
#define MODBUSRTU_BROADCAST 0

namespace ModbusAdapter
{
typedef bool (*cbTransaction)(ResultCode res, void* data, uint16_t len); // Callback for requests

struct TAddress {
    enum RegType {COIL, ISTS, IREG, HREG};
    RegType type;
    uint16_t address;
    bool operator==(const TAddress &obj) const { // TAddress == TAddress
	    return type == obj.type && address == obj.address;
	}
    TAddress& operator++() {     // ++TAddress
        address++;
        return *this;
    }
    TAddress  operator++(int) {  // TAddress++
        TAddress result(*this);
         ++(*this);
        return result;
    }
    TAddress& operator+=(const int& inc) {  // TAddress += integer
        address += inc;
        return *this;
    }
    const TAddress operator+(const int& inc) const {    // TAddress + integer
        TAddress result(*this);
        result.address += inc;
        return result;
    }
    bool isCoil() {
       return type == COIL;
    }
    bool isIsts() {
       return type == ISTS;
    }
    bool isIreg() {
        return type == IREG;
    }
    bool isHreg() {
        return type == HREG;
    }
};

class RtuMaster
{

	byte _slaveId; // unit ID sent to slave, cleared after
	byte _unitId; // configured unit ID from setup
	Stream* _port;
	int16_t _rtsPin = -1;
	uint32_t _timestamp = 0;
	uint8_t _sentFunctionCode = 0;
	cbTransaction _callBackFunction = nullptr;
	unsigned int _interFrameDelay;	// inter-frame delay in mS
	uint32_t _lastTimeStamp = 0;
	uint16_t _len = 0;

public:
	RtuMaster();
	void Init(long baudRate, uint8_t mosbussAddress, int16_t rtsPin = -1);
	bool Transfer(byte* frame, cbTransaction cb);
	void run();

private:
	uint8_t masterPDU(uint8_t* frame, uint8_t* sourceFrame, TAddress startreg, void* output);
	bool rawSend(uint8_t slaveId, uint8_t* frame, uint8_t len);
	bool cleanup(); 	
	uint16_t crc16(uint8_t address, uint8_t* frame, uint8_t pdulen);
};
}