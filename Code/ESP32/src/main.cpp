#include <Arduino.h>
#include <esp32ModbusRTU.h>
#include "Log.h"
#include "IOT.h"
#include "TcpSlave.h"

using namespace ModbusAdapter;
#define SERIAL_BAUD 115200

IOT _iot = IOT();
TcpSlave _tcpSlave;
boolean _slaveConnected = false;

void WiFiEvent(WiFiEvent_t event)
{
	switch (event)
	{
	case SYSTEM_EVENT_STA_GOT_IP:
		_tcpSlave.init(_iot.BaudRate(), _iot.TCPPort(), _iot.ModbusAddress());
		_slaveConnected = true;
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		_slaveConnected = false;
		_tcpSlave.close();
		break;
	default:
		break;
	}
}

void setup()
{
	Serial.begin(115200);
	while (!Serial)
	{
		; // wait for serial port to connect.
	}
	WiFi.onEvent(WiFiEvent);
	_iot.Init();
	logd("Setup Done");
}

void loop()
{
		
	_iot.Run();
	if (_slaveConnected)
	{
		_tcpSlave.run();
	}
}

