#include "NotifyState.h"

#include "../../Display/DisplayPower.h"
#include "../../Notify/NotifyActions.h"
#include "../../Notify/NotifyLook.h"
#include "../../Notify/NotifyStore.h"
#include "../../Notify/NotifyUi.h"
#include "../../Ui/Anim.h"
#include "../../Ui/Ui.h"

NotifyState notifyState;

// Side panel next to a picture
#define PANEL_X NOTIFY_PIC_W
#define PANEL_W (SCREEN_WIDTH - NOTIFY_PIC_W)
#define PANEL_PAD 8

// Card layout without a picture
#define CARD_X 16
#define CARD_Y 12
#define CARD_W (SCREEN_WIDTH - 2 * CARD_X)
#define CARD_H 200
#define ROW_Y (CARD_Y + CARD_H + 10)
#define ROW_H 40

#define BAR_H 3

void NotifyState::show(const Notification& n, bool asDetail) {
  current = n;
  detail = asDetail;
  openedMs = millis();
  seenVersion = notifyStore.getVersion();
  if (active) {
    applyBoost();
    render();
  }
}

void NotifyState::enter() {
  active = true;
  openedMs = millis();
  applyBoost();
  render();
}

void NotifyState::exit() {
  active = false;
  displayPower.setBoost(false);
  // Free the picture; it is decoded again if the overlay comes back
  picture.reset();
  pictureFrom = nullptr;
  pictureHold.reset();
}

void NotifyState::applyBoost() {
  displayPower.setBoost(!detail && current.wake);
  if (current.wake) displayPower.wake();
}

// Same text, picture and button states: nothing to redraw.
bool NotifyState::sameLook(const Notification& n) {
  if (n.revision != current.revision) return false;
  for (size_t i = 0; i < n.actions.size() && i < current.actions.size(); i++) {
    if (n.actions[i].state != current.actions[i].state) return false;
  }
  return true;
}

void NotifyState::loop() {
  if (notifyStore.getVersion() != seenVersion) {
    seenVersion = notifyStore.getVersion();
    Notification fresh;
    // Removal is NotifyUi's business (it closes or switches the overlay)
    if (notifyStore.get(current.id, fresh) && !sameLook(fresh)) {
      bool onlyButtons = fresh.revision == current.revision;
      current = fresh;
      if (onlyButtons) {
        for (const Button& b : buttons) renderButton(b);
        anim::present();
      } else {
        applyBoost();
        render();
      }
    }
  }

  if (detail && millis() - openedMs > NOTIFY_DETAIL_TIMEOUT_MS) {
    notifyUi.closeDetail();
    return;
  }
  renderBar(true);
}

// ---------------------------------------------------------------------------

void NotifyState::render() {
  buttons.clear();
  lastBarW = -1;
  if (!detail && current.image != NotifyImage::None) {
    renderWithPicture();
  } else {
    renderCard();
  }
  renderBar(false);
  anim::present();
}

void NotifyState::renderPicture() {
  gfx->fillRect(0, 0, NOTIFY_PIC_W, NOTIFY_PIC_H, BG_DARK);
  const int16_t cx = NOTIFY_PIC_W / 2, cy = NOTIFY_PIC_H / 2;

  // Decode when the picture changed (the blob pointer identifies it)
  if (current.jpeg != nullptr && current.jpeg.get() != pictureFrom) {
    pictureHold = current.jpeg;
    pictureFrom = current.jpeg.get();
    pictureError = "";
    uint32_t start = millis();
    if (!decodeJpeg(current.jpeg->data, current.jpeg->len, NOTIFY_PIC_W,
                    NOTIFY_PIC_H, picture, pictureError)) {
      Serial.printf("Notification picture: %s\n", pictureError.c_str());
    } else {
      Serial.printf("Notification picture %dx%d decoded in %lu ms\n", picture.w,
                    picture.h, millis() - start);
    }
  }
  if (current.jpeg == nullptr) {
    picture.reset();
    pictureFrom = nullptr;
    pictureHold.reset();
  }

  if (picture.ok()) {
    gfx->draw16bitRGBBitmap((NOTIFY_PIC_W - picture.w) / 2,
                            (NOTIFY_PIC_H - picture.h) / 2, picture.pixels,
                            picture.w, picture.h);
    if (current.image == NotifyImage::Pending) {
      // An update is on its way; keep showing the last one
      gfx->fillRoundRect(8, 8, 92, 24, 12, BG_PANEL);
      ui::text(&IconSm, ICON_REFRESH, 16, 26, NEON_CYAN);
      ui::text(&FigSm, "Updating", 34, 25, TEXT_MAIN);
    }
    return;
  }

  ui::corners(12, 12, NOTIFY_PIC_W - 24, NOTIFY_PIC_H - 24, GRAY_DARK);
  const char* line = "No picture";
  if (current.image == NotifyImage::Pending) line = "Getting the picture...";
  if (pictureError.length() && current.jpeg != nullptr) line = "Can't show this picture";
  ui::textMid(&IconXl, ICON_CAMERA, cx, cy - 16, GRAY_MID);
  ui::textMid(&FigMed, line, cx, cy + 30, GRAY_LIGHT);
  if (pictureError.length() && current.jpeg != nullptr) {
    ui::textMid(&FigSm, pictureError, cx, cy + 52, GRAY_MID);
  }
}

void NotifyState::renderWithPicture() {
  renderPicture();

  uint16_t accent = notifyAccent(current.accent, current.icon);
  gfx->fillRect(PANEL_X, 0, PANEL_W, SCREEN_HEIGHT, BG_PANEL);
  gfx->fillRect(PANEL_X, 0, 2, SCREEN_HEIGHT, accent);

  const int16_t x = PANEL_X + PANEL_PAD + 2, w = PANEL_W - 2 * PANEL_PAD - 2;
  ui::text(&IconMd, notifyIconGlyph(current.icon), x, 30, accent);
  String age = ui::ago((millis() - current.createdMs) / 1000);
  ui::text(&FigSm, current.source.length() ? ui::fit(&FigSm, current.source, w - 28)
                                           : String(""),
           x + 26, 20, GRAY_LIGHT);
  ui::text(&FigSm, age == "0s" ? String("now") : age + " ago", x + 26, 34,
           TEXT_SOFT);

  // Buttons first, so the text knows where to stop
  layoutColumn(x, w, SCREEN_HEIGHT - 10);
  int16_t textBottom = buttons.empty() ? SCREEN_HEIGHT - 12 : buttons[0].y - 8;
  for (const Button& b : buttons) textBottom = min(textBottom, (int16_t)(b.y - 8));

  int16_t y = 62;
  for (const String& line : ui::wrap(&FigMed, current.title, w, 3)) {
    if (y > textBottom) break;
    ui::text(&FigMed, line, x, y, TEXT_MAIN);
    y += 19;
  }
  y += 4;
  int bodyLines = max(0, (textBottom - y + 4) / 16);
  for (const String& line : ui::wrap(&FigSm, current.body, w, bodyLines)) {
    ui::text(&FigSm, line, x, y, GRAY_LIGHT);
    y += 16;
  }
  for (const Button& b : buttons) renderButton(b);
}

void NotifyState::renderCard() {
  ui::background();
  uint16_t accent = notifyAccent(current.accent, current.icon);
  ui::section(CARD_X, CARD_Y, CARD_W, CARD_H, accent);

  const int16_t textX = CARD_X + 84, textW = CARD_W - 84 - 18;
  ui::textMid(&IconXl, notifyIconGlyph(current.icon), CARD_X + 44,
              CARD_Y + 52, accent);

  int16_t y = CARD_Y + 40;
  vector<String> title = ui::wrap(&SgBig, current.title, textW, 2);
  for (const String& line : title) {
    ui::text(&SgBig, line, textX, y, TEXT_MAIN);
    y += 30;
  }
  if (title.empty()) y = CARD_Y + 30;
  y += title.empty() ? 0 : 2;
  int maxLines = title.size() > 1 ? 4 : 5;
  for (const String& line : ui::wrap(&FigMed, current.body, textW, maxLines)) {
    ui::text(&FigMed, line, textX, y, GRAY_LIGHT);
    y += 21;
  }

  String caption = current.source;
  String age = ui::ago((millis() - current.createdMs) / 1000);
  age = age == "0s" ? String("just now") : age + " ago";
  caption = caption.length() ? caption + "  -  " + age : age;
  if (detail) caption += "  -  stays on Home until cleared";
  ui::text(&FigSm, ui::fit(&FigSm, caption, CARD_W - 36), CARD_X + 18,
           CARD_Y + CARD_H - 14, TEXT_SOFT);

  layoutRow(CARD_X, CARD_W, ROW_Y, ROW_H);
  for (const Button& b : buttons) renderButton(b);
}

// Buttons stacked bottom-up in the side panel: Close last.
void NotifyState::layoutColumn(int16_t x, int16_t w, int16_t bottom) {
  const int16_t h = 30, gap = 6;
  int16_t y = bottom - h;
  buttons.push_back({x, y, w, h, -1});
  for (int i = (int)current.actions.size() - 1; i >= 0; i--) {
    y -= h + gap;
    buttons.push_back({x, y, w, h, i});
  }
}

// Buttons in a row under the card: actions, then Close.
void NotifyState::layoutRow(int16_t x, int16_t w, int16_t y, int16_t h) {
  const int16_t gap = 10;
  int n = current.actions.size() + 1;
  int16_t bw = min<int16_t>(150, (w - gap * (n - 1)) / n);
  int16_t start = x + (w - (bw * n + gap * (n - 1))) / 2;
  for (int i = 0; i < n; i++) {
    int action = i < (int)current.actions.size() ? i : -1;
    buttons.push_back({(int16_t)(start + i * (bw + gap)), y, bw, h, action});
  }
}

void NotifyState::renderButton(const Button& b) {
  uint16_t accent = notifyAccent(current.accent, current.icon);
  bool close = b.action < 0;
  gfx->fillRoundRect(b.x, b.y, b.w, b.h, 8, close ? BG_CARD : BG_HIGHLIGHT);
  gfx->drawRoundRect(b.x, b.y, b.w, b.h, 8, close ? GRAY_MID : accent);

  String label = close ? String(detail ? "Close" : "Dismiss")
                       : current.actions[b.action].label;
  uint16_t color = close ? TEXT_MAIN : accent;
  if (!close) {
    switch (current.actions[b.action].state) {
      case ActionState::Running:
        label = "...";
        color = GRAY_LIGHT;
        break;
      case ActionState::Ok:
        label = String("Done");
        color = NEON_GREEN;
        break;
      case ActionState::Failed:
        label = String("Failed");
        color = STATUS_RED;
        break;
      default:
        break;
    }
  }
  // Same baseline for every label, descenders or not
  ui::text(&FigMed, ui::fit(&FigMed, label, b.w - 12), b.x + b.w / 2,
           b.y + b.h / 2 + 6, color, ui::Center);
}

// ---------------------------------------------------------------------------
// Time left, as a thin bar

float NotifyState::remaining() {
  if (detail) {
    uint32_t elapsed = millis() - openedMs;
    return elapsed >= NOTIFY_DETAIL_TIMEOUT_MS
               ? 0
               : 1.0f - elapsed / (float)NOTIFY_DETAIL_TIMEOUT_MS;
  }
  if (current.timeoutMs == 0 || current.deadlineMs == 0) return -1;
  int32_t left = (int32_t)(current.deadlineMs - millis());
  if (left <= 0) return 0;
  return min(1.0f, left / (float)current.timeoutMs);
}

bool NotifyState::barGeometry(int16_t& x, int16_t& y, int16_t& w) {
  if (!detail && current.image != NotifyImage::None) {
    x = PANEL_X + 2;
    w = PANEL_W - 2;
    y = SCREEN_HEIGHT - BAR_H;
  } else {
    x = CARD_X;
    w = CARD_W;
    y = SCREEN_HEIGHT - BAR_H - 2;
  }
  return true;
}

void NotifyState::renderBar(bool present) {
  float left = remaining();
  if (left < 0) return;
  int16_t x, y, w;
  barGeometry(x, y, w);
  int16_t filled = lroundf(w * left);
  if (filled == lastBarW) return;
  lastBarW = filled;
  uint16_t accent = notifyAccent(current.accent, current.icon);
  gfx->fillRect(x, y, w, BAR_H, GRAY_DARK);
  gfx->fillRect(x, y, filled, BAR_H, detail ? GRAY_MID : accent);
  if (present) anim::presentRows(y, BAR_H);
}

// ---------------------------------------------------------------------------

void NotifyState::dismiss() {
  if (detail) {
    notifyUi.closeDetail();
  } else {
    notifyUi.dismissFull(current.id);
  }
}

bool NotifyState::onGesture(const Gesture& gesture) {
  openedMs = detail ? millis() : openedMs;  // detail: touching keeps it open
  if (gesture.type != GestureType::Tap) {
    dismiss();  // any swipe puts it away
    return true;
  }
  for (const Button& b : buttons) {
    if (!gesture.startsIn(b.x, b.y, b.w, b.h)) continue;
    if (b.action < 0) {
      dismiss();
    } else {
      const NotifyAction& a = current.actions[b.action];
      if (a.state != ActionState::Running) {
        NotifyActions::run(current.id, current.revision, b.action, a.url,
                           a.method);
      }
    }
    return true;
  }
  return true;  // taps elsewhere do nothing (no accidental dismiss)
}
