#include "NeaWeatherApiTypes.h"

WeatherCondition parseWeatherCondition(const String& condition) {
  String c = condition;
  c.toLowerCase();
  c.trim();

  // Fair conditions
  if (c == "fair" || c == "fair (day)") return WeatherCondition::FairDay;
  if (c == "fair (night)") return WeatherCondition::FairNight;
  if (c == "fair and warm" || c == "warm") return WeatherCondition::FairWarm;

  // Cloudy conditions
  if (c == "partly cloudy") return WeatherCondition::PartlyCloudy;
  if (c == "partly cloudy (day)") return WeatherCondition::PartlyCloudyDay;
  if (c == "partly cloudy (night)") return WeatherCondition::PartlyCloudyNight;
  if (c == "cloudy") return WeatherCondition::Cloudy;

  // Haze/mist/fog
  if (c == "hazy") return WeatherCondition::Hazy;
  if (c == "slightly hazy") return WeatherCondition::SlightlyHazy;
  if (c == "mist") return WeatherCondition::Mist;
  if (c == "fog") return WeatherCondition::Fog;
  if (c == "windy") return WeatherCondition::Windy;

  // Rain conditions
  if (c == "light rain") return WeatherCondition::LightRain;
  if (c == "moderate rain") return WeatherCondition::ModerateRain;
  if (c == "heavy rain") return WeatherCondition::HeavyRain;
  if (c == "passing showers") return WeatherCondition::PassingShowers;
  if (c == "light showers") return WeatherCondition::LightShowers;
  if (c == "showers") return WeatherCondition::Showers;
  if (c == "heavy showers") return WeatherCondition::HeavyShowers;

  // Thunderstorms
  if (c == "thundery showers" || c == "thunderstorm")
    return WeatherCondition::Thunderstorm;
  if (c == "heavy thundery showers" || c == "heavy thunderstorm" ||
      c == "heavy thundery showers with gusty winds")
    return WeatherCondition::HeavyThunderstorm;

  return WeatherCondition::Unknown;
}

String weatherConditionToString(WeatherCondition condition) {
  switch (condition) {
    case WeatherCondition::Fair:
    case WeatherCondition::FairDay:
      return "Fair";
    case WeatherCondition::FairNight:
      return "Fair (Night)";
    case WeatherCondition::FairWarm:
      return "Fair & Warm";
    case WeatherCondition::PartlyCloudy:
    case WeatherCondition::PartlyCloudyDay:
    case WeatherCondition::PartlyCloudyNight:
      return "Partly Cloudy";
    case WeatherCondition::Cloudy:
      return "Cloudy";
    case WeatherCondition::Hazy:
      return "Hazy";
    case WeatherCondition::SlightlyHazy:
      return "Slightly Hazy";
    case WeatherCondition::Windy:
      return "Windy";
    case WeatherCondition::Mist:
      return "Mist";
    case WeatherCondition::Fog:
      return "Fog";
    case WeatherCondition::LightRain:
      return "Light Rain";
    case WeatherCondition::ModerateRain:
      return "Moderate Rain";
    case WeatherCondition::HeavyRain:
      return "Heavy Rain";
    case WeatherCondition::PassingShowers:
      return "Passing Showers";
    case WeatherCondition::LightShowers:
      return "Light Showers";
    case WeatherCondition::Showers:
      return "Showers";
    case WeatherCondition::HeavyShowers:
      return "Heavy Showers";
    case WeatherCondition::Thunderstorm:
      return "Thunderstorm";
    case WeatherCondition::HeavyThunderstorm:
    case WeatherCondition::HeavyThunderstormWithGustyWinds:
      return "Heavy Thunderstorm";
    default:
      return "Unknown";
  }
}

String weatherConditionToShortString(WeatherCondition condition) {
  switch (condition) {
    case WeatherCondition::Fair:
    case WeatherCondition::FairDay:
    case WeatherCondition::FairNight:
    case WeatherCondition::FairWarm:
      return "Fair";
    case WeatherCondition::PartlyCloudy:
    case WeatherCondition::PartlyCloudyDay:
    case WeatherCondition::PartlyCloudyNight:
      return "P.Cloudy";
    case WeatherCondition::Cloudy:
      return "Cloudy";
    case WeatherCondition::Hazy:
    case WeatherCondition::SlightlyHazy:
      return "Hazy";
    case WeatherCondition::Windy:
      return "Windy";
    case WeatherCondition::Mist:
    case WeatherCondition::Fog:
      return "Fog";
    case WeatherCondition::LightRain:
    case WeatherCondition::ModerateRain:
    case WeatherCondition::HeavyRain:
      return "Rain";
    case WeatherCondition::PassingShowers:
    case WeatherCondition::LightShowers:
    case WeatherCondition::Showers:
    case WeatherCondition::HeavyShowers:
      return "Showers";
    case WeatherCondition::Thunderstorm:
    case WeatherCondition::HeavyThunderstorm:
    case WeatherCondition::HeavyThunderstormWithGustyWinds:
      return "T-Storm";
    default:
      return "???";
  }
}