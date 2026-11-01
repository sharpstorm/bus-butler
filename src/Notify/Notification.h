#pragma once

#include <Arduino.h>

#include <memory>
#include <vector>

using namespace std;

/*
  A notification pushed to the device over HTTP (see NotifyApi.h).

  Three styles:
    Full   - takes over the screen until its timeout, a dismiss request, or
             the user closes it.
    Banner - sits in the Home page's alert slot (like the MRT banner) until
             it is dismissed over HTTP.
    Toast  - floats over any page for a few seconds; tap or swipe it away.
*/
enum class NotifyStyle : uint8_t { Full, Banner, Toast };

enum class NotifyImage : uint8_t {
  None,         // no picture area
  Pending,      // picture area with "loading" until PUT /api/notify/image
  Ready,        // a JPEG is attached
  Unavailable,  // picture area saying there's no picture
};

enum class ActionState : uint8_t { Idle, Running, Ok, Failed };

#define NOTIFY_ID_MAX 32
#define NOTIFY_TITLE_MAX 80
#define NOTIFY_BODY_MAX 300
#define NOTIFY_MAX_ACTIONS 3
#define NOTIFY_ACTION_LABEL_MAX 16
#define NOTIFY_URL_MAX 300
#define NOTIFY_MAX_IMAGE_BYTES (160 * 1024)

#define NOTIFY_MAX_FULL 4
#define NOTIFY_MAX_BANNERS 8
#define NOTIFY_MAX_TOASTS 4

// Defaults when the push doesn't say
#define NOTIFY_FULL_TIMEOUT_S 60
#define NOTIFY_TOAST_TIMEOUT_S 6
#define NOTIFY_TOAST_MAX_WAIT_MS (60 * 1000)  // a queued toast older is dropped

struct NotifyAction {
  String label;
  String url;     // http:// only, called by the device when tapped
  String method;  // GET or POST
  String icon;    // optional icon name
  ActionState state = ActionState::Idle;
};

// JPEG bytes in PSRAM, shared between the store and the screen.
struct JpegBlob {
  uint8_t* data = nullptr;
  size_t len = 0;
  ~JpegBlob() { free(data); }
};

struct Notification {
  String id;
  NotifyStyle style = NotifyStyle::Toast;
  String title;
  String body;
  String icon;    // icon name, see notifyIconGlyph()
  String accent;  // colour name, see notifyAccent()
  String source;  // small caption, e.g. "Sanctuary"
  uint8_t priority = 1;      // 0 low .. 3 urgent
  uint32_t timeoutMs = 0;    // 0 = until dismissed
  uint32_t deadlineMs = 0;   // millis() when it goes (0 = never)
  bool wake = false;         // wake the screen (full: also brighten)
  vector<NotifyAction> actions;
  NotifyImage image = NotifyImage::None;
  shared_ptr<JpegBlob> jpeg;  // set when image == Ready (kept while Pending)
  uint32_t createdMs = 0;
  uint32_t seq = 0;       // arrival order
  uint32_t revision = 0;  // changes on every update of this notification
};

const char* notifyStyleName(NotifyStyle style);
bool parseNotifyStyle(const String& name, NotifyStyle& out);
