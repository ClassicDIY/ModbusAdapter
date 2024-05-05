#include "IOT.h"
#include <EEPROM.h>
#include <IotWebConfESP32HTTPUpdateServer.h>
#include "Log.h"

namespace ModbusAdapter
{

static const char baudRates[][PORT_OPTIONS_STR_LEN] = { "600", "1200", "1800", "2400", "4800", "9600", "14400", "19200", "38400", "57600", "115200", "230400" };
static const char dataBits[][PORT_OPTIONS_STR_LEN] = { "5", "6", "7", "8" };
static const char parity[][PORT_OPTIONS_STR_LEN] = { "none", "even", "odd" };
static const char stopBits[][PORT_OPTIONS_STR_LEN] = { "1", "2" };


DNSServer _dnsServer;
WebServer _webServer(80);
HTTPUpdateServer _httpUpdater;
IotWebConf _iotWebConf(TAG, &_dnsServer, &_webServer, TAG, CONFIG_VERSION);
char _modbusPort[IOTWEBCONF_WORD_LEN];
char _modbusAddress[IOTWEBCONF_WORD_LEN];
char _rtuBaudRate[PORT_OPTIONS_STR_LEN];
char _rtuDataBits[PORT_OPTIONS_STR_LEN];
char _rtuParity[PORT_OPTIONS_STR_LEN];
char _rtuStopBits[PORT_OPTIONS_STR_LEN];

iotwebconf::ParameterGroup RTU_Group = iotwebconf::ParameterGroup("Modbus RTU", "Modbus RTU");
iotwebconf::NumberParameter modbusPortParam = iotwebconf::NumberParameter("TCP Modbus port", "modbusPortParam", _modbusPort, IOTWEBCONF_WORD_LEN, "number", NULL, "502");
iotwebconf::NumberParameter modbusAddressParam = iotwebconf::NumberParameter("Modbus Address", "modbusAddressParam", _modbusAddress, IOTWEBCONF_WORD_LEN, "number", NULL, "10");
IotWebConfSelectParameter rtuBaudRateSelectParam = IotWebConfSelectParameter("BAUD rate", "baudSelector", _rtuBaudRate, PORT_OPTIONS_STR_LEN, (char*)baudRates, (char*)baudRates, sizeof(baudRates) / PORT_OPTIONS_STR_LEN, PORT_OPTIONS_STR_LEN);
IotWebConfSelectParameter rtuDataBitsSelectParam = IotWebConfSelectParameter("Data Bits", "dbSelector", _rtuDataBits, PORT_OPTIONS_STR_LEN, (char*)dataBits, (char*)dataBits, sizeof(dataBits) / PORT_OPTIONS_STR_LEN, PORT_OPTIONS_STR_LEN);
IotWebConfSelectParameter rtuParitySelectParam = IotWebConfSelectParameter("Parity", "paritySelector", _rtuParity, PORT_OPTIONS_STR_LEN, (char*)parity, (char*)parity, sizeof(parity) / PORT_OPTIONS_STR_LEN, PORT_OPTIONS_STR_LEN);
IotWebConfSelectParameter rtuStopBitsSelectParam = IotWebConfSelectParameter("Stop bits", "stopBitsSelector", _rtuStopBits, PORT_OPTIONS_STR_LEN, (char*)stopBits, (char*)stopBits, sizeof(stopBits) / PORT_OPTIONS_STR_LEN, PORT_OPTIONS_STR_LEN);

void WiFiEvent(WiFiEvent_t event)
{
	logd("[WiFi-event] event: %d", event);
	String s;
	JsonDocument doc;
	switch (event)
	{
	case SYSTEM_EVENT_STA_GOT_IP:
		// logd("WiFi connected, IP address: %s", WiFi.localIP().toString().c_str());
		doc["IP"] = WiFi.localIP().toString().c_str();
		doc["ApPassword"] = TAG;
		serializeJson(doc, s);
		s += '\n';
		Serial.printf(s.c_str()); // send json to flash tool
		break;
	case SYSTEM_EVENT_STA_DISCONNECTED:
		logw("WiFi lost connection");
		break;
	default:
		break;
	}
}

IOT::IOT()
{
}

/**
 * Handle web requests to "/" path.
 */
void handleRoot()
{
	// -- Let IotWebConf test and handle captive portal requests.
	if (_iotWebConf.handleCaptivePortal())
	{
		logd("Captive portal");
		// -- Captive portal request were already served.
		return;
	}
	logd("handleSettings");
	String s = "<!DOCTYPE html><html lang=\"en\"><head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1, user-scalable=no\"/>";
	s += "<title>";
	s += _iotWebConf.getThingName();
	s += "</title></head><body>";
	s += _iotWebConf.getThingName();
	s += " (Free Memory: ";
	s += ESP.getFreeHeap();
	s += ")";
	
	s += "<ul>";
	s += "<li>Modbus port: ";
	s += _modbusPort;
	s += "</ul>";

	s += "<ul>";
	s += "<li>Modbus Address: ";
	s += _modbusAddress;
	s += "</ul>";

	s += "<ul>";
	s += "<li>RTU Baud Rate: ";
	s += _rtuBaudRate;
	s += "</ul>";

	s += "<ul>";
	s += "<li>RTU Data Bits: ";
	s += _rtuDataBits;
	s += "</ul>";

		s += "<ul>";
	s += "<li>RTU Parity: ";
	s += _rtuParity;
	s += "</ul>";

		s += "<ul>";
	s += "<li>RTU Stop Bits: ";
	s += _rtuStopBits;
	s += "</ul>";

	s += "Go to <a href='config'>configure page</a> to change values.";
	s += "</body></html>\n";
	_webServer.send(200, "text/html", s);
}

void IOT::Init()
{
	pinMode(FACTORY_RESET_PIN, INPUT_PULLUP);
	_iotWebConf.setStatusPin(WIFI_STATUS_PIN);
	_iotWebConf.setConfigPin(WIFI_AP_PIN);
	if (digitalRead(FACTORY_RESET_PIN) == LOW)
	{
		EEPROM.begin(IOTWEBCONF_CONFIG_START + IOTWEBCONF_CONFIG_VERSION_LENGTH);
		for (byte t = 0; t < IOTWEBCONF_CONFIG_VERSION_LENGTH; t++)
		{
			EEPROM.write(IOTWEBCONF_CONFIG_START + t, 0);
		}
		EEPROM.commit();
		EEPROM.end();
		logw("Factory Reset!");
	}
	WiFi.onEvent(WiFiEvent);
	RTU_Group.addItem(&modbusPortParam);
	RTU_Group.addItem(&modbusAddressParam);
	RTU_Group.addItem(&rtuBaudRateSelectParam);
	RTU_Group.addItem(&rtuDataBitsSelectParam);
	RTU_Group.addItem(&rtuParitySelectParam);
	RTU_Group.addItem(&rtuStopBitsSelectParam);
	_iotWebConf.addParameterGroup(&RTU_Group);
	boolean validConfig = _iotWebConf.init();
	if (!validConfig)
	{
		logw("!invalid configuration!");
		_iotWebConf.resetWifiAuthInfo();
		strcpy(_modbusPort, DEFAULT_PORT);
		strcpy(_modbusAddress, DEFAULT_MODBUS_ADDRESS);
		strcpy(_rtuBaudRate, DEFAULT_BAUD);
		strcpy(_rtuDataBits, DEFAULT_DataBits);
		strcpy(_rtuParity, DEFAULT_PARITY);
		strcpy(_rtuStopBits, DEFAULT_StopBits);
	}
	else
	{
		_iotWebConf.skipApStartup(); // Set WIFI_AP_PIN to gnd to force AP mode
	}
	// Set up required URL handlers on the web server.
	_webServer.on("/", handleRoot);
	_webServer.on("/config", [] { _iotWebConf.handleConfig(); });
	_webServer.onNotFound([]() { _iotWebConf.handleNotFound(); });
}

void IOT::Run()
{
	_iotWebConf.doLoop();
	if (WiFi.isConnected())
	{
		// ToDo
	}
	else
	{
		if (Serial.peek() == '{')
		{
			String s = Serial.readStringUntil('}');
			s += "}";
			JsonDocument doc;
			DeserializationError err = deserializeJson(doc, s);
			if (err)
			{
				loge("deserializeJson() failed: %s", err.c_str());
			}
			else
			{
				if (doc.containsKey("ssid") && doc.containsKey("password"))
				{
					iotwebconf::Parameter *p = _iotWebConf.getWifiSsidParameter();
					strcpy(p->valueBuffer, doc["ssid"]);
					logd("Setting ssid: %s", p->valueBuffer);
					p = _iotWebConf.getWifiPasswordParameter();
					strcpy(p->valueBuffer, doc["password"]);
					logd("Setting password: %s", p->valueBuffer);
					p = _iotWebConf.getApPasswordParameter();
					strcpy(p->valueBuffer, TAG); // reset to default AP password
					_iotWebConf.saveConfig();
					esp_restart(); // force reboot
				}
				else
				{
					logw("Received invalid json: %s", s.c_str());
				}
			}
		}
		else
		{
			Serial.read(); // discard data
		}
	}
}

int IOT::BaudRate()
{
	return atoi(_rtuBaudRate);
}

uint32_t IOT::SerialConfig()
{
	// stops 5
	// parity bit 0 , 1
	// bits  bit 2, 3
	logd("Config %s %s %s", _rtuDataBits, _rtuParity, _rtuStopBits);
	uint32_t config = SERIAL_5N1;
	config |= (atoi(_rtuStopBits) == 2 ? 0x00000030 : 0x00000010);
	switch (atoi(_rtuDataBits)) {
		case 6:
		config |= 0x00000004;
		break;
		case 7:
		config |= 0x00000008;
		break;
		case 8:
		config |= 0x0000000C;
		break;
	}
	if (strcmp(_rtuParity, "even") == 0) {
		config |= 0x00000002;
	} else if (strcmp(_rtuParity, "odd") == 0) {
		config |= 0x00000003;
	}
	return config;
}

uint8_t IOT::ModbusAddress()
{
	return atoi(_modbusAddress);
}

int IOT::TCPPort()
{
	return atoi(_modbusPort);
}
} // namespace ModbusAdapter