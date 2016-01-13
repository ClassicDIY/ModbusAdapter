
#include <WiFiUdp.h>
#include <WiFiServer.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266WiFi.h>
#include "TcpSlave.h"

#define BAUD 115200

const char* ssid_ap = "Bangor";
const char* password = "acura22546";

TcpSlave _tcpSlave;

int freq = 5000;


void setup() {
	pinMode(BUILTIN_LED, OUTPUT);
	digitalWrite(BUILTIN_LED, HIGH);

	Serial.begin(BAUD);
	delay(1000);
	Serial.println();
	Serial.println();
	Serial.println("Startup");

	_tcpSlave.config(ssid_ap, password, BAUD);
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());
	Serial.println("Setup done");
}

void loop() {
	
	_tcpSlave.task();
	//if (_rtuMaster.CheckResponse(RECORD_LENGTH, registerArray) == none)
	//{
	//	freq = 5000;
	//	//Serial.print("UUUUUU ");
	//	//printHex(registerArray, 1);
	//	//printHex(registerArray, 32);


	//	//Serial.print("UUUUUU ");
	//	//unsigned int test = 0x04c8;
	//	//printHex(&test, 1);
	//}
	//else
	//{

	//	if (err == nothing)
	//	{
	//		freq = 1000;
	//	}
	//	else
	//	{
	//		freq = 200;
	//	}

	//	//Serial.print("XXXXXX ");
	//	//unsigned int err = _rtuMaster.Error();
	//	//printHex(&err, 1);
	//}
	//digitalWrite(BUILTIN_LED, LOW);
	//delay(freq);
	//digitalWrite(BUILTIN_LED, HIGH);
	//delay(freq);

}


// Debug helper functions

void printHex(unsigned int* ptr, int numberOfWords)
{
	for (int i = 0; i < numberOfWords; i++)
	{
		int w = ptr[i];
		unsigned char l = w & 0x00ff;
		unsigned char h = w >> 8;
		Serial.print(" ");
		p(h);
		p(l);
		Serial.print(" ");
	}
	Serial.println("");
}


void p(byte X) {

	if (X < 10) { Serial.print("0"); }
	Serial.print(X, HEX);

}