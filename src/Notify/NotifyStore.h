#pragma once

#include <Arduino.h>

#include <vector>

#include "Notification.h"

/*
  Every pushed notification. Written by the web server task, read by the UI
  loop; all access goes through one mutex and readers get copies (the JPEG
  is shared, not copied). getVersion() changes on every change so screens
  know to look again.
*/
class NotifyStore {
 public:
  // Adds or replaces (same id) a notification. Replacing keeps an attached
  // picture unless the new one says otherwise.
  void upsert(Notification n);
  // Takes ownership of data (allocated with malloc/ps_malloc). Returns false
  // if there is no notification with that id.
  bool attachImage(const String& id, uint8_t* data, size_t len);
  bool remove(const String& id);
  void clear();
  void setActionState(const String& id, uint32_t revision, size_t index,
                      ActionState state);

  bool get(const String& id, Notification& out);
  vector<Notification> list();
  // The full-screen notification to show: highest priority, then newest.
  bool topFull(Notification& out);
  // The next toast to show: oldest first.
  bool nextToast(Notification& out);
  // Banners, highest priority then newest first.
  vector<Notification> banners();
  int count(NotifyStyle style);

  // Drops notifications past their deadline and toasts queued too long
  // (except the one on screen).
  void expire(const String& showingToast);

  uint32_t getVersion() { return version; }
  // Changes only when banners change (the Home page redraws on it).
  uint32_t getBannerVersion() { return bannerVersion; }

 private:
  vector<Notification> items;
  uint32_t nextSeq = 1;
  volatile uint32_t version = 1;
  volatile uint32_t bannerVersion = 1;
  void changed(NotifyStyle style) {
    version++;
    if (style == NotifyStyle::Banner) bannerVersion++;
  }

  void lock();
  void unlock();
  void trim(NotifyStyle style, size_t max);
};

extern NotifyStore notifyStore;
