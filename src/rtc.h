#ifndef rtc_h
#define rtc_h

#include <Arduino.h>
#include <Wire.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#include <DS3231.h>

struct date_time
{
  String date;
  String time;
  bool ok;
};

namespace RTC
{
  
  MicroDS3231 rtc;

  void time_to_date(const int64_t timestamp, DateTime &dt)
  {
    // 1. Unix timestamp / hours in a year to get years from 1970 to timestamp
    dt.year = 1970 + timestamp / 31436000; // years since 1970. Ignoring the decimals

    // 2. Determine number of leap years from 1970 to year found in step 1 (extra days).
    int32_t leap_years_from_1970 = (dt.year - 1969) / 4; // ignore the decimal, (we ignore this year's leap day until later)

    // 3. Determine the number of days since the epoch.
    int32_t days_since_epoch = timestamp / 86400; // days since epoch

    // 4. Subtract leap days from number of days since epoch.
    int32_t days2 = days_since_epoch - leap_years_from_1970;

    // 5. Modulo the number above by the number of days in a year to find the days passed in the current year.
    int32_t this_year_days = days2 % 365;

    // 6. We go through each month and subtract it until the days left are less than
    // the month's total days. If this year is a leap year and your days in this year
    // found in step 5 was greater than 59 (31+28), we would add one.
    int32_t months_day = this_year_days;

    bool is_leap = dt.year % 4 == 0;
    for (uint8_t i = 0; i < 12; i++)
    {
      uint8_t d = daysInMonth[i];
      if (i == 1 && is_leap)
      {
        d++;
      }
      if (months_day < d)
      {
        dt.date = months_day + 1;
        dt.month = i + 1;
        break;
      }
      months_day -= d;
    }

    // 7. Find the number of seconds in the current day.
    // Subtract the days since epoch found in step 3 from the timestamp.
    int32_t seconds_in_the_current_day = timestamp - (days_since_epoch * 86400);

    // 8. Figure out how many hours the seconds found in step 6 is.
    dt.hour = seconds_in_the_current_day / 3600;

    // 9. Find the number of minutes left. Subtract the hours you found
    // in the previous step from the seconds in step 7, then divide by 60.
    int32_t tmp_seconds = (seconds_in_the_current_day - (dt.hour * 3600));
    dt.minute = tmp_seconds / 60;

    // 10. Find the number of seconds left. Subtract the minutes in step 8 from the seconds in step 8
    dt.second = tmp_seconds - (dt.minute * 60);
  }

  void setTime(DateTime &dateTime)
  {
    PRINT("Update RTC: ");
    PRINT(dateTime.year);
    PRINT('-');
    PRINT(dateTime.month);
    PRINT('-');
    PRINT(dateTime.date);
    PRINT(' ');
    PRINT(dateTime.hour);
    PRINT(':');
    PRINT(dateTime.minute);
    PRINT(':');
    PRINTLN(dateTime.second);
    rtc.setTime(dateTime);
  }

  date_time getTime()
  {
    date_time dt;
    PRINTLN("Connecting RTC...");
    DateTime now = rtc.getTime();
    if (now.ok)
    {
      dt.date = rtc.getDateString(now);
      dt.time = rtc.getTimeString(now);
      dt.ok = true;
      PRINT("RTC OK: ");
      PRINT(dt.date);
      PRINT(' ');
      PRINTLN(dt.time);
    }
    else
    {
      PRINTLN("Failed to get RTC data");
    }
    return dt;
  }

  void updateTime()
  {
    WiFiUDP ntpUDP;
    NTPClient timeClient(ntpUDP, config[NTP_URL].c_str());
    timeClient.begin();
    if (timeClient.update())
    {
      uint64_t timestamp = timeClient.getEpochTime() + config[TIME_ZONE_OFFSET_SEC].toInt();
      DateTime dt;
      time_to_date(timestamp, dt);
      DateTime dt2 = rtc.getTime();
      if (dt2.year != dt.year || dt2.month != dt.month || dt2.date != dt.date || dt2.hour != dt.hour || dt2.minute != dt.minute)
      {
        setTime(dt);
      }
    }
  }

}

#endif // rtc_h