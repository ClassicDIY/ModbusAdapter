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
#include "TcpSlave.h"

WiFiServer _server(MODBUSIP_PORT);

TcpSlave::TcpSlave()
{

}


TcpSlave::~TcpSlave()
{
}

void TcpSlave::config(const char* ssid, const char* password, long baudRate) {
	_rtuMaster.Init(baudRate, 0);
	WiFi.begin(ssid, password);
	_server.begin();

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
					//Serial.print("i is = ");
					//Serial.println(i);
					_len = 0;
					return;   //Not a MODBUSIP packet
				}
				_len = _MBAP[4] << 8 | _MBAP[5];
				_len--;  // Do not count with last byte from MBAP

				if (_MBAP[2] != 0 || _MBAP[3] != 0) {
					//Serial.println("Not a MODBUSIP packet");
					_len = 0;
					return;   //Not a MODBUSIP packet
				}
				if (_len > MODBUSIP_MAXFRAME) {
					//Serial.print("Length is over MODBUSIP_MAXFRAME: ");
					//Serial.println(_len);
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
					//Serial.println("i != _len");
					_len = 0;
					free(_frame);
					return;   //Not a MODBUSIP packet
				}
				_rtuMaster.Transfer(_MBAP[6], _frame);
				free(_frame);
				_len = 0;
			}
			else {
				if (_rtuMaster.TransferBack(_sendbuffer) == Status::ok) {
					size_t len = _sendbuffer[8] + 8;
					_client.write((byte*)_sendbuffer, len);
				}
			}

		}
		else {
			//Serial.println("Not Connected");
			_client.stop();
		}
	}
}