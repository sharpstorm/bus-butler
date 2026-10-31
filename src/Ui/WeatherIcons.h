#pragma once

#include "../NeaWeatherApi/NeaWeatherApiTypes.h"
#include "Ui.h"

namespace ui {

inline const char* weatherIcon(WeatherCondition c) {
  switch (c) {
    case WeatherCondition::Fair:
    case WeatherCondition::FairDay:
    case WeatherCondition::FairWarm:
      return ICON_SUN;
    case WeatherCondition::FairNight:
    case WeatherCondition::PartlyCloudyNight:
      return ICON_MOON;
    case WeatherCondition::PartlyCloudy:
    case WeatherCondition::PartlyCloudyDay:
      return ICON_PARTLY;
    case WeatherCondition::Cloudy:
      return ICON_CLOUD;
    case WeatherCondition::Hazy:
    case WeatherCondition::SlightlyHazy:
    case WeatherCondition::Mist:
    case WeatherCondition::Fog:
      return ICON_FOG;
    case WeatherCondition::Windy:
      return ICON_WIND;
    case WeatherCondition::Thunderstorm:
    case WeatherCondition::HeavyThunderstorm:
    case WeatherCondition::HeavyThunderstormWithGustyWinds:
      return ICON_STORM;
    case WeatherCondition::Unknown:
      return ICON_CLOUD;
    default:
      return ICON_RAIN;
  }
}

inline uint16_t weatherColor(WeatherCondition c) {
  switch (c) {
    case WeatherCondition::Fair:
    case WeatherCondition::FairDay:
    case WeatherCondition::FairWarm:
    case WeatherCondition::Thunderstorm:
    case WeatherCondition::HeavyThunderstorm:
    case WeatherCondition::HeavyThunderstormWithGustyWinds:
      return NEON_YELLOW;
    case WeatherCondition::FairNight:
    case WeatherCondition::PartlyCloudyNight:
      return NEON_PURPLE;
    case WeatherCondition::Windy:
      return NEON_CYAN;
    case WeatherCondition::Unknown:
      return GRAY_MID;
    default:
      return isRainyCondition(c) ? NEON_BLUE : GRAY_LIGHT;
  }
}

// "Partly Cloudy" etc. without the (Day)/(Night) suffix, short enough for
// tight spaces.
inline String weatherLabel(WeatherCondition c) {
  switch (c) {
    case WeatherCondition::Fair:
    case WeatherCondition::FairDay:
    case WeatherCondition::FairNight:
      return "Fair";
    case WeatherCondition::FairWarm:
      return "Fair & warm";
    case WeatherCondition::PartlyCloudy:
    case WeatherCondition::PartlyCloudyDay:
    case WeatherCondition::PartlyCloudyNight:
      return "Partly cloudy";
    case WeatherCondition::Cloudy:
      return "Cloudy";
    case WeatherCondition::Hazy:
      return "Hazy";
    case WeatherCondition::SlightlyHazy:
      return "Slightly hazy";
    case WeatherCondition::Windy:
      return "Windy";
    case WeatherCondition::Mist:
      return "Mist";
    case WeatherCondition::Fog:
      return "Fog";
    case WeatherCondition::LightRain:
      return "Light rain";
    case WeatherCondition::ModerateRain:
      return "Rain";
    case WeatherCondition::HeavyRain:
      return "Heavy rain";
    case WeatherCondition::PassingShowers:
      return "Passing showers";
    case WeatherCondition::LightShowers:
      return "Light showers";
    case WeatherCondition::Showers:
      return "Showers";
    case WeatherCondition::HeavyShowers:
      return "Heavy showers";
    case WeatherCondition::Thunderstorm:
      return "Thundery showers";
    case WeatherCondition::HeavyThunderstorm:
    case WeatherCondition::HeavyThunderstormWithGustyWinds:
      return "Heavy storms";
    default:
      return "--";
  }
}

}  // namespace ui
