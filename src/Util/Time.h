#pragma once

#include "Arduino.h"

#define NTP_SERVER "time1.google.com"
#define GMT_OFFSET_SECS 28800
#define MINUTE 60
#define HOUR 60 * 60
#define DAY 24 * 60 * 60

class TimeSyncer {
 public:
  TimeSyncer();
  bool syncNtpTime();
};

class TimeUtil {
 public:
  static long iso8601StringToUnixTime(String time);
  static time_t getUnixTime();
  static String getTimeString();
  static String getDateString();

  static TimeSyncer syncer;
  static void printLocalTime();

  static String monthDisplayText[12];
  static String dayOfWeekDisplayText[7];
};
