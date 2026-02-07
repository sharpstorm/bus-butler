#include "WeatherRenderer.h"

#include "../../Screen/Colors.h"
#include "../../Screen/Fonts/Icons.h"
#include "../../Screen/Screen.h"
#include "../../Util/Time.h"
#include "./HomescreenConstants.h"

uint16_t WeatherRenderer::getWeatherIconColor(WeatherCondition condition) {
  switch (condition) {
    case WeatherCondition::Fair:
    case WeatherCondition::FairDay:
    case WeatherCondition::FairWarm:
      return NEON_YELLOW;
    case WeatherCondition::FairNight:
      return NEON_PURPLE;
    case WeatherCondition::PartlyCloudy:
    case WeatherCondition::PartlyCloudyDay:
    case WeatherCondition::PartlyCloudyNight:
    case WeatherCondition::Cloudy:
      return GRAY_LIGHT;
    case WeatherCondition::Hazy:
    case WeatherCondition::SlightlyHazy:
    case WeatherCondition::Mist:
    case WeatherCondition::Fog:
      return RGB565(180, 180, 200);
    case WeatherCondition::Windy:
      return NEON_CYAN;
    case WeatherCondition::LightRain:
    case WeatherCondition::ModerateRain:
    case WeatherCondition::PassingShowers:
    case WeatherCondition::LightShowers:
    case WeatherCondition::Showers:
      return NEON_BLUE;
    case WeatherCondition::HeavyRain:
    case WeatherCondition::HeavyShowers:
      return RGB565(0, 80, 255);
    case WeatherCondition::Thunderstorm:
    case WeatherCondition::HeavyThunderstorm:
    case WeatherCondition::HeavyThunderstormWithGustyWinds:
      return NEON_YELLOW;
    default:
      return GRAY_MID;
  }
}

void WeatherRenderer::drawWeatherIcon(int16_t x, int16_t y,
                                      WeatherCondition condition,
                                      uint16_t color, float scale) {
  // Select icon based on condition
  const unsigned char* icon = nullptr;
  int16_t iconW = 0;
  int16_t iconH = 0;

  switch (condition) {
    case WeatherCondition::Fair:
    case WeatherCondition::FairDay:
    case WeatherCondition::FairNight:
    case WeatherCondition::FairWarm:
    case WeatherCondition::PartlyCloudy:
    case WeatherCondition::PartlyCloudyDay:
    case WeatherCondition::PartlyCloudyNight:
      icon = ICON_SUNNY;
      iconW = ICON_SUNNY_WIDTH;
      iconH = ICON_SUNNY_HEIGHT;
      break;
    case WeatherCondition::Cloudy:
    case WeatherCondition::Hazy:
    case WeatherCondition::SlightlyHazy:
    case WeatherCondition::Mist:
    case WeatherCondition::Fog:
    case WeatherCondition::Windy:
      icon = ICON_WINDY;
      iconW = ICON_WINDY_WIDTH;
      iconH = ICON_WINDY_HEIGHT;
      break;
    case WeatherCondition::LightRain:
    case WeatherCondition::ModerateRain:
    case WeatherCondition::HeavyRain:
    case WeatherCondition::PassingShowers:
    case WeatherCondition::LightShowers:
    case WeatherCondition::Showers:
    case WeatherCondition::HeavyShowers:
      icon = ICON_RAIN;
      iconW = ICON_RAIN_WIDTH;
      iconH = ICON_RAIN_HEIGHT;
      break;
    case WeatherCondition::Thunderstorm:
    case WeatherCondition::HeavyThunderstorm:
    case WeatherCondition::HeavyThunderstormWithGustyWinds:
      icon = ICON_THUNDER;
      iconW = ICON_THUNDER_WIDTH;
      iconH = ICON_THUNDER_HEIGHT;
      break;
    default:
      icon = ICON_SUNNY;
      iconW = ICON_SUNNY_WIDTH;
      iconH = ICON_SUNNY_HEIGHT;
      break;
  }

  if (icon != nullptr) {
    // Center icon at given position
    int16_t offsetX = x - iconW / 2;
    int16_t offsetY = y - iconH / 2;
    gfx->drawBitmap(offsetX, offsetY, icon, iconW, iconH, color);
  }
}

void WeatherRenderer::renderWeatherIndicator(WeatherCondition condition,
                                             const String& label,
                                             bool isTwoHour,
                                             const String& temp) {
  int footerY = SCREEN_HEIGHT - FOOTER_HEIGHT;

  // Weather indicator area: between date/time and buttons
  // x: 170 to 380 (before buttons at 394)
  int weatherX = 168;
  int weatherW = 220;

  // Clear weather area
  gfx->fillRect(weatherX, footerY + 4, weatherW, FOOTER_HEIGHT - 4, BG_PANEL);

  // Separator line
  gfx->drawFastVLine(weatherX, footerY + 8, FOOTER_HEIGHT - 16, GRAY_MID);

  // Mode indicator (2HR/24HR) with accent color
  uint16_t accentColor = isTwoHour ? NEON_CYAN : NEON_MAGENTA;
  gfx->setFont(NULL);
  gfx->setTextSize(1);
  gfx->setTextColor(accentColor);
  ScreenRenderer::drawText(isTwoHour ? "2HR" : "24HR", weatherX + 8,
                           footerY + 6);

  // Draw small weather icon (scaled down visually by positioning)
  uint16_t iconColor = getWeatherIconColor(condition);
  // Icon centered at weatherX + 60, vertically centered
  int iconCenterX = weatherX + 60;
  int iconCenterY = footerY + FOOTER_HEIGHT / 2 + 4;
  drawWeatherIcon(iconCenterX, iconCenterY, condition, iconColor, 0.5f);

  // Condition text
  gfx->setTextSize(1);
  gfx->setTextColor(WHITE);
  String shortLabel = label;
  if (shortLabel.length() > 10) {
    shortLabel = shortLabel.substring(0, 9) + "..";
  }
  ScreenRenderer::drawText(shortLabel, weatherX + 95, footerY + 14);

  // Temperature (if provided)
  if (temp.length() > 0) {
    gfx->setTextColor(NEON_ORANGE);
    ScreenRenderer::drawText(temp, weatherX + 95, footerY + 28);
  }

  // Small decorative accent
  gfx->fillRect(weatherX + weatherW - 8, footerY + 10, 3, FOOTER_HEIGHT - 20,
                accentColor);
}

void WeatherRenderer::renderWeatherError(const char* errorText) {
  int footerY = SCREEN_HEIGHT - FOOTER_HEIGHT;

  int weatherX = 168;
  int weatherW = 220;

  gfx->fillRect(weatherX, footerY + 4, weatherW, FOOTER_HEIGHT - 4, BG_PANEL);
  gfx->drawFastVLine(weatherX - 5, footerY + 8, FOOTER_HEIGHT - 16, GRAY_MID);

  // "?" indicator
  gfx->setFont(NULL);
  gfx->setTextSize(2);
  gfx->setTextColor(GRAY_MID);
  ScreenRenderer::drawText("?", weatherX + 20, footerY + 20);

  // Error text
  gfx->setTextSize(1);
  gfx->setTextColor(GRAY_LIGHT);
  ScreenRenderer::drawText(errorText, weatherX + 50, footerY + 22);
  gfx->setTextSize(1);
}

void WeatherRenderer::finalize() { gfx->flush(); }
