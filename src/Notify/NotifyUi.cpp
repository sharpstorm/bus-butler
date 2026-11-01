#include "NotifyUi.h"

#include "../Display/DisplayPower.h"
#include "../States/AppStateManager.h"
#include "../States/Notify/NotifyState.h"
#include "../Ui/Anim.h"
#include "../Ui/Ui.h"
#include "NotifyLook.h"
#include "NotifyStore.h"

NotifyUi notifyUi;

#define CHECK_INTERVAL_MS 500
#define EXPIRE_INTERVAL_MS 250

// Notifications wait until the device has finished booting / setting up.
bool NotifyUi::canShow() {
  AppState s = appStateManager.getActiveStateId();
  return s != AppState::Init && s != AppState::Boot && s != AppState::Setup;
}

void NotifyUi::loop() {
  uint32_t now = millis();
  if (now - lastExpireMs >= EXPIRE_INTERVAL_MS) {
    lastExpireMs = now;
    notifyStore.expire(toastVisible ? toast.id : String());
  }
  if (!canShow()) {
    if (toastVisible) hideToast();
    return;
  }
  if (notifyStore.getVersion() != seenVersion ||
      now - lastCheckMs >= CHECK_INTERVAL_MS) {
    seenVersion = notifyStore.getVersion();
    lastCheckMs = now;
    reconcileFull();
    reconcileToast();
  }
  tickToast();
}

// ---- Full screen ----------------------------------------------------------------

void NotifyUi::reconcileFull() {
  Notification top;
  bool hasFull = notifyStore.topFull(top);
  bool open = appStateManager.getActiveStateId() == AppState::Notify;

  if (!open) {
    if (hasFull) {
      if (toastVisible) hideToast();  // not over the slide-in; it comes back after
      notifyState.show(top, false);
      appStateManager.openOverlay(AppState::Notify);
    }
    return;
  }

  if (notifyState.isDetail()) {
    if (hasFull) {
      notifyState.show(top, false);  // a full one takes over the details
      return;
    }
    Notification banner;
    if (!notifyStore.get(notifyState.currentId(), banner) ||
        banner.style != NotifyStyle::Banner) {
      appStateManager.closeOverlay();  // the banner was dismissed
    }
    return;
  }

  if (!hasFull) {
    appStateManager.closeOverlay();
  } else if (top.id != notifyState.currentId()) {
    notifyState.show(top, false);
  }
  // Same one: NotifyState picks up its updates itself
}

void NotifyUi::dismissFull(const String& id) {
  notifyStore.remove(id);
  Notification next;
  if (notifyStore.topFull(next)) {
    notifyState.show(next, false);
  } else {
    appStateManager.closeOverlay();
  }
  seenVersion = notifyStore.getVersion();
}

void NotifyUi::closeDetail() {
  if (appStateManager.getActiveStateId() == AppState::Notify) {
    appStateManager.closeOverlay();
  }
}

void NotifyUi::openDetail(const String& bannerId) {
  Notification banner;
  if (!notifyStore.get(bannerId, banner)) return;
  if (toastVisible) hideToast();
  notifyState.show(banner, true);
  appStateManager.openOverlay(AppState::Notify);
}

// ---- Toasts ---------------------------------------------------------------------

void NotifyUi::reconcileToast() {
  // Toasts wait while a full-screen notification is up
  if (appStateManager.getActiveStateId() == AppState::Notify) {
    if (toastVisible) hideToast();
    return;
  }
  if (toastVisible) {
    Notification fresh;
    if (!notifyStore.get(toast.id, fresh) ||
        fresh.style != NotifyStyle::Toast) {
      hideToast();
      toastGapUntil = millis() + TOAST_GAP_MS;
    } else if (fresh.revision != toast.revision) {
      toast = fresh;
      toastShownMs = millis();  // an update gets its full time again
      toastBarW = -1;
      anim::refreshOverlay();
    }
    return;
  }
  if ((int32_t)(millis() - toastGapUntil) < 0) return;
  Notification next;
  if (notifyStore.nextToast(next)) showToast(next);
}

void NotifyUi::showToast(const Notification& n) {
  toast = n;
  toastShownMs = millis();
  toastVisible = true;
  toastBarW = -1;
  if (n.wake) displayPower.wake();
  anim::setOverlay(TOAST_Y, TOAST_H, drawToast);
}

void NotifyUi::hideToast() {
  toastVisible = false;
  anim::clearOverlay();
}

void NotifyUi::tickToast() {
  if (!toastVisible) return;
  uint32_t elapsed = millis() - toastShownMs;
  if (elapsed >= toast.timeoutMs) {
    notifyStore.remove(toast.id);
    hideToast();
    toastGapUntil = millis() + TOAST_GAP_MS;
    lastCheckMs = 0;  // look for the next one soon
    return;
  }
  // Move the time-left bar when it has shrunk by a pixel or more
  // (at most ~12 times a second: each refresh re-sends the toast's rows)
  int16_t barW = (TOAST_W - 24) * (1.0f - elapsed / (float)toast.timeoutMs);
  if (barW != toastBarW && millis() - toastRefreshMs >= 80) {
    toastRefreshMs = millis();
    anim::refreshOverlay();
  }
}

bool NotifyUi::toastContains(int16_t x, int16_t y) {
  return toastVisible && x >= TOAST_X - TOAST_TOUCH_MARGIN &&
         x < TOAST_X + TOAST_W + TOAST_TOUCH_MARGIN &&
         y >= TOAST_Y - TOAST_TOUCH_MARGIN && y < TOAST_Y + TOAST_H + 4;
}

void NotifyUi::dismissToastByUser() {
  if (!toastVisible) return;
  notifyStore.remove(toast.id);
  hideToast();
  toastGapUntil = millis() + TOAST_GAP_MS;
  lastCheckMs = 0;
}

void NotifyUi::drawToast() {
  const Notification& n = notifyUi.toast;
  const int16_t x = TOAST_X, y = TOAST_Y, w = TOAST_W, h = TOAST_H;
  uint16_t accent = notifyAccent(n.accent, n.icon);

  gfx->fillRoundRect(x, y, w, h, 10, BG_PANEL);
  gfx->drawRoundRect(x, y, w, h, 10, accent);
  gfx->drawRoundRect(x + 1, y + 1, w - 2, h - 2, 9, BG_HIGHLIGHT);
  ui::textMid(&IconMd, notifyIconGlyph(n.icon), x + 26, y + h / 2 - 2, accent);

  const int16_t textX = x + 48, textW = w - 48 - 34;
  String title = n.title.length() ? n.title : n.body;
  String body = n.title.length() ? n.body : n.source;
  if (body.length()) {
    ui::text(&FigMed, ui::fit(&FigMed, title, textW), textX, y + 23, TEXT_MAIN);
    ui::text(&FigSm, ui::fit(&FigSm, body, textW), textX, y + 41, GRAY_LIGHT);
  } else {
    ui::text(&FigMed, ui::fit(&FigMed, title, textW), textX, y + 32, TEXT_MAIN);
  }
  ui::textMid(&IconSm, ICON_CLOSE, x + w - 20, y + h / 2 - 2, GRAY_MID);

  // Time left
  uint32_t elapsed = millis() - notifyUi.toastShownMs;
  float left = n.timeoutMs ? 1.0f - min(1.0f, elapsed / (float)n.timeoutMs) : 0;
  int16_t barW = (w - 24) * left;
  notifyUi.toastBarW = barW;
  gfx->fillRect(x + 12, y + h - 6, w - 24, 2, GRAY_DARK);
  gfx->fillRect(x + 12, y + h - 6, barW, 2, accent);
}
