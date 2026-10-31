#pragma once

#include <Arduino_GFX_Library.h>

/*
  Colours come from the active palette so the whole UI can switch to the
  warm night palette (no blue light) during the night window. Use these
  names rather than raw RGB565 values; TEXT_MAIN instead of WHITE for text.
  Screens must redraw after setNightPalette() changes the palette.
*/
struct Palette {
  uint16_t neonCyan, neonMagenta, neonPink, neonPurple, neonBlue, neonGreen,
      neonYellow, neonOrange;
  uint16_t dimCyan, dimMagenta, dimPink, dimPurple, dimBlue;
  uint16_t bgDark, bgPanel, bgCard, bgHighlight;
  uint16_t grayDark, grayMid, grayLight;
  uint16_t statusGreen, statusYellow, statusRed, statusArriving;
  uint16_t textMain, textSoft, gridLine;
};

extern const Palette DAY_PALETTE;
extern const Palette NIGHT_PALETTE;
extern const Palette* activePalette;

// Returns true if the palette changed (screens need a redraw).
bool setNightPalette(bool night);
bool isNightPalette();

// Cyberpunk palette (day) / warm neon (night)
#define NEON_CYAN (activePalette->neonCyan)
#define NEON_MAGENTA (activePalette->neonMagenta)
#define NEON_PINK (activePalette->neonPink)
#define NEON_PURPLE (activePalette->neonPurple)
#define NEON_BLUE (activePalette->neonBlue)
#define NEON_GREEN (activePalette->neonGreen)
#define NEON_YELLOW (activePalette->neonYellow)
#define NEON_ORANGE (activePalette->neonOrange)

// Dimmed versions (for glow effects / backgrounds)
#define DIM_CYAN (activePalette->dimCyan)
#define DIM_MAGENTA (activePalette->dimMagenta)
#define DIM_PINK (activePalette->dimPink)
#define DIM_PURPLE (activePalette->dimPurple)
#define DIM_BLUE (activePalette->dimBlue)

// Background colors
#define BG_DARK (activePalette->bgDark)
#define BG_PANEL (activePalette->bgPanel)
#define BG_CARD (activePalette->bgCard)
#define BG_HIGHLIGHT (activePalette->bgHighlight)

// Grays
#define GRAY_DARK (activePalette->grayDark)
#define GRAY_MID (activePalette->grayMid)
#define GRAY_LIGHT (activePalette->grayLight)

// Status colors
#define STATUS_GREEN (activePalette->statusGreen)
#define STATUS_YELLOW (activePalette->statusYellow)
#define STATUS_RED (activePalette->statusRed)
#define STATUS_ARRIVING (activePalette->statusArriving)

// Text
#define TEXT_MAIN (activePalette->textMain)
#define TEXT_SOFT (activePalette->textSoft)  // secondary values
#define GRID_LINE (activePalette->gridLine)
