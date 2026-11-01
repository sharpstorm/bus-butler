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
      "actions": [{"label": "Refresh", "url": "http://...", "method": "POST"}]
    }
  PUT    /api/notify/image?id=door   body: a baseline JPEG, <= 160 KB. Shown
                                     scaled down by 1/2, 1/4 or 1/8 to fit
                                     362x272 (the picture area).
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
