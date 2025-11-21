#pragma once

#include <Arduino_GFX_Library.h>

#define LCD_CS 45
#define LCD_SCK 47
#define LCD_D0 21
#define LCD_D1 48
#define LCD_D2 40
#define LCD_D3 39
#define LCD_BL_PIN 1

#define LCD_RST GFX_NOT_DEFINED
#define LCD_ROTATION 0
#define LCD_IS_IPS true

#define LCD_WIDTH 480
#define LCD_HEIGHT 272

#define SCREEN_WIDTH LCD_WIDTH
#define SCREEN_HEIGHT LCD_HEIGHT
#define SCREEN_X_CENTER (LCD_WIDTH / 2)
#define SCREEN_Y_CENTER (LCD_HEIGHT / 2)

extern Arduino_DataBus* bus;
extern Arduino_GFX* g;
extern Arduino_GFX* gfx;

