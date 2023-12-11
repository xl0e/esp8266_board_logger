#ifndef DS3231_h
#define DS3231_h

#include "Wire.h"
#include <Arduino.h>

#define DS3231_I2C_ADDR 0x68

const uint8_t daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

struct DateTime
{
	bool ok = false;
	uint8_t second;
	uint8_t minute;
	uint8_t hour;
	uint8_t date;
	uint8_t month;
	uint32_t year;
};

class MicroDS3231
{
public:
	MicroDS3231()
	{
		Wire.begin();
	}

	void setTime(int8_t seconds, int8_t minutes, int8_t hours, int8_t date, int8_t month, int16_t year)
	{

		month = constrain(month, 1, 12);
		date = constrain(date, 1, daysInMonth[month - 1]);
		seconds = constrain(seconds, 0, 59);
		minutes = constrain(minutes, 0, 59);
		hours = constrain(hours, 0, 23);
		year -= 2000;
		Wire.beginTransmission(DS3231_I2C_ADDR);
		Wire.write(0x00);
		uint8_t buff[DATA_LEN];
		buff[0] = encodeRegister(seconds);
		buff[1] = encodeRegister(minutes);
		buff[2] = encodeHours(hours);
		// skip week day
		buff[4] = encodeRegister(date);
		buff[5] = encodeRegister(month);
		buff[6] = encodeHours(year);

		Wire.write(buff, DATA_LEN);
		Wire.endTransmission();
	}

	void setTime(DateTime time)
	{
		setTime(time.second, time.minute, time.hour, time.date, time.month, time.year);
	}

	DateTime getTime(void)
	{
		DateTime now;
		Wire.beginTransmission(DS3231_I2C_ADDR);
		Wire.write(0x0);
		if (Wire.endTransmission() != 0)
			return now;

		Wire.requestFrom(DS3231_I2C_ADDR, DATA_LEN);
		uint8_t buff[DATA_LEN];
		Wire.readBytes(buff, DATA_LEN);

		now.second = decodeRegister(buff[0]);
		now.minute = decodeRegister(buff[1]);
		now.hour = decodeHours(buff[2]);
		// buff[3] Skip week day
		now.date = decodeRegister(buff[4]);
		now.month = decodeRegister(buff[5]);
		now.year = decodeRegister(buff[6]) + 2000;
		now.ok = true;
		return now;
	}

	String getTimeString(DateTime dateTime)
	{
		String str = "";
		if (dateTime.hour < 10)
			str += '0';
		str += dateTime.hour;
		str += ':';
		if (dateTime.minute < 10)
			str += '0';
		str += dateTime.minute;
		str += ':';
		if (dateTime.second < 10)
			str += '0';
		str += dateTime.second;
		return str;
	}

	String getDateString(DateTime dateTime)
	{
		String str = "";
		str += dateTime.year;
		str += '-';
		if (dateTime.month < 10)
			str += '0';
		str += dateTime.month;
		str += '-';
		if (dateTime.date < 10)
			str += '0';
		str += dateTime.date;
		return str;
	}

private:
	const size_t DATA_LEN = 7;

	uint8_t decodeRegister(uint8_t data)
	{
		return ((data >> 4) * 10 + (data & 0x0F));
	}

	uint8_t encodeRegister(int8_t data)
	{
		return (((data / 10) << 4) | (data % 10));
	}

	uint8_t encodeHours(const uint8_t hours)
	{
		if (hours > 19)
			return (0x2 << 4) | (hours % 20);
		else if (hours > 9)
			return (0x1 << 4) | (hours % 10);
		else
			return hours;
	}

	uint8_t decodeHours(const uint8_t data)
	{
		if (data & 0x20)
			return ((data & 0xF) + 20);
		else if (data & 0x10)
			return ((data & 0xF) + 10);
		else
			return (data & 0xF);
	}
};

#endif