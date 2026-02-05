#include "Time.h"

#include <ctime>

#include "../Wifi/WifiManager.h"

using namespace std;

String TimeUtil::monthDisplayText[] = {"Jan", "Feb", "Mar", "Apr",
                                       "May", "Jun", "Jul", "Aug",
                                       "Sep", "Oct", "Nov", "Dec"};

String TimeUtil::dayOfWeekDisplayText[] = {"Sun", "Mon", "Tue", "Wed",
                                           "Thu", "Fri", "Sat"};

long TimeUtil::iso8601StringToUnixTime(String time) {
  if (time.length() == 0) {
    return 0;
  }

  // We assume it follows the format YYYY-mm-DDTHH:MM:SS+TZ
  tm tm_struct = {0};
  char* result = strptime(time.c_str(), "%Y-%m-%dT%H:%M:%S%z", &tm_struct);
  tm_struct.tm_isdst = 0;

  time_t t = std::mktime(&tm_struct);
  return t;
}

long TimeUtil::getUnixTime() {
  long unixTime;
  if (!time(&unixTime)) {
    return 0;
  }
  return unixTime;
}

String TimeUtil::getTimeString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "";
  }

  // HH:mm tt
  char buf[9];
  int8_t normalizedHour = timeinfo.tm_hour % 12;
  int8_t amPmHour = normalizedHour == 0 ? 12 : normalizedHour;
  String amPm = timeinfo.tm_hour >= 12 ? "pm" : "am";
  sprintf(buf, "%02d:%02d%s", amPmHour, timeinfo.tm_min, amPm);
  buf[8] = 0;
  return String(buf);
}

String TimeUtil::getDateString() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return "";
  }

  // ddd DD MMM
  char buf[11];
  sprintf(buf, "%s %d %s", dayOfWeekDisplayText[timeinfo.tm_wday],
          timeinfo.tm_mday, monthDisplayText[timeinfo.tm_mon]);
  buf[10] = 0;
  return String(buf);
}

TimeSyncer::TimeSyncer() {
  setenv("TZ", "UTC-8", 1);
  tzset();
}

bool TimeSyncer::syncNtpTime() {
  if (!wifiManager.isConnectedToWifi()) {
    return false;
  }
  configTime(GMT_OFFSET_SECS, 0, NTP_SERVER);
  return true;
}

void TimeUtil::printLocalTime() {
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
}

TimeSyncer TimeUtil::syncer;
