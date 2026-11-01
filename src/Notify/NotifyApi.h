#pragma once

#include <ArduinoJson.h>
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>

/*
  Notification push API. Every call needs "Authorization: Bearer <token>",
  the token set in the web portal (Notifications). No token = switched off.

  POST   /api/notify                 JSON, adds or replaces by id:
    {
      "id": "door",                  required, [A-Za-z0-9._:-], <= 32 chars
      "style": "full" | "banner" | "toast",          default "toast"
      "title": "Someone's at the door", "body": "...",
      "icon": "doorbell", "accent": "cyan", "source": "Sanctuary",
      "priority": 0..3,              default 1; higher full-screens jump the queue
      "timeout": seconds,            full 60, toast 6 (max 30), banner 0 = until dismissed
      "wake": true,                  default true for full, false otherwise
      "image": "pending" | "none" | "unavailable",   full only; omitted keeps
                                     the current picture of an update
      "badge": "LIVE", "badgeAccent": "red",  a label on the picture / card
      "actions": [{"label": "Refresh", "url": "http://...", "method": "POST",
                   "accent": "red"}],
      "merge": true,                 only change the fields given (else a
                                     replacement: missing fields reset)
      "timer": "keep"                don't restart the countdown (default
                                     "restart")
    }
  PUT    /api/notify/image?id=door[&timer=keep]
                                     body: a baseline JPEG, <= 160 KB. Shown
                                     scaled down by 1/2, 1/4 or 1/8 to fit
                                     362x272 (the picture area). Send new
                                     ones as often as ~1/s for a moving view:
                                     only the picture is redrawn, buttons
                                     keep their state; timer=keep stops each
                                     one restarting the countdown.
  DELETE /api/notify?id=door         dismiss (id=* clears everything)
  POST   /api/notify/dismiss?id=door same, for clients without DELETE
  GET    /api/notify                 what's active

  Portal (no token, like the rest of the portal):
  GET  /api/config/notify            {"tokenSet": bool, "full": n, ...}
  POST /api/config/notify            {"token": "..."} ("" switches off)
  POST /api/config/notify/generate   makes a random token, returns it once
  POST /api/config/notify/test       {"style": "toast"} shows a sample
*/
namespace NotifyApi {

void registerRoutes(AsyncWebServer& server);

}  // namespace NotifyApi
