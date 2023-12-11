#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <WiFiClientSecure.h>
#include <SD.h>

#undef PIN_WIRE_SDA
#undef PIN_WIRE_SCL
#define PIN_WIRE_SDA (2)
#define PIN_WIRE_SCL (0)

#define ENABLE_DEBUG

#ifdef ENABLE_DEBUG

#define PRINT(...) Serial.print(__VA_ARGS__);
#define PRINTLN(...) Serial.println(__VA_ARGS__);
#define PRINTF(f, ...) Serial.printf(f, __VA_ARGS__);

#else
#define PRINT(...) ;
#define PRINTLN(...) ;
#define PRINTF(f, ...) ;
#endif

#include <TelegramClient.h>

#include "global.h"
#include "file_logger.h"
#include "rtc.h"

WiFiClientSecure wifiClient;
TelegramClient telegram(wifiClient);

bool wifi_init(const String &ssid, const String &passphrase)
{
  WiFi.begin(ssid, passphrase);
  uint8_t retry_count = 0;
  PRINTLN("");
  while (WiFi.status() != WL_CONNECTED && retry_count++ < 40) // wait 8 sec
  {
    delay(200);
    PRINT("\r");
    PRINT(retry_count);
    PRINT("  ");
  }
  PRINTLN("");
  return WiFi.status() == WL_CONNECTED;
}

void send_files(File &file)
{
  if (FileUtils::isFile(file))
  {
    String name = file.fullName();
    PRINT("Sending file: ");
    PRINTLN(name);
    String response = telegram.sendFile(config[TELEGRAM_CHAT_ID].toInt(), file);
    if (is_response_ok(response))
    {
      PRINT("File sent successfully. Removing: ");
      PRINTLN(name);
      file.close();
      SD.remove(name);
    }
    else
    {
      PRINTLN("Error to send file:");
      PRINTLN(response);
    }
  }
}

void setup()
{
#ifdef ENABLE_DEBUG
  Serial.begin(115200);
  while (!Serial)
    ;
  Serial.println();
  Serial.println("Serial is ready");
#endif
  PRINTLN("Enable power");
  pinMode(POWER_PIN, OUTPUT);
  digitalWrite(POWER_PIN, HIGH);

  delay(100);
  File file;
  if (SD.begin(15))
  {
    file = SD.open("/node_config.txt", FILE_READ);
  }
  config.init(&file);

  if (FileUtils::hasFiles(LOGS_DIR))
  {
    PRINT("Connecting Wifi...");
    if (wifi_init(config[SSID], config[PASSWORD]))
    {
      PRINTLN();
      PRINT("Local IP:");
      PRINTLN(WiFi.localIP());
      wifiClient.setInsecure();
      telegram.setToken(config[TELEGRAM_TOKEN]);
      FileUtils::iterateRecursive(LOGS_DIR, send_files);

      RTC::updateTime();
    }
  }
  else
  {
    PRINTLN("Nothing to send");
  }

  logData();

  PRINTLN("Disable power");
  digitalWrite(POWER_PIN, LOW);
  PRINT("Go to sleep...");
  ESP.deepSleep(config[SLEEP_TIME_SEC].toInt() * 1e6);
}

void loop()
{
}