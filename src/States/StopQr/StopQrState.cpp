#include "StopQrState.h"

#include "../../Screen/Screen.h"
#include "../../Ui/Anim.h"
#include "../../Ui/Ui.h"
#include "../../Util/QrCode.h"
#include "../AppStateManager.h"

QrRequest qrRequest;

#define QR_BOX 248  // white square incl. quiet zone
#define QR_X 12
#define QR_Y ((SCREEN_HEIGHT - QR_BOX) / 2)
#define TEXT_X (QR_X + QR_BOX + 18)
#define TEXT_W (SCREEN_WIDTH - TEXT_X - 12)

void StopQrState::enter() {
  enteredMs = millis();
  lastSecondsLeft = -1;

  ui::background();
  gfx->fillRoundRect(QR_X, QR_Y, QR_BOX, QR_BOX, 8, WHITE);

  int16_t size = 0;
  uint8_t* bitmap = nullptr;
  for (uint8_t version = 3; version <= 6 && bitmap == nullptr; version++) {
    bitmap = QrCode::generate(qrRequest.url.c_str(), version, QR_BOX - 8, &size);
  }
  if (bitmap != nullptr) {
    int16_t offset = (QR_BOX - size) / 2;
    gfx->drawBitmap(QR_X + offset, QR_Y + offset, bitmap, size, size, BLACK,
                    WHITE);
    free(bitmap);
  } else {
    ui::textMid(&FigMed, "Link too long", QR_X + QR_BOX / 2,
                QR_Y + QR_BOX / 2, BLACK);
  }

  // Text column
  ui::section(TEXT_X - 6, QR_Y, SCREEN_WIDTH - TEXT_X, QR_BOX, NEON_CYAN);
  int16_t y = QR_Y + 30;
  for (const String& line : ui::wrap(&FigMed, qrRequest.title, TEXT_W, 3)) {
    ui::text(&FigMed, line, TEXT_X + 4, y, TEXT_MAIN);
    y += 20;
  }
  ui::text(&SgSm, qrRequest.subtitle, TEXT_X + 4, y + 2, NEON_CYAN);
  y += 30;
  for (const String& line : ui::wrap(&FigSm, qrRequest.hint, TEXT_W, 4)) {
    ui::text(&FigSm, line, TEXT_X + 4, y, GRAY_LIGHT);
    y += 16;
  }
  ui::text(&FigSm, "Tap anywhere to close", TEXT_X + 4, QR_Y + QR_BOX - 30,
           TEXT_SOFT);
  renderCountdown(QR_TIMEOUT_MS / 1000, false);
  anim::present();
}

void StopQrState::renderCountdown(int secondsLeft, bool show) {
  int16_t y = QR_Y + QR_BOX - 24;
  gfx->fillRect(TEXT_X, y, TEXT_W, 16, BG_CARD);
  ui::text(&FigSm, "Closes in " + String(secondsLeft) + "s", TEXT_X + 4,
           y + 12, GRAY_MID);
  if (show) anim::presentRows(y, 16);
}

void StopQrState::loop() {
  uint32_t elapsed = millis() - enteredMs;
  if (elapsed >= QR_TIMEOUT_MS) {
    appStateManager.closeOverlay();
    return;
  }
  int secondsLeft = (QR_TIMEOUT_MS - elapsed + 999) / 1000;
  if (secondsLeft != lastSecondsLeft) {
    lastSecondsLeft = secondsLeft;
    renderCountdown(secondsLeft, true);
  }
}

bool StopQrState::onGesture(const Gesture& gesture) {
  appStateManager.closeOverlay();
  return true;
}
