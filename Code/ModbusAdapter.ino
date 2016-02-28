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

#include <DNSServer.h>
#include <ESP8266mDNS.h>
#include <WiFiManager.h>
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include "TcpSlave.h"

#include "Trace.h"

#define MODBUS_BAUD 19200
#define SERIAL_BAUD 115200
#define SLAVE_ID 10

int freq = 5000;
uint8_t gpio_id;
SoftwareSerial _swSer(13, 15, false, 128);
TcpSlave _tcpSlave;
IPAddress apIP(192, 168, 100, 4);
const byte DNS_PORT = 53;
DNSServer dnsServer;
const char *myHostname = "modbusdapter";  //hostname for mDNS. http ://modbusdapter.local

void setupDNS() {
	//DNS
	// if DNSServer is started with "*" for domain name, it will reply with
	// provided IP to all DNS request
	dnsServer.start(DNS_PORT, "*", apIP);

	// Setup MDNS responder
	if (!MDNS.begin(myHostname)) {
		traceln(&Serial, F("Error setting up MDNS responder!"));
	}
	else {
		traceln(&Serial, F("mDNS responder started"));
		// Add service to MDNS-SD
		MDNS.addService("http", "tcp", 80);
		MDNS.addService("ws", "tcp", 81);
	}
}

void setup() {
	pinMode(BUILTIN_LED, OUTPUT);
	digitalWrite(BUILTIN_LED, HIGH);
	Serial.begin(SERIAL_BAUD);
	Serial.setDebugOutput(true);

	setupDNS();

	WiFiManager wifiManager;
	//wifiManager.resetSettings(); //reset settings - for testing
	wifiManager.setTimeout(180);
	if (!wifiManager.autoConnect("AutoConnectAP")) {
		traceln(&Serial, F("failed to connect and hit timeout"));
		delay(3000);
		//reset and try again, or maybe put it to deep sleep
		ESP.reset();
		delay(5000);
	}

	traceln(&Serial, F("WiFi connected"));
	traceln(&Serial, F("IP address: "));
	traceln(&Serial, WiFi.localIP());

	_tcpSlave.config(MODBUS_BAUD, SLAVE_ID, &Serial);
	traceln(&Serial, F("Setup done"));
}

void loop() {
	_tcpSlave.task();

}


