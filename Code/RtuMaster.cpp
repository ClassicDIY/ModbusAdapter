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
#include "RtuMaster.h"

RtuMaster::RtuMaster()
{
	rtuError = Status::ok;
}

RtuMaster::~RtuMaster()
{
}

void RtuMaster::Init(long baudRate, unsigned char id)
{
	_swSer.begin(baudRate);

	// Modbus states that a baud rate higher than 19200 must use a fixed 750 us 
	// for inter character time out and 1.75 ms for a _frame delay.
	// For baud rates below 19200 the timeing is more critical and has to be calculated.
	// E.g. 9600 baud in a 10 bit packet is 960 characters per second
	// In milliseconds this will be 960characters per 1000ms. So for 1 character
	// 1000ms/960characters is 1.04167ms per character and finaly modbus states an
	// intercharacter must be 1.5T or 1.5 times longer than a normal character and thus
	// 1.5T = 1.04167ms * 1.5 = 1.5625ms. A _frame delay is 3.5T.

	if (baudRate > 19200)
	{
		T1_5 = 750;
		T3_5 = 1750;
	}
	else
	{
		T1_5 = 15000000 / baudRate; // 1T * 1.5 = T1.5
		T3_5 = 35000000 / baudRate; // 1T * 3.5 = T3.5
	}
	slaveId = id;
}

unsigned int RtuMaster::Transfer(byte MBAP[], byte* v)
{
	_transactionId = (MBAP[0] << 8) | MBAP[1];
	//_frame[0] = MBAP[6];
	_frame[0] = SlaveId;
	for (int i = 0; i < 6; i++) {
		_frame[i+1] = v[i];
	}
	unsigned int crc16 = calculateCRC(6); // the first 6 bytes of the _frame is used in the CRC calculation
	_frame[6] = crc16 >> 8; // crc Lo
	_frame[7] = crc16 & 0xFF; // crc Hi
	_swSer.flush();
	sendPacket(8);
	return ((_frame[4] << 8) | _frame[5]) * T1_5; // requested length * T1-5
}


//_frame[0] = MB_FC_READ_REGS;
//_frame[1] = _len - 2;   //byte count
//
//word val;
//word i = 0;
//while (numregs--) {
//	//retrieve the value from the register bank for the current register
//	val = this->Hreg(startreg + i);
//	//write the high byte of the register value
//	_frame[2 + i * 2] = val >> 8;
//	//write the low byte of the register value
//	_frame[3 + i * 2] = val & 0xFF;
//	i++;
//}


Status::RtuError RtuMaster::TransferBack(byte *tcpFrame)
{
	rtuError = getData();
	if (rtuError == Status::ok) // if there's something in the buffer continue
	{
		
		unsigned int len;
		len = 3 + _frame[2]; // MBAP length + UID + Data Length
		tcpFrame[0] = _transactionId >> 8; // transaction Id
		tcpFrame[1] = _transactionId & 0xFF;
		tcpFrame[2] = 0;
		tcpFrame[3] = 0;
		tcpFrame[4] = len >> 8;
		tcpFrame[5] = len & 0xFF;
		tcpFrame[6] = _frame[0]; //UID
		tcpFrame[7] = _frame[1]; //CMD
		tcpFrame[8] = _frame[2]; //data length
		word val;
		word i = 0;
		while (len--) {
			tcpFrame[i+9] = _frame[i+3];
			i++;
		}
		rtuError = Status::ok;
	}
	else
	{
		rtuError = Status::nothing;
	}
	return Error();
}

void RtuMaster::Read(unsigned int address, unsigned int no_of_registers)
{
	_frame[0] = slaveId;
	_frame[1] = READ_HOLDING_REGISTERS;
	_frame[2] = address >> 8; // address Hi
	_frame[3] = address & 0xFF; // address Lo
	_frame[4] = no_of_registers >> 8; // no_of_registers Hi
	_frame[5] = no_of_registers & 0xFF; // no_of_registers Lo

	unsigned int crc16 = calculateCRC(6); // the first 6 bytes of the _frame is used in the CRC calculation
	_frame[6] = crc16 >> 8; // crc Lo
	_frame[7] = crc16 & 0xFF; // crc Hi
	sendPacket(8);
	return;
}

Status::RtuError RtuMaster::CheckResponse(unsigned int no_of_registers, unsigned int* register_array)
{
	unsigned char buffer = getData();

	if (buffer > 0) // if there's something in the buffer continue
	{
		if (_frame[0] == slaveId) // check id returned
		{
			// to indicate an exception response a slave will 'OR' 
			// the requested function with 0x80 
			if ((_frame[1] & 0x80) == 0x80) // exctract 0x80
			{
				// the third byte in the exception response packet is the actual exception
				switch (_frame[2])
				{
					case ILLEGAL_FUNCTION: rtuError = Status::illegal_function; break;
					case ILLEGAL_DATA_ADDRESS: rtuError = Status::illegal_data_address; break;
					case ILLEGAL_DATA_VALUE: rtuError = Status::illegal_data_value; break;
					default: rtuError = Status::misc_exceptions;
				}

			}
			else // the response is valid
			{
				if (_frame[1] == READ_HOLDING_REGISTERS) // check function returned
				{
					check_F3_data(buffer, no_of_registers, register_array);
				}
				else // incorrect function returned
				{
					rtuError = Status::incorrect_function_returned;
				}
			} // check exception response
		}
		else // incorrect id returned
		{
			rtuError = Status::incorrect_id_returned;
		}
	}
	else
	{
		rtuError = Status::nothing;
	}
	return Error();
}

void RtuMaster::check_F3_data(unsigned char buffer, unsigned int no_of_registers, unsigned int* register_array)
{
	unsigned char no_of_bytes = no_of_registers * 2;
	if (_frame[2] == no_of_bytes) // check number of bytes returned
	{
		// combine the crc Low & High bytes
		unsigned int recieved_crc = ((_frame[buffer - 2] << 8) | _frame[buffer - 1]);
		unsigned int calculated_crc = calculateCRC(buffer - 2);

		if (calculated_crc == recieved_crc) // verify checksum
		{
			unsigned char index = 3;
			for (unsigned char i = 0; i < no_of_registers; i++)
			{
				// start at the 4th element in the recieveFrame and combine the Lo byte 
				register_array[i] = (_frame[index] << 8) | _frame[index + 1];
				index += 2;
			}
			rtuError = Status::ok;
		}
		else // checksum failed
		{
			rtuError = Status::checksum_failed;
		}
	}
	else // incorrect number of bytes returned  
	{
		rtuError = Status::incorrect_bytes_returned;
	}
}

unsigned int RtuMaster::calculateCRC(unsigned char bufferSize)
{
	unsigned int temp, temp2, flag;
	temp = 0xFFFF;
	for (unsigned char i = 0; i < bufferSize; i++)
	{
		temp = temp ^ _frame[i];
		for (unsigned char j = 1; j <= 8; j++)
		{
			flag = temp & 0x0001;
			temp >>= 1;
			if (flag)
				temp ^= 0xA001;
		}
	}
	// Reverse byte order. 
	temp2 = temp >> 8;
	temp = (temp << 8) | temp2;
	temp &= 0xFFFF;
	return temp; // crcLo byte is first & crcHi byte is last
}

void RtuMaster::sendPacket(unsigned char bufferSize)
{
	for (unsigned char i = 0; i < bufferSize; i++) {
		_swSer.write(_frame[i]);
	}
	_swSer.flush();
	// allow a _frame delay to indicate end of transmission
	delayMicroseconds(T3_5);
}

// get the serial data from the buffer
Status::RtuError RtuMaster::getData()
{
	unsigned int index = 0;
	unsigned char overflowFlag = 0;

	while (_swSer.available())
	{
		// The maximum number of bytes is limited to the serial buffer size of 128 bytes
		// If more bytes is received than the BUFFER_SIZE the overflow flag will be set and the 
		// serial buffer will be red untill all the data is cleared from the receive buffer,
		// while the slave is still responding.
		if (overflowFlag)
			_swSer.read();
		else
		{
			if (index == RTU_BUFFER_SIZE) {
				overflowFlag = 1;
			}
			_frame[index] = _swSer.read();
			index++;
		}

		delayMicroseconds(T1_5); // inter character time out
	}

	// The minimum buffer size from a slave can be an exception response of 5 bytes 
	// If the buffer was partialy filled clear the buffer.
	// The maximum number of bytes in a modbus packet is 256 bytes.
	// The serial buffer limits this to 128 bytes.
	// If the buffer overflows than clear the buffer and set
	// a packet error.
	if ((index > 0 && index < 5) || overflowFlag)
	{
		index = 0;
		return Status::buffer_errors;
	}

	return index == 0 ? Status::nothing : Status::ok;
}