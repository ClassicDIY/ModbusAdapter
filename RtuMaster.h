#pragma once
#include "Arduino.h"

#define RTU_BUFFER_SIZE 256
#define RtuTimeout 1000
#define RtuRetryCount 10

#define READ_HOLDING_REGISTERS 3
// modbus specific exceptions
#define ILLEGAL_FUNCTION 1
#define ILLEGAL_DATA_ADDRESS 2
#define ILLEGAL_DATA_VALUE 3

struct Status {
	typedef enum {
		ok,
		nothing,
		incorrect_id_returned,
		incorrect_function_returned,
		incorrect_bytes_returned,
		checksum_failed,
		buffer_errors,
		illegal_function,
		illegal_data_address,
		illegal_data_value,
		misc_exceptions,
	} RtuError;
};




class RtuMaster
{
	unsigned char _frame[RTU_BUFFER_SIZE];
	unsigned int T1_5; // inter character time out in microseconds
	unsigned int T3_5; // frame delay in microseconds
	unsigned char slaveId;
	unsigned int _transactionId;
	Status::RtuError rtuError;

public:
	RtuMaster();
	~RtuMaster();
	void Init(long baudRate, unsigned char id);
	void Transfer(unsigned int transactionId, byte* v);
	Status::RtuError TransferBack(byte *frame);
	void Read(unsigned int address, unsigned int no_of_registers);
	Status::RtuError CheckResponse(unsigned int no_of_registers, unsigned int* register_array);
	Status::RtuError Error() { return rtuError; }



private:
	unsigned int calculateCRC(unsigned char bufferSize);
	void sendPacket(unsigned char bufferSize);
	
	unsigned char getData();
	void check_F3_data(unsigned char buffer, unsigned int no_of_registers, unsigned int* register_array);
};

