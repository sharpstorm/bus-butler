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
