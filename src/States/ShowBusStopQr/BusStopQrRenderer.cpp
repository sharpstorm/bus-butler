#include "BusStopQrRenderer.h"

#include "../../Screen/Colors.h"
#include "../../Screen/Screen.h"
#include "../../Util/QrCode.h"

#define HEADER_HEIGHT 28

void BusStopQrRenderer::renderLayout(const String& busStopCode) {
  gfx->fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
  renderHeader();

  int16_t final_size = 0;
  String url = "https://bus.gov.sg/stops/" + busStopCode;

  uint8_t* qrBitmap = QrCode::generate(url.c_str(), 5, 200, &final_size);

  if (qrBitmap != nullptr) {
    gfx->drawBitmap(20, HEADER_HEIGHT + 10, qrBitmap, final_size, final_size,
                    BLACK, WHITE);
    free(qrBitmap);
  }

  // Render back button
  gfx->fillRect(BACK_BUTTON_X, BACK_BUTTON_Y, BACK_BUTTON_WIDTH, BUTTON_HEIGHT,
                BG_CARD);
  gfx->drawRect(BACK_BUTTON_X, BACK_BUTTON_Y, BACK_BUTTON_WIDTH, BUTTON_HEIGHT,
                NEON_CYAN);

  // Button label
  gfx->setFont(NULL);
  gfx->setTextSize(2);
  gfx->setTextColor(WHITE);
  ScreenRenderer::drawCenterText("Back",
                                 BACK_BUTTON_X + (BACK_BUTTON_WIDTH / 2),
                                 BACK_BUTTON_Y + (BUTTON_HEIGHT / 2));

  gfx->flush();
}

void BusStopQrRenderer::renderHeader() {
  // Render header
  gfx->fillRect(0, 0, SCREEN_WIDTH, HEADER_HEIGHT, BG_PANEL);
  gfx->drawFastHLine(0, HEADER_HEIGHT - 1, SCREEN_WIDTH, NEON_CYAN);
  gfx->drawFastHLine(0, HEADER_HEIGHT - 2, SCREEN_WIDTH, DIM_CYAN);

  // Title
  gfx->setFont(NULL);
  gfx->setTextSize(1);
  gfx->setTextColor(NEON_CYAN);
  ScreenRenderer::drawCenterText("Continue on Web", SCREEN_WIDTH / 2, 10);
}

void BusStopQrRenderer::clearScreen() {
  gfx->fillRect(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, BLACK);
  gfx->flush();
}

void BusStopQrRenderer::renderCountdown(const int secondsLeft) {
  gfx->fillRect(COUNTDOWN_X, COUNTDOWN_Y, SCREEN_WIDTH - COUNTDOWN_X, 40,
                BLACK);
  String text = "Scan in " + String(secondsLeft) + " seconds";
  gfx->setFont(NULL);
  gfx->setTextSize(1);
  gfx->setTextColor(CYAN);
  ScreenRenderer::drawText(text, COUNTDOWN_X, COUNTDOWN_Y + 20);
  gfx->flush();
}
