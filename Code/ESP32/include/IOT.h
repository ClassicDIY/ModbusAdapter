#pragma once

#include "WiFi.h"
#include "ArduinoJson.h"
#include <IotWebConf.h>
#include <IotWebConfUsing.h>

#define STR_LEN 64    // general string buffer size
#define PORT_OPTIONS_STR_LEN 8 
#define CONFIG_LEN 32 // configuration string buffer size
#define DEFAULT_PORT "502"
#define DEFAULT_BAUD "19200"
#define DEFAULT_DataBits "8N1"
#define DEFAULT_PARITY "none"
#define DEFAULT_StopBits "1"
#define DEFAULT_MODBUS_ADDRESS "10"

const char BAUD_OPTION[] PROGMEM = "<option value='{v}'>{v}</option>";
const char BAUD_OPTION_SELECTED[] PROGMEM = "<option value='{v}' selected='selected'>{v}</option>";
const char SELECT_BAUD[] PROGMEM = "<div class><label for='baudSelector'>RTU Baud Rate </label><div><select style='padding:5px;font-size:1em;width:95%;' id='baudSelector' name='baudSelector'>{o}</select></div><div class='em'></div></div>\n";

namespace ModbusAdapter
{


class IOT
{
public:
    IOT();
    void Init();
    void Run();
    int BaudRate();
    uint32_t SerialConfig();
    int TCPPort();
    uint8_t ModbusAddress();

private:

};
} // namespace ModbusAdapter