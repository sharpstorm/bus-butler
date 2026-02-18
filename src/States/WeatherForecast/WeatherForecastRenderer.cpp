#include "WeatherForecastRenderer.h"

#include "../../Screen/Colors.h"
#include "../../Screen/Fonts/Icons.h"
#include "../../Screen/Fonts/RobotoMonoLight32.h"
#include "../../Screen/Screen.h"
#include "../../Util/Time.h"

uint16_t WeatherForecastRenderer::getWeatherIconColor(
    WeatherCondition condition) {
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

void WeatherForecastRenderer::drawLargeWeatherIcon(int16_t x, int16_t y,
                                                   WeatherCondition condition,
                                                   uint16_t color) {
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
    // Draw icon multiple times to create larger appearance (3x3 grid)
    for (int dy = 0; dy < 3; dy++) {
      for (int dx = 0; dx < 3; dx++) {
        int16_t offsetX = x + (dx * iconW) - iconW - (iconW / 2);
        int16_t offsetY = y + (dy * iconH) - iconH - (iconH / 2);
        gfx->drawBitmap(offsetX, offsetY, icon, iconW, iconH, color);
      }
    }
  }
}

void WeatherForecastRenderer::renderHeader(const String& title) {
  // Header bar
  gfx->fillRect(0, 0, SCREEN_WIDTH, 28, BG_PANEL);
  gfx->drawFastHLine(0, 27, SCREEN_WIDTH, NEON_CYAN);
  gfx->drawFastHLine(0, 26, SCREEN_WIDTH, DIM_CYAN);

  // Title
  gfx->setFont(NULL);
  gfx->setTextSize(1);
  gfx->setTextColor(NEON_CYAN);
  ScreenRenderer::drawCenterText(title, SCREEN_WIDTH / 2, 10);
}

void WeatherForecastRenderer::renderNavigationHint() {
  // Bottom hint
  gfx->setFont(NULL);
  gfx->setTextSize(1);
  gfx->setTextColor(GRAY_MID);
  ScreenRenderer::drawCenterText("Swipe up to return", SCREEN_WIDTH / 2,
                                 SCREEN_HEIGHT - 12);
  ScreenRenderer::drawCenterText("Swipe left/right for forecasts",
                                 SCREEN_WIDTH / 2, SCREEN_HEIGHT - 24);
}

void WeatherForecastRenderer::renderTwoHourForecast(
    const String& town, WeatherCondition condition) {
  clearScreen();
  renderHeader("2-HOUR FORECAST");

  // Town name and condition text
  gfx->setFont(&Roboto_Mono_Light_32);
  gfx->setTextSize(1);
  gfx->setTextColor(WHITE);
  ScreenRenderer::drawCenterText(town, SCREEN_WIDTH / 4, 150);

  // Condition text
  uint16_t iconColor = getWeatherIconColor(condition);
  String conditionText = weatherConditionToString(condition);
  gfx->setFont(NULL);
  gfx->setTextSize(2);
  gfx->setTextColor(iconColor);
  ScreenRenderer::drawCenterText(conditionText, SCREEN_WIDTH / 4, 180);

  // Large weather icon in center
  drawLargeWeatherIcon(SCREEN_WIDTH * 3 / 4, 140, condition, iconColor);

  renderDateTime();
  renderNavigationHint();
}

void WeatherForecastRenderer::renderPeriodForecast(const String& periodName,
                                                   WeatherCondition condition,
                                                   const String& description,
                                                   const String& tempRange) {
  clearScreen();
  renderHeader("24-HOUR FORECAST");

  // Period name
  gfx->setFont(&Roboto_Mono_Light_32);
  gfx->setTextSize(1);
  gfx->setTextColor(NEON_MAGENTA);
  ScreenRenderer::drawCenterText(periodName, SCREEN_WIDTH / 4, 140);

  uint16_t iconColor = getWeatherIconColor(condition);

  // Condition text
  String conditionText = weatherConditionToString(condition);
  gfx->setFont(NULL);
  gfx->setTextSize(2);
  gfx->setTextColor(iconColor);
  ScreenRenderer::drawCenterText(conditionText, SCREEN_WIDTH / 4, 170);

  // Temperature
  gfx->setTextSize(2);
  gfx->setTextColor(NEON_ORANGE);
  ScreenRenderer::drawCenterText(tempRange, SCREEN_WIDTH / 4, 200);

  // Large weather icon
  drawLargeWeatherIcon(SCREEN_WIDTH * 3 / 4, 140, condition, iconColor);

  renderDateTime();
  renderNavigationHint();
}

void WeatherForecastRenderer::renderError(const char* errorText) {
  clearScreen();
  renderHeader("WEATHER FORECAST");

  gfx->setFont(NULL);
  gfx->setTextSize(3);
  gfx->setTextColor(GRAY_MID);
  ScreenRenderer::drawCenterText("?", SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2 - 20);

  gfx->setTextSize(2);
  gfx->setTextColor(GRAY_LIGHT);
  ScreenRenderer::drawCenterText(errorText, SCREEN_WIDTH / 2,
                                 SCREEN_HEIGHT / 2 + 20);

  renderDateTime();
  renderNavigationHint();
}

void WeatherForecastRenderer::renderDateTime() {
  // Clear date/time area only
  gfx->fillRect(0, 60, SCREEN_WIDTH / 2, 60, BLACK);

  gfx->setTextSize(1);
  gfx->setFont(&Roboto_Mono_Light_32);
  gfx->setTextColor(WHITE);
  ScreenRenderer::drawCenterText(TimeUtil::getTimeString(), SCREEN_WIDTH / 4,
                                 80);

  gfx->setFont(NULL);
  gfx->setTextSize(2);
  gfx->setTextColor(NEON_CYAN);
  ScreenRenderer::drawCenterText(TimeUtil::getDateString(), SCREEN_WIDTH / 4,
                                 106);
}

void WeatherForecastRenderer::clearScreen() {
  gfx->fillScreen(BG_DARK);

  // Grid pattern for visual interest
  for (int x = 0; x < SCREEN_WIDTH; x += 40) {
    gfx->drawFastVLine(x, 28, SCREEN_HEIGHT - 28, RGB565(12, 12, 18));
  }
  for (int y = 28; y < SCREEN_HEIGHT; y += 40) {
    gfx->drawFastHLine(0, y, SCREEN_WIDTH, RGB565(12, 12, 18));
  }
}

void WeatherForecastRenderer::finalize() { gfx->flush(); }
