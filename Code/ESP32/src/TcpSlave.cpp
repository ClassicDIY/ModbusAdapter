#include "TcpSlave.h"
#include "Log.h"

WiFiServer _server(MODBUSIP_PORT);

TcpSlave::TcpSlave()
{
}
void TcpSlave::init(long baudRate, unsigned char id)
{
	_rtuMaster.Init(baudRate, id);
	_server.begin();
	_server.begin();
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
				// logd("_client.available");
				int i = 0;
				while (_client.available())
				{
					_MBAP[i] = _client.read();
					i++;
					if (i == 7)
						break; //MBAP length has 7 bytes size
				}
				if (i != 7)
				{
					logd("i is = %d", i);
					_len = 0;
					return; //Not a MODBUSIP packet
				}
				_len = _MBAP[4] << 8 | _MBAP[5];
				_len--; // Do not count with last byte from MBAP

				if (_MBAP[2] != 0 || _MBAP[3] != 0)
				{
					logd("Not a MODBUSIP packet");
					_len = 0;
					return; //Not a MODBUSIP packet
				}
				if (_len > MODBUSIP_MAXFRAME)
				{
					logd("Length is over MODBUSIP_MAXFRAME: %d", _len);
					_len = 0;
					return; //Length is over MODBUSIP_MAXFRAME
				}
				_frame = (byte *)malloc(_len);
				i = 0;
				while (_client.available())
				{
					_frame[i] = _client.read();
					i++;
					if (i == _len)
						break;
				}
				if (i != _len)
				{
					logd("i != _len");
					_len = 0;
					free(_frame);
					logd("Not a MODBUSIP packet");
					return; //Not a MODBUSIP packet
				}
				_client.flush();
				// logd("Transfer _MBAP: ");
				// printHexString((char *)_MBAP, 7);
				// logd("Transfer _frame: ");
				// printHexString((char *)_frame, _len);
				int l = (_frame[3] << 8) + _frame[4];
				int d = (_frame[1] << 8) + _frame[2];
				logd("Request %d from %d", l, d);
				_timeToReceive = _rtuMaster.Transfer(_MBAP, _frame);
				_timeToReceive += micros();
				free(_frame);
				_len = 0;
			}
			else if (micros() < _timeToReceive)
			{
				Status::RtuError err = _rtuMaster.TransferBack(_sendbuffer);
				if (err == Status::ok) {
					long unsigned int len = _sendbuffer[8];
					len += 9;
					_client.write((byte*)_sendbuffer, len);
					_client.flush();
					logd("TransferBack _frame: ");
					printHexString((char *)_sendbuffer, len);
				}
				else
				{
					logd("RTU Error: %d", err);
				}
				logd("TransferBack done");
			}
		}
		else {
			_client.stop();
		}
	}
}