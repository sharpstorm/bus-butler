#include "ScreenReference.h"

// Display constants
Arduino_DataBus* bus =
    new Arduino_ESP32QSPI(LCD_CS, LCD_SCK, LCD_D0, LCD_D1, LCD_D2, LCD_D3);
Arduino_GFX* g = new Arduino_NV3041A(bus, LCD_RST, LCD_ROTATION, LCD_IS_IPS);
Arduino_GFX* gfx = new Arduino_Canvas(LCD_WIDTH, LCD_HEIGHT, g);
