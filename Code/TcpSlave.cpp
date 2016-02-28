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
#include "Trace.h"
#include "TcpSlave.h"

WiFiServer _server(MODBUSIP_PORT);

TcpSlave::TcpSlave()
{

}

TcpSlave::~TcpSlave()
{
}

void TcpSlave::config(long baudRate, unsigned char id, Print* p) {

	_rtuMaster.Init(baudRate, 0);
	_server.begin();
	_stm = p;
	traceln(_stm, F("TcpSlave::config"));
}

void TcpSlave::task() {
	if (_client == 0) {
		_client = _server.available();
	}
	if (_client != 0) {
		if (_client.connected()) {
			if (_client.available()) {
				
				int i = 0;
				while (_client.available()) {
					_MBAP[i] = _client.read();
					i++;
					if (i == 7) break;  //MBAP length has 7 bytes size
				}
				if (i != 7) {
					trace(_stm, F("i is = "));
					traceln(_stm, i);
					_len = 0;
					return;   //Not a MODBUSIP packet
				}
				_len = _MBAP[4] << 8 | _MBAP[5];
				_len--;  // Do not count with last byte from MBAP

				if (_MBAP[2] != 0 || _MBAP[3] != 0) {
					traceln(_stm, F("Not a MODBUSIP packet"));
					_len = 0;
					return;   //Not a MODBUSIP packet
				}
				if (_len > MODBUSIP_MAXFRAME) {
					trace(_stm, F("Length is over MODBUSIP_MAXFRAME: "));
					traceln(_stm, _len);
					_len = 0;
					return;      //Length is over MODBUSIP_MAXFRAME
				}
				_frame = (byte*)malloc(_len);
				i = 0;
				while (_client.available()) {
					_frame[i] = _client.read();
					i++;
					if (i == _len) break;
				}
				if (i != _len) {
					traceln(_stm, F("i != _len"));
					_len = 0;
					free(_frame);
					return;   //Not a MODBUSIP packet
				}
				trace(_stm, F("Transfer _MBAP: "));
				printHex(_stm, _MBAP, 7);
				trace(_stm, F("Transfer _frame: "));
				printHex(_stm, _frame, _len);
				_timeToReceive = _rtuMaster.Transfer(_MBAP, _frame);
				_timeToReceive += micros();
				free(_frame);
				_len = 0;
			}
			else if (micros() < _timeToReceive) {
				if (_rtuMaster.TransferBack(_sendbuffer) == Status::ok) {
					long unsigned int len = _sendbuffer[8];
					len += 9;
					_client.write((byte*)_sendbuffer, len);
					_client.flush();
					trace(_stm, F("TransferBack _frame: "));
					printHex(_stm, _sendbuffer, len);
				}
			}

		}
		else {
			traceln(_stm, F("Not Connected"));
			_client.stop();
		}
	}
}