#include "IOT.h"
#include <EEPROM.h>
#include "Log.h"

namespace ModbusAdapter
{

DNSServer _dnsServer;
WebServer _webServer(80);
HTTPUpdateServer _httpUpdater;
IotWebConf _iotWebConf(TAG, &_dnsServer, &_webServer, TAG, CONFIG_VERSION);
String baudSelectOptionsHtml;
char _modbusPort[IOTWEBCONF_WORD_LEN];
char _rtuBaudRate[IOTWEBCONF_WORD_LEN];
char _modbusAddress[IOTWEBCONF_WORD_LEN];
IotWebConfSeparator seperatorParam = IotWebConfSeparator("Modbus RTU");
IotWebConfParameter modbusPortParam = IotWebConfParameter("TCP Modbus port", "modbusPortParam", _modbusPort, IOTWEBCONF_WORD_LEN, "number", NULL, "502");
IotWebConfParameter modbusAddressParam = IotWebConfParameter("Modbus Address", "modbusAddressParam", _modbusAddress, IOTWEBCONF_WORD_LEN, "number", NULL, "10");
IotWebConfParameter rtuBaudRateSelectParam = IotWebConfParameter(NULL, "baudSelector", _rtuBaudRate, IOTWEBCONF_WORD_LEN, "number", "0", "19200", SELECT_BAUD, true);

void WiFiEvent(WiFiEvent_t event)
{
	logd("[WiFi-event] event: %d", event);
	String s;
	StaticJsonDocument<128> doc;
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
	s += "Go to <a href='config'>configure page</a> to change values.";
	s += "</body></html>\n";
	_webServer.send(200, "text/html", s);
}

boolean formValidator()
{
	boolean valid = true;

	String s;

	s = _webServer.arg("baudSelector");
	logd("baudSelector: %s ", s.c_str());

	strcpy(_rtuBaudRate, s.c_str());
	return valid;
}

void LoadBauds(const char *v)
{
	String option = BAUD_OPTION;
	if (strcmp(_rtuBaudRate, v) == 0)
	{
		option = BAUD_OPTION_SELECTED;
	}
	option.replace("{v}", v);
	baudSelectOptionsHtml += option;
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
	_iotWebConf.setupUpdateServer(&_httpUpdater);
	_iotWebConf.addParameter(&seperatorParam);
	_iotWebConf.addParameter(&modbusPortParam);
	_iotWebConf.addParameter(&modbusAddressParam);
	_iotWebConf.addParameter(&rtuBaudRateSelectParam);
	_iotWebConf.setFormValidator(&formValidator);
	boolean validConfig = _iotWebConf.init();
	if (!validConfig)
	{
		logw("!invalid configuration!");
		_iotWebConf.resetWifiAuthInfo();
		strcpy(_modbusPort, DEFAULT_PORT);
		strcpy(_modbusAddress, DEFAULT_MODBUS_ADDRESS);
		strcpy(_rtuBaudRate, DEFAULT_BAUD);
	}
	else
	{
		_iotWebConf.skipApStartup(); // Set WIFI_AP_PIN to gnd to force AP mode
	}
	// Set up required URL handlers on the web server.
	_webServer.on("/", handleRoot);
	_webServer.on("/config", [] {
		baudSelectOptionsHtml.clear();
		LoadBauds("600");
		LoadBauds("1200");
		LoadBauds("1800");
		LoadBauds("2400");
		LoadBauds("4800");
		LoadBauds("9600");
		LoadBauds("14400");
		LoadBauds("19200");
		LoadBauds("38400");
		LoadBauds("57600");
		LoadBauds("115200");
		LoadBauds("230400");
		String pitem = SELECT_BAUD;
		pitem.replace("{o}", baudSelectOptionsHtml);
		rtuBaudRateSelectParam.customHtml = pitem.c_str();
		_iotWebConf.handleConfig();
	});
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
			StaticJsonDocument<128> doc;
			DeserializationError err = deserializeJson(doc, s);
			if (err)
			{
				loge("deserializeJson() failed: %s", err.c_str());
			}
			else
			{
				if (doc.containsKey("ssid") && doc.containsKey("password"))
				{
					IotWebConfParameter *p = _iotWebConf.getWifiSsidParameter();
					strcpy(p->valueBuffer, doc["ssid"]);
					logd("Setting ssid: %s", p->valueBuffer);
					p = _iotWebConf.getWifiPasswordParameter();
					strcpy(p->valueBuffer, doc["password"]);
					logd("Setting password: %s", p->valueBuffer);
					p = _iotWebConf.getApPasswordParameter();
					strcpy(p->valueBuffer, TAG); // reset to default AP password
					_iotWebConf.configSave();
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

uint8_t IOT::ModbusAddress()
{
	return atoi(_modbusAddress);
}

int IOT::TCPPort()
{
	return atoi(_modbusPort);
}
} // namespace ModbusAdapter