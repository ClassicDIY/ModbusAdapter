#pragma once

#include "WiFi.h"
#include "ArduinoJson.h"
#include "IotWebConf.h"

#define STR_LEN 64    // general string buffer size
#define CONFIG_LEN 32 // configuration string buffer size

namespace ModbusAdapter
{
class IOT
{
public:
    IOT();
    void Init();
    void Run();

private:
};
} // namespace ModbusAdapter