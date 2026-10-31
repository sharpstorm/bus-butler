#include "Colors.h"

const Palette DAY_PALETTE = {
    // neon cyan, magenta, pink, purple, blue, green, yellow, orange
    RGB565(0, 255, 255), RGB565(255, 0, 255), RGB565(255, 16, 120),
    RGB565(180, 0, 255), RGB565(0, 120, 255), RGB565(0, 255, 128),
    RGB565(255, 255, 0), RGB565(255, 128, 0),
    // dim cyan, magenta, pink, purple, blue
    RGB565(0, 80, 80), RGB565(80, 0, 80), RGB565(80, 8, 40),
    RGB565(60, 0, 80), RGB565(0, 40, 80),
    // bg dark, panel, card, highlight
    RGB565(8, 8, 12), RGB565(16, 16, 24), RGB565(24, 24, 36),
    RGB565(32, 32, 48),
    // gray dark, mid, light
    RGB565(32, 32, 32), RGB565(64, 64, 64), RGB565(128, 128, 128),
    // status green, yellow, red, arriving
    RGB565(0, 255, 100), RGB565(255, 220, 0), RGB565(255, 60, 60),
    RGB565(255, 0, 128),
    // text main, soft, grid line
    RGB565(255, 255, 255), RGB565(198, 195, 198), RGB565(12, 12, 18),
};

// Warm neon: the day palette's roles with the blue channel removed. Hot
// pink stands in for magenta and orange for cyan (see the N3 mockup).
const Palette NIGHT_PALETTE = {
    RGB565(255, 154, 41), RGB565(255, 44, 107), RGB565(255, 90, 110),
    RGB565(208, 138, 154), RGB565(231, 162, 115), RGB565(173, 227, 99),
    RGB565(255, 211, 66), RGB565(255, 178, 66),
    RGB565(107, 64, 16), RGB565(107, 18, 48), RGB565(80, 20, 30),
    RGB565(70, 40, 48), RGB565(70, 48, 32),
    RGB565(8, 4, 0), RGB565(16, 8, 8), RGB565(24, 16, 16),
    RGB565(41, 24, 24),
    RGB565(40, 30, 28), RGB565(90, 70, 62), RGB565(156, 134, 123),
    RGB565(173, 227, 99), RGB565(255, 195, 66), RGB565(255, 73, 57),
    RGB565(255, 44, 107),
    RGB565(255, 235, 222), RGB565(222, 195, 181), RGB565(16, 8, 8),
};

const Palette* activePalette = &DAY_PALETTE;

bool setNightPalette(bool night) {
  const Palette* next = night ? &NIGHT_PALETTE : &DAY_PALETTE;
  if (next == activePalette) return false;
  activePalette = next;
  return true;
}

bool isNightPalette() { return activePalette == &NIGHT_PALETTE; }
