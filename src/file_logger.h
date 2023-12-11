#include <Arduino.h>
#include <Wire.h>
#include <SD.h>
#include <FileUtils.h>

#include "global.h"
#include "rtc.h"
#include "bmx280.h"

const String CURRENT_DATE = "/.currentDate";
const String LOGS_DIR = "/logs";
const String FILE_PREFIX = "/log";
const String FILE_SUFFIX = ".csv";
const String LOG_FILE = FILE_PREFIX + FILE_SUFFIX;

#define BLINK_DURATION 20

File getOrCreateLogFile(bool &createHeaders)
{
  PRINT("Open log file... ");

  if (!SD.exists(LOG_FILE))
  {
    createHeaders = true;
  }
  File logFile = SD.open(LOG_FILE, FILE_WRITE);
  if (!FileUtils::isFile(logFile))
  {
    PRINTLN("Error creating log file");
    return logFile;
  }
  PRINTLN("DONE");
  createHeaders = logFile.size() == 0;
  return logFile;
}

String getCurrentDate(date_time now)
{
  if (!SD.exists(CURRENT_DATE))
  {
    PRINTF("First run, set current date to: %s\n", now.date.c_str());
    File f = SD.open(CURRENT_DATE, FILE_WRITE);
    f.print(now.date);
    f.print('\n');
    f.close();
    return now.date;
  }
  File f = SD.open(CURRENT_DATE, FILE_READ);
  char currentDate[15] = {0};
  f.readBytesUntil('\n', currentDate, 15);
  f.close();
  return String(currentDate);
}

void setCurrentDate(String date)
{
  PRINTF("Set current date to: %s\n", date.c_str());
  File f = SD.open(CURRENT_DATE, FILE_WRITE);
  f.truncate(0);
  f.print(date);
  f.print('\n');
  f.close();
}

void logData()
{
  date_time now = RTC::getTime();
  if (!now.ok)
  {
    PRINTLN(F("ERROR: Can't connect RTC"));
    return;
  }

  String currentDate = getCurrentDate(now);
  if (!currentDate.equals(now.date))
  {
    String backupFile = LOGS_DIR + FILE_PREFIX + "-" + currentDate + FILE_SUFFIX;
    PRINT("Create backup: ");
    PRINTLN(backupFile);
    if (SD.rename(LOG_FILE, backupFile))
    {
      PRINTLN("Failed to move file to directory");
    }
    setCurrentDate(now.date);
  }
  PRINTLN(F("------------------------------------------"));
  PRINTLN(F("Start log cycle..."));
  bmx280_data bmx_data = BMX280::getData();
  if (!bmx_data.valid)
  {
    PRINTLN(F("BMX280 data is not valid..."));
    return;
  }
  bool createHeaders = false;
  File logFile = getOrCreateLogFile(createHeaders);
  if (!logFile)
  {
    PRINTLN(F("ERROR: Can't create file, exiting..."));
    return;
  }
  if (createHeaders)
  {
    PRINTLN(F("Write file headers"));
    logFile.print("Date;Pressure[Pa];Temperature[C];Humidity[%]");
    logFile.print('\n');
  }
  logFile.print(now.date);
  logFile.print(' ');
  logFile.print(now.time);
  logFile.print(';');
  logFile.print(bmx_data.pressure, 0);
  logFile.print(';');
  logFile.print(bmx_data.temperature, 1);
  logFile.print(';');
  logFile.print(bmx_data.humidity, 1);
  logFile.print('\n');
  logFile.flush();
  logFile.close();
  delay(100);
  PRINTLN("Log cycle DONE");
  PRINTLN("------------------------------------------");
}
