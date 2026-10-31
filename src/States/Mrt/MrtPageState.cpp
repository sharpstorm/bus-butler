#include "MrtPageState.h"

#include "../../Data/DataStore.h"
#include "../../Screen/Screen.h"
#include "../../Ui/Anim.h"
#include "../../Ui/Ui.h"
#include "../AppStateManager.h"

#define RENDER_INTERVAL_MS 30000
#define MSG_LINE_H 15

static const char* NORMAL_LINES[] = {"NSL", "EWL", "NEL", "CCL", "DTL",
                                     "TEL", "BPL", "SLRT", "PLRT"};

void MrtPageState::enter() {
  scroll = 0;
  render();
}

void MrtPageState::loop() {
  if (dataStore.getVersion() != lastDataVersion ||
      millis() - lastRenderMs > RENDER_INTERVAL_MS) {
    render();
  }
}

bool MrtPageState::onGesture(const Gesture& g) {
  // Scroll long disruption messages
  if (g.type == GestureType::SwipeUp && scroll < maxScroll) {
    scroll = min(maxScroll, scroll + 4);
    render();
    return true;
  }
  if (g.type == GestureType::SwipeDown && scroll > 0) {
    scroll = max(0, scroll - 4);
    render();
    return true;
  }
  return false;
}

void MrtPageState::render() {
  lastRenderMs = millis();
  lastDataVersion = dataStore.getVersion();

  dataStore.lock();
  const SourceStatus& status = dataStore.trainStatus;
  String subtitle;
  if (status.ok || status.lastSuccessMs > 0) {
    subtitle = "checked " +
               ui::ago((millis() - status.lastSuccessMs) / 1000) + " ago";
  }
  ui::pageHeader("MRT", subtitle, 2, AppStateManager::PAGE_COUNT);

  if (dataStore.trainAlerts == nullptr) {
    ui::section(8, 36, 464, 228, NEON_CYAN);
    String message = status.lastError == -11 ? "Set an LTA API key"
                     : status.attempted      ? "Can't reach LTA right now"
                                             : "Checking train service...";
    ui::textMid(&FigMed, message, 240, 150, GRAY_LIGHT);
  } else if (dataStore.trainAlerts->isDisrupted()) {
    renderDisrupted();
  } else {
    renderNormal();
  }
  dataStore.unlock();
  anim::present();
}

void MrtPageState::renderNormal() {
  maxScroll = 0;
  ui::section(8, 36, 464, 228, NEON_CYAN);
  ui::text(&IconLg, ICON_CHECK, 22, 96, STATUS_GREEN);
  ui::text(&FigMed, "All lines running normally", 70, 72, TEXT_MAIN);
  ui::text(&FigSm, "Checked every 2 minutes. Disruptions show on Home.", 70,
           90, GRAY_LIGHT);

  // Line chips, 3 x 3
  const int16_t chipW = 140, chipH = 34, gap = 8;
  const int16_t startX = 8 + (464 - (chipW * 3 + gap * 2)) / 2;
  for (int i = 0; i < 9; i++) {
    int16_t cx = startX + (i % 3) * (chipW + gap);
    int16_t cy = 116 + (i / 3) * (chipH + gap);
    gfx->fillRoundRect(cx, cy, chipW, chipH, 6, BG_HIGHLIGHT);
    gfx->fillRoundRect(cx + 8, cy + 9, 6, chipH - 18, 3,
                       trainLineColor(NORMAL_LINES[i]));
    ui::text(&SgMed, trainLineShortName(NORMAL_LINES[i]), cx + 22, cy + 22,
             TEXT_MAIN);
    gfx->fillCircle(cx + chipW - 14, cy + chipH / 2, 4, STATUS_GREEN);
  }
}

void MrtPageState::renderDisrupted() {
  const TrainAlerts& alerts = *dataStore.trainAlerts;
  int16_t y = 36;

  // Up to two affected segments as cards
  size_t cards = min<size_t>(2, alerts.segments.size());
  for (size_t i = 0; i < cards; i++) {
    const TrainAlertSegment& seg = alerts.segments[i];
    const int16_t h = 58;
    ui::section(8, y, 464, h, NEON_MAGENTA);
    gfx->fillRoundRect(16, y + 10, 5, h - 20, 2, trainLineColor(seg.line));
    ui::text(&FigMed, trainLineName(seg.line), 30, y + 22, TEXT_MAIN);
    if (seg.direction.length()) {
      String dir = seg.direction.equalsIgnoreCase("both")
                       ? String("Both directions")
                       : "Towards " + seg.direction;
      ui::text(&SgSm, ui::fit(&SgSm, dir, 180), 460, y + 21, GRAY_LIGHT,
               ui::Right);
    }
    ui::text(&SgMed, stationRange(seg.stations), 30, y + 40, NEON_YELLOW);

    String help;
    if (seg.freeBus.length()) help += "Free bus: " + stationRange(seg.freeBus);
    if (seg.freeShuttle.length()) {
      if (help.length()) help += "   ";
      help += "Shuttle: " + stationRange(seg.freeShuttle);
    }
    if (help.length()) {
      ui::text(&SgSm, ui::fit(&SgSm, help, 300), 460, y + 40, TEXT_SOFT,
               ui::Right);
    }
    if (alerts.segments.size() > cards && i == cards - 1) {
      ui::text(&SgSm, "+" + String(alerts.segments.size() - cards) + " more",
               30, y + 53, GRAY_LIGHT);
    }
    y += h + 8;
  }

  // Messages, wrapped, scrollable
  int16_t boxH = 264 - y;
  if (boxH < 40) return;
  ui::section(8, y, 464, boxH, NEON_CYAN);
  vector<String> lines;
  for (const String& msg : alerts.messages) {
    for (const String& line : ui::wrap(&FigSm, msg, 440, 20)) {
      lines.push_back(line);
    }
    lines.push_back("");
  }
  if (lines.empty()) {
    ui::text(&FigSm, "No further details from LTA yet.", 20, y + 22,
             GRAY_LIGHT);
    maxScroll = 0;
    return;
  }
  int visible = (boxH - 14) / MSG_LINE_H;
  maxScroll = max(0, (int)lines.size() - visible);
  scroll = min(scroll, maxScroll);
  for (int i = 0; i < visible && scroll + i < (int)lines.size(); i++) {
    ui::text(&FigSm, lines[scroll + i], 20, y + 18 + i * MSG_LINE_H, TEXT_MAIN);
  }
  if (maxScroll > 0) {
    // Scroll position bar
    int16_t trackH = boxH - 16;
    int16_t barH = max<int16_t>(12, trackH * visible / lines.size());
    int16_t barY = y + 8 + (trackH - barH) * scroll / maxScroll;
    gfx->fillRoundRect(464, barY, 3, barH, 1, GRAY_MID);
  }
}
