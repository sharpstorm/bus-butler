#pragma once

#include <Arduino.h>

// Icon glyph (in the Icon* fonts) for a notification icon name. Unknown or
// empty names fall back to the bell.
const char* notifyIconGlyph(const String& name);

// Colour for an accent name, from the active palette (so night mode works).
// Empty picks a colour that suits the icon.
uint16_t notifyAccent(const String& accent, const String& icon);
