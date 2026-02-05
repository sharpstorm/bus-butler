#pragma once

#include <Arduino_GFX_Library.h>

// Cyberpunk color palette - RGB565 format

// Primary neon colors
#define NEON_CYAN RGB565(0, 255, 255)
#define NEON_MAGENTA RGB565(255, 0, 255)
#define NEON_PINK RGB565(255, 16, 120)
#define NEON_PURPLE RGB565(180, 0, 255)
#define NEON_BLUE RGB565(0, 120, 255)
#define NEON_GREEN RGB565(0, 255, 128)
#define NEON_YELLOW RGB565(255, 255, 0)
#define NEON_ORANGE RGB565(255, 128, 0)

// Dimmed versions (for glow effects / backgrounds)
#define DIM_CYAN RGB565(0, 80, 80)
#define DIM_MAGENTA RGB565(80, 0, 80)
#define DIM_PINK RGB565(80, 8, 40)
#define DIM_PURPLE RGB565(60, 0, 80)
#define DIM_BLUE RGB565(0, 40, 80)

// Background colors
#define BG_DARK RGB565(8, 8, 12)
#define BG_PANEL RGB565(16, 16, 24)
#define BG_CARD RGB565(24, 24, 36)
#define BG_HIGHLIGHT RGB565(32, 32, 48)

// Grays
#define GRAY_DARK RGB565(32, 32, 32)
#define GRAY_MID RGB565(64, 64, 64)
#define GRAY_LIGHT RGB565(128, 128, 128)

// Status colors
#define STATUS_GREEN RGB565(0, 255, 100)
#define STATUS_YELLOW RGB565(255, 220, 0)
#define STATUS_RED RGB565(255, 60, 60)
#define STATUS_ARRIVING RGB565(255, 0, 128)