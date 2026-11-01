#include "NotifyLook.h"

#include "../Screen/Colors.h"
#include "../Ui/Fonts/UiFonts.h"

struct IconName {
  const char* name;
  const char* glyph;
};

static const IconName ICONS[] = {
    {"bell", ICON_BELL},         {"doorbell", ICON_DOORBELL},
    {"vacuum", ICON_VACUUM},     {"robot", ICON_ROBOT},
    {"camera", ICON_CAMERA},     {"warning", ICON_WARNING},
    {"error", ICON_ERROR},       {"info", ICON_INFO},
    {"check", ICON_CHECK},       {"home", ICON_HOME},
    {"power", ICON_POWER},       {"battery", ICON_BATTERY},
    {"rain", ICON_RAIN},         {"train", ICON_TRAIN},
    {"bus", ICON_BUS},           {"wifi", ICON_WIFI},
    {"refresh", ICON_REFRESH},   {"close", ICON_CLOSE},
    {"clock", ICON_CLOCK},       {"thermo", ICON_THERMO},
    {"timer", ICON_TIMER},       {"settings", ICON_SETTINGS},
};

const char* notifyIconGlyph(const String& name) {
  for (const IconName& icon : ICONS) {
    if (name.equalsIgnoreCase(icon.name)) return icon.glyph;
  }
  return ICON_BELL;
}

uint16_t notifyAccent(const String& accent, const String& icon) {
  String a = accent;
  a.toLowerCase();
  if (a == "cyan") return NEON_CYAN;
  if (a == "magenta") return NEON_MAGENTA;
  if (a == "pink") return NEON_PINK;
  if (a == "purple") return NEON_PURPLE;
  if (a == "blue") return NEON_BLUE;
  if (a == "green") return NEON_GREEN;
  if (a == "yellow") return NEON_YELLOW;
  if (a == "orange") return NEON_ORANGE;
  if (a == "red") return STATUS_RED;

  // By icon
  if (icon == "warning" || icon == "battery") return NEON_YELLOW;
  if (icon == "error") return STATUS_RED;
  if (icon == "check") return NEON_GREEN;
  if (icon == "doorbell" || icon == "camera") return NEON_MAGENTA;
  if (icon == "vacuum" || icon == "robot") return NEON_GREEN;
  return NEON_CYAN;
}
