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
#include <ESP8266WiFi.h>
#include "RtuMaster.h"
#include "Trace.h"

#define MODBUSIP_PORT 	  502
#define MODBUSIP_MAXFRAME 200
#define MODBUSIP_TIMEOUT   10
#define TCP_BUFFER_SIZE 300


class TcpSlave
{
private:
	byte _MBAP[7];
	byte _sendbuffer[TCP_BUFFER_SIZE];
	WiFiClient _client;
	byte *_frame;
	byte  _len;
	RtuMaster _rtuMaster;
	Print* _stm;
	unsigned int _timeToReceive;

public:
	TcpSlave();
	~TcpSlave();
	void config(long baudRate, unsigned char id, Print* p = NULL);
	void task();
};

