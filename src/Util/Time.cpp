#include "Time.h"

#include <ctime>

#include "../Wifi/WifiManager.h"

using namespace std;

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
