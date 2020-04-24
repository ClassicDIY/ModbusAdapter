#include <Arduino.h>
#include <WiFi.h>
#include <ThreadController.h>
#include <Thread.h>
// #include <esp32ModbusRTU.h>
#include "Log.h"
#include "IOT.h"
#include "TcpSlave.h"

using namespace ModbusAdapter;
#define MODBUS_BAUD 19200
#define SERIAL_BAUD 115200
#define SLAVE_ID 1

const char* ssid = "router";
const char* password = "passowrd";
const char * hostName = "ModbusAdapter";

ThreadController _controller = ThreadController();
Thread *_workerThreadMonitor = new Thread();
// IOT _iot = IOT();
// esp32ModbusRTU modbus(&Serial1, 5); // Serial1 connected to modbus RTU port
TcpSlave _tcpSlave;

// void runMonitor()
// {
// // 	Serial.print("sending Modbus request...\n");
// // 	modbus.readHoldingRegisters(0x01, 4112, 16);
		
// }

// void setupRTU()
// {
// 	Serial1.begin(19200, SERIAL_8N1, GPIO_NUM_16, GPIO_NUM_17, false); // Modbus connection
// 	while (!Serial1)
// 	{
// 		; // wait for serial port to connect.
// 	}
// 	modbus.onData([](uint8_t serverAddress, esp32Modbus::FunctionCode fc, uint8_t *data, size_t length) {
// 		Serial.printf("id 0x%02x fc 0x%02x len %u: 0x", serverAddress, fc, length);
// 		for (size_t i = 0; i < length; ++i)
// 		{
// 			Serial.printf("%02x", data[i]);
// 		}
// 		std::reverse(data, data + 4); // fix endianness
// 		Serial.printf("\nval: %d", *data);
// 		Serial.print("\n\n");
// 	});
// 	  modbus.onError([](esp32Modbus::Error error) {
// 	    Serial.printf("error: 0x%02x\n\n", static_cast<uint8_t>(error));
// 	  });
// 	  modbus.begin();
// }

void setup()
{
	Serial.begin(115200);
	while (!Serial)
	{
		; // wait for serial port to connect.
	}
	WiFi.softAP(hostName);
	WiFi.begin(ssid, password);
	if (WiFi.waitForConnectResult() != WL_CONNECTED) {
		Serial.printf("STA: Failed!\n");
		WiFi.disconnect(false);
		delay(1000);
		WiFi.begin(ssid, password);
	}

	Serial.println("WiFi connected");
	Serial.print("IP address: ");
	Serial.println(WiFi.localIP());
	// // Configure main worker thread
	// _workerThreadMonitor->onRun(runMonitor);
	// _workerThreadMonitor->setInterval(200);
	// _controller.add(_workerThreadMonitor);
	// // _iot.Init();
	// setupRTU();
	logd("Setup init");
	_tcpSlave.init(MODBUS_BAUD, 0x01);
	// server.begin();
	logd("Setup Done");
}

void loop()
{
		
	// _iot.Run();
	if (WiFi.isConnected())
	{
		// _controller.run();
		_tcpSlave.run();
	}
}

