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

//#include <stddef.h>
//#include <stdbool.h>
//#include <WiFiUdp.h>
//#include <WiFiServer.h>
//#include <WiFiClienh>
//#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include "TcpSlave.h"
#include "NotInGit.h"

#include "Trace.h"

#define MODBUS_BAUD 19200
#define SERIAL_BAUD 115200
#define SLAVE_ID 10

int freq = 5000;
uint8_t gpio_id;
SoftwareSerial _swSer(13, 15, false, 128);
TcpSlave _tcpSlave;


void setup() {
	pinMode(BUILTIN_LED, OUTPUT);
	digitalWrite(BUILTIN_LED, HIGH);
	Serial.begin(SERIAL_BAUD);
	Serial.setDebugOutput(true);
	_tcpSlave.config(ssid_ap, password, MODBUS_BAUD, SLAVE_ID, &Serial);

	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		traceln(&Serial, F("."));
	}

	traceln(&Serial, F("WiFi connected"));
	traceln(&Serial, F("IP address: "));
	traceln(&Serial, WiFi.localIP());
	traceln(&Serial, F("Setup done"));
}

void loop() {
	_tcpSlave.task();

	//digitalWrite(BUILTIN_LED, LOW);
	//delay(freq);
	//digitalWrite(BUILTIN_LED, HIGH);
	//delay(freq);

}


// Debug helper functions

