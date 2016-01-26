/* 
	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

#pragma once
#include "Arduino.h"
#include <SoftwareSerial.h>

#define RTU_BUFFER_SIZE 264
#define RtuTimeout 1000
#define RtuRetryCount 10
#define SlaveId 10

#define READ_HOLDING_REGISTERS 3
// modbus specific exceptions
#define ILLEGAL_FUNCTION 1
#define ILLEGAL_DATA_ADDRESS 2
#define ILLEGAL_DATA_VALUE 3

extern SoftwareSerial _swSer;

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
	unsigned int Transfer(byte MBAP[], byte* v);
	Status::RtuError TransferBack(byte *frame);
	void Read(unsigned int address, unsigned int no_of_registers);
	Status::RtuError CheckResponse(unsigned int no_of_registers, unsigned int* register_array);
	Status::RtuError Error() { return rtuError; }



private:
	unsigned int calculateCRC(unsigned char bufferSize);
	void sendPacket(unsigned char bufferSize);
	
	Status::RtuError getData();
	void check_F3_data(unsigned char buffer, unsigned int no_of_registers, unsigned int* register_array);
};

