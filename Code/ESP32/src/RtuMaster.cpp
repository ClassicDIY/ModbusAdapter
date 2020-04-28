
#include "RtuMaster.h"
#include "Log.h"

RtuMaster::RtuMaster()
{
	rtuError = Status::ok;
}

void RtuMaster::Init(long baudRate, byte modbusAddress)
{
	Serial1.begin(baudRate, SERIAL_8N1, GPIO_NUM_16, GPIO_NUM_17, false); // Modbus connection
	while (!Serial1)
	{
		; // wait for serial port to connect.
	}
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
	_slaveId = modbusAddress;
}

unsigned int RtuMaster::Transfer(byte MBAP[], byte* v)
{
	_transactionId = (MBAP[0] << 8) | MBAP[1];
	_requestUnitId = MBAP[6];
	_frame[0] = _slaveId;
	for (int i = 0; i < 6; i++) {
		_frame[i+1] = v[i];
	}
	unsigned int crc16 = calculateCRC(6); // the first 6 bytes of the _frame is used in the CRC calculation
	_frame[6] = crc16 >> 8; // crc Lo
	_frame[7] = crc16 & 0xFF; // crc Hi
	Serial1.flush();
	sendPacket(8);
	return ((_frame[4] << 8) | _frame[5]) * T1_5; // requested length * T1-5
}

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
		tcpFrame[6] = _requestUnitId; //UID
		tcpFrame[7] = _frame[1]; //CMD
		tcpFrame[8] = _frame[2]; //data length
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
		Serial1.write(_frame[i]);
	}
	Serial1.flush();
	// allow a _frame delay to indicate end of transmission
	delayMicroseconds(T3_5);
	logd("sendPacket _frame: ");
	printHexString((char *)_frame, bufferSize);
}

// get the serial data from the buffer
Status::RtuError RtuMaster::getData()
{
	unsigned int index = 0;
	unsigned char overflowFlag = 0;
	unsigned int wait = millis() +500;

	while (!Serial1.available())
	{
		if (wait < millis())
		{
			break;
		}
	}
	while (Serial1.available())
	{
		// The maximum number of bytes is limited to the serial buffer size of 128 bytes
		// If more bytes is received than the BUFFER_SIZE the overflow flag will be set and the 
		// serial buffer will be red untill all the data is cleared from the receive buffer,
		// while the slave is still responding.
		if (overflowFlag)
			Serial1.read();
		else
		{
			if (index == RTU_BUFFER_SIZE) {
				overflowFlag = 1;
			}
			_frame[index] = Serial1.read();
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