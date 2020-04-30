#include "TcpSlave.h"
#include "Enumerations.h"
#include "Log.h"

namespace ModbusAdapter
{

bool cbResponse(ResultCode res, void *data, uint16_t len)
{
	if (res != EX_SUCCESS)
	{
		Serial.printf_P("Request result: 0x%02X, Mem: %d\n", res, ESP.getFreeHeap());
	}
	else
	{
		//respond to TCP master
		// _client.write((byte*)data, len);
		// _client.flush();
		logi("Responded with %d bytes", len);
		printHexString((char *)data, len);
	}
	return true;
}

WiFiServer _server(MODBUS_DEFAULT_PORT);

TcpSlave::TcpSlave()
{
}
void TcpSlave::init(long baudRate, long tcpPort, uint8_t mosbusAddress)
{
	_rtuMaster.Init(baudRate, mosbusAddress);
	_server.begin(tcpPort);
}

void TcpSlave::close()
{
	_server.close();
}

void TcpSlave::run()
{
	if (!_client)
	{
		_client = _server.available();
		if (_client)
		{
			logd("Client connected!");
		}
	}
	if (_client)
	{
		if (_client.connected())
		{
			if (_client.available())
			{
				int i = 0;
				uint8_t  len;
				while (_client.available())
				{
					_MBAP[i] = _client.read();
					i++;
					if (i == 7)
						break; //MBAP length has 7 bytes size
				}
				if (i != 7)
				{
					logd("Not a MODBUSIP packet i is = %d", i);
					len = 0;
					return; //Not a MODBUSIP packet
				}
				len = _MBAP[4] << 8 | _MBAP[5];
				len--; // Do not count with last byte from MBAP

				if (_MBAP[2] != 0 || _MBAP[3] != 0)
				{
					logw("Not a MODBUSIP packet");
					len = 0;
					return; //Not a MODBUSIP packet
				}
				if (len > MODBUSIP_MAXFRAME)
				{
					logw("Length is over MODBUSIP_MAXFRAME: %d", len);
					len = 0;
					return; //Length is over MODBUSIP_MAXFRAME
				}
				uint8_t *frame = (byte *)malloc(len);
				i = 0;
				while (_client.available())
				{
					frame[i] = _client.read();
					i++;
					if (i == len)
						break;
				}
				if (i != len)
				{
					logd("i != _len");
					len = 0;
					free(frame);
					logw("Not a MODBUSIP packet");
					return; //Not a MODBUSIP packet
				}
				_client.flush();
				logd("TCP _MBAP: ");
				printHexString((char *)_MBAP, 7);
				logd("TCP _frame: ");
				printHexString((char *)frame, len);
				bool res = _rtuMaster.Transfer(frame, cbResponse);
				logd("RTU Transfer: %s", res ? "Ok" : "Failed");
				free(frame);
				len = 0;
			}
			_rtuMaster.run();
		}
		else {
			_client.stop();
		}
	}
}
}