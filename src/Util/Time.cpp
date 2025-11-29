#include "Time.h"

#include <ctime>

using namespace std;

#define SGT_CORRECTION (8 * 60 * 60)

long TimeUtil::iso8601StringToUnixTime(String time) {
  // We assume it follows the format YYYY-mm-DDTHH:MM:SS+TZ
  tm tm_struct = {0};
  char* result = strptime(time.c_str(), "%Y-%m-%dT%H:%M:%S%z", &tm_struct);
  tm_struct.tm_isdst = 0;

  time_t t = std::mktime(&tm_struct);
  return t - SGT_CORRECTION;
}
