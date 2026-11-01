# Bus Butler

Firmware for a bus arrival and weather forecast IoT device that runs on the JC4827W543 board (GUITION 4.3 inch ESP32 LCD module).

This firmware is specifically for the capacitive touch panel variant. It targets the following hardware stack:
- **MCU:** ESP32-S3 dual 240MHz Cores
- **Flash:** 4MB
- **LCD Controller:** `NV3041A`
- **Touch Controller:** `GT911`
- **Screen Resolution:** 480x272 LCD

> [!CAUTION]
> There are no stability guarantees in this firmware, use at your own risk.

# Demo Deployment

![Demo Screen](readme-assets/screen.jpg "Demo Screen")

# External Dependencies

- [LTA Datamall](https://datamall.lta.gov.sg/content/datamall/en.html) for bus arrival data
- NEA datasets on [data.gov.sg](https://data.gov.sg) for weather forecasts
- [bus.gov.sg](https://bus.gov.sg) for web continuity

# Using the device

| Where | Gesture | What it does |
| --- | --- | --- |
| Anywhere | Swipe left / right | Move between Home, Weather, MRT and Status |
| Anywhere | Pull down from the top edge | Controls: brightness, auto-dim, night dim, refresh, web portal QR |
| Home, bus stop header | Swipe left / right | Switch between saved bus stops |
| Home, bus list | Swipe up / down | Next / previous page of services |
| Home, bus list | Tap | Full-screen QR for the stop on bus.gov.sg |
| Home, clock and weather | Tap | Open the Weather page |
| Home, MRT banner | Tap | Open the MRT page (the banner only appears during disruptions) |
| Home, pushed banner | Tap | Open its full text and buttons (see Notifications) |
| Toast | Tap or swipe | Dismiss it |
| Full-screen notification | Tap a button / swipe | Run that button / dismiss it |

Pages slide in and out, the controls panel slides down from the top, the bus list pushes up or down between pages, the backlight fades when dimming or waking, and an arriving bus's dot pulses. The Status page shows the measured animation frame rate.

Settings (bus stops, service order, dimming, weather town and region, WiFi, API key) are in the web portal at `http://busbutler.local` or the device's IP address. The Controls panel can show a QR code for it.

# Notifications

Apps on your network can push notifications to the screen over HTTP. [Sanctuary](../sanctuary) does this for doorbell presses and robot vacuum updates, but anything that can make an HTTP request can use it.

Turn it on in the web portal: **Connection → Notifications → Generate new token**. Every push must send that token as `Authorization: Bearer <token>`. Without a token, pushes are refused. The portal also has buttons that show a sample of each style.

There are three styles:

| Style | Looks like | Goes away | Touch |
| --- | --- | --- | --- |
| `full` | Takes over the screen. Can show a picture (doorbell), text, and up to 3 buttons | After its `timeout` (default 60 s), on `DELETE`, or when you tap **Dismiss** | Buttons call their URL. **Dismiss** or any swipe closes it. Taps elsewhere do nothing, so a stray touch doesn't close a doorbell alert |
| `banner` | In the Home page's alert slot, like the MRT banner | Only on `DELETE` (or its `timeout`, if one is given) | Tap opens the full text and its buttons. Closing that leaves the banner up |
| `toast` | A card floating at the bottom of any page, with a bar showing time left | After its `timeout` (default 6 s, max 30) | Tap it, or flick it away in any direction |

The order of the Home alert slot is: pushed banners with priority 2 or more, then MRT disruptions, then other pushed banners, then heavy rain. With several banners it shows the most important one and "+N". A full-screen notification with `"wake": true` (the default) wakes the screen. It also brightens it past night and idle dimming until it closes. Toasts wait while a full-screen notification is up.

```sh
TOKEN=...   # from the portal
# Toast
curl -X POST http://busbutler.local/api/notify -H "Authorization: Bearer $TOKEN" \
  -H 'Content-Type: application/json' \
  -d '{"id":"laundry","style":"toast","title":"Laundry done","body":"Dryer finished","icon":"check"}'

# Full screen with a picture that follows
curl -X POST http://busbutler.local/api/notify -H "Authorization: Bearer $TOKEN" \
  -H 'Content-Type: application/json' \
  -d '{"id":"door","style":"full","title":"Someone is at the door","image":"pending","priority":3,
       "actions":[{"label":"Open gate","url":"http://192.168.1.10:8123/api/webhook/gate","method":"POST"}]}'
curl -X PUT 'http://busbutler.local/api/notify/image?id=door' -H "Authorization: Bearer $TOKEN" \
  -H 'Content-Type: image/jpeg' --data-binary @door.jpg

# Dismiss (id=* for all)
curl -X DELETE 'http://busbutler.local/api/notify?id=door' -H "Authorization: Bearer $TOKEN"
```

`POST /api/notify` needs `Content-Type: application/json`; without it, the request gets a 404. Fields:

| Field | Meaning |
| --- | --- |
| `id` | Required. 1-32 characters from `A-Z a-z 0-9 . _ - :`. Pushing the same id again updates the notification in place, and its timer starts again |
| `style` | `full`, `banner` or `toast` (default) |
| `title`, `body` | Text. At least one is needed. The fonts are ASCII only: other characters are skipped |
| `icon` | `bell` `doorbell` `vacuum` `robot` `camera` `warning` `error` `info` `check` `home` `power` `battery` `rain` `train` `bus` `wifi` `refresh` `clock` `thermo` `timer` `settings` |
| `accent` | `cyan` `magenta` `pink` `purple` `blue` `green` `yellow` `orange` `red`. If left out, it follows the icon |
| `source` | Small caption, e.g. `Sanctuary` |
| `priority` | 0-3, default 1. A higher full-screen notification jumps the queue |
| `timeout` | Seconds. Full 60, toast 6 (1-30), banner 0 (until dismissed) |
| `wake` | Wake the screen. Default `true` for full, `false` otherwise |
| `image` | Full only: `pending` (picture area says loading; on an update it keeps the current picture with an "Updating" badge), `unavailable`, `none`. If left out, an update keeps the current picture |
| `actions` | Up to 3 `{"label", "url", "method"}` buttons (`http://` only, `GET` or `POST`). The screen calls the URL when tapped and shows ... / Done / Failed. Toasts don't show buttons |

Pictures (`PUT /api/notify/image?id=...`) must be baseline (not progressive) JPEGs of at most 160 KB. They show in a 362×272 area, shrunk by ½, ¼ or ⅛ to fit. Send 362×272 or smaller to avoid cropping. `GET /api/notify` lists what is active.

Notifications are kept in memory only, so a restart clears them. The token stops blind pushes, but the portal itself has no password: anyone who can open the portal can also set a new token.

# Data sources

- LTA DataMall: bus arrivals, bus stop names (BusStops), MRT disruptions (TrainServiceAlerts)
- data.gov.sg (NEA): 2-hour, 24-hour and 4-day forecasts; air temperature, humidity and rainfall from the station nearest the configured town; PSI for the configured region; UV index

# Building

```
cd web-portal && npm ci && npm run build && cd ..
pio run -t upload
```

The firmware uses a custom `partitions.csv`: one 2.5 MB app slot (no OTA) with NVS and SPIFFS at the stock `default.csv` offsets and sizes. The first upload after changing partition tables must be a full `pio run -t upload` so the table is rewritten. Saved settings are kept.

UI fonts are generated into `src/Ui/Fonts/` by `scripts/generate_fonts.py` (Figtree, Space Grotesk and Material Icons, converted the same way as Adafruit's `fontconvert`). See the script header to regenerate them.
