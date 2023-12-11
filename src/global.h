#ifndef global_h
#define global_h

#include <JsonConfig.h>

#define POWER_PIN 5

const String SSID = "ssid";
const String PASSWORD = "password";
const String NODE_ID = "node_id";
const String NTP_URL = "ntp_server";
const String TELEGRAM_TOKEN = "telegram_token";
const String TELEGRAM_CHAT_ID = "telegram_chat_id";
const String SLEEP_TIME_SEC = "sleep_time_sec";
const String TIME_ZONE_OFFSET_SEC = "time_zone_offset_sec";

const String *keys[] = {
    &SSID,
    &PASSWORD,
    &NODE_ID,
    &SLEEP_TIME_SEC,
    &NTP_URL,
    &TIME_ZONE_OFFSET_SEC,
    &TELEGRAM_TOKEN,
    &TELEGRAM_CHAT_ID};

JsonConfig config(keys);

// Sometimes there are problems with memory using ArduinoJson
bool is_response_ok(const String &json)
{
    int p1 = json.indexOf(F("\"ok\""));
    int p2 = json.indexOf(F(":"), p1);
    p1 = json.indexOf(F(","), p2);
    String result = json.substring(p2 + 1, p1);
    result.trim();
    PRINTLN(result);
    return result == F("true") || result == F("\"true\"");
}

#endif // global_h