#include "NotifyStore.h"

#include <algorithm>

static SemaphoreHandle_t storeMutex = nullptr;

const char* notifyStyleName(NotifyStyle style) {
  switch (style) {
    case NotifyStyle::Full:
      return "full";
    case NotifyStyle::Banner:
      return "banner";
    default:
      return "toast";
  }
}

bool parseNotifyStyle(const String& name, NotifyStyle& out) {
  if (name == "full") {
    out = NotifyStyle::Full;
  } else if (name == "banner") {
    out = NotifyStyle::Banner;
  } else if (name == "toast") {
    out = NotifyStyle::Toast;
  } else {
    return false;
  }
  return true;
}

void NotifyStore::lock() {
  if (storeMutex == nullptr) {
    storeMutex = xSemaphoreCreateRecursiveMutex();
  }
  xSemaphoreTakeRecursive(storeMutex, portMAX_DELAY);
}

void NotifyStore::unlock() { xSemaphoreGiveRecursive(storeMutex); }

void NotifyStore::upsert(Notification n, bool keepTimer) {
  lock();
  uint32_t now = millis();
  n.deadlineMs = 0;
  // Toast timers start when the toast is shown (NotifyUi), not on arrival.
  if (n.timeoutMs > 0 && n.style != NotifyStyle::Toast) {
    n.deadlineMs = now + n.timeoutMs;
    if (n.deadlineMs == 0) n.deadlineMs = 1;
  }
  bool replaced = false;
  for (Notification& old : items) {
    if (old.id != n.id) continue;
    // Keep the picture while a new one is on its way
    if (n.image == NotifyImage::Pending && old.jpeg != nullptr) {
      n.jpeg = old.jpeg;
    }
    n.createdMs = old.createdMs;
    n.seq = old.style == n.style ? old.seq : nextSeq++;
    n.revision = old.revision + 1;
    n.imageRevision = old.imageRevision + (n.jpeg != old.jpeg ? 1 : 0);
    if (keepTimer && old.style == n.style) n.deadlineMs = old.deadlineMs;
    if (old.style == NotifyStyle::Banner) bannerVersion++;
    old = n;
    replaced = true;
    break;
  }
  if (!replaced) {
    n.createdMs = now;
    n.seq = nextSeq++;
    n.revision = 1;
    items.push_back(n);
  }
  NotifyStyle style = n.style;
  trim(style, style == NotifyStyle::Full     ? NOTIFY_MAX_FULL
              : style == NotifyStyle::Banner ? NOTIFY_MAX_BANNERS
                                             : NOTIFY_MAX_TOASTS);
  changed(style);
  unlock();
}

// Drops the lowest-priority, oldest notifications of a style beyond max.
void NotifyStore::trim(NotifyStyle style, size_t max) {
  while ((size_t)count(style) > max) {
    int victim = -1;
    for (size_t i = 0; i < items.size(); i++) {
      if (items[i].style != style) continue;
      if (victim < 0 || items[i].priority < items[victim].priority ||
          (items[i].priority == items[victim].priority &&
           items[i].seq < items[victim].seq)) {
        victim = i;
      }
    }
    if (victim < 0) return;
    items.erase(items.begin() + victim);
  }
}

bool NotifyStore::attachImage(const String& id, uint8_t* data, size_t len,
                              bool keepTimer) {
  shared_ptr<JpegBlob> blob = make_shared<JpegBlob>();
  blob->data = data;
  blob->len = len;
  lock();
  bool found = false;
  for (Notification& n : items) {
    if (n.id != id) continue;
    n.jpeg = blob;
    bool wasReady = n.image == NotifyImage::Ready;
    n.image = NotifyImage::Ready;
    // Only the picture changed, unless the "loading" state ends
    if (wasReady) {
      n.imageRevision++;
    } else {
      n.revision++;
    }
    // A picture arriving late still gets its full time on screen.
    if (!keepTimer && n.timeoutMs > 0 && n.style != NotifyStyle::Toast) {
      n.deadlineMs = millis() + n.timeoutMs;
      if (n.deadlineMs == 0) n.deadlineMs = 1;
    }
    found = true;
    changed(n.style);
    break;
  }
  unlock();
  // Not found: blob frees data when it goes out of scope
  return found;
}

bool NotifyStore::remove(const String& id) {
  lock();
  bool found = false;
  for (size_t i = 0; i < items.size(); i++) {
    if (items[i].id == id) {
      changed(items[i].style);
      items.erase(items.begin() + i);
      found = true;
      break;
    }
  }
  unlock();
  return found;
}

void NotifyStore::clear() {
  lock();
  items.clear();
  version++;
  bannerVersion++;
  unlock();
}

void NotifyStore::setActionState(const String& id, uint32_t revision,
                                 size_t index, ActionState state) {
  lock();
  for (Notification& n : items) {
    if (n.id == id && n.revision == revision && index < n.actions.size()) {
      n.actions[index].state = state;
      changed(n.style);
      break;
    }
  }
  unlock();
}

bool NotifyStore::get(const String& id, Notification& out) {
  lock();
  bool found = false;
  for (const Notification& n : items) {
    if (n.id == id) {
      out = n;
      found = true;
      break;
    }
  }
  unlock();
  return found;
}

vector<Notification> NotifyStore::list() {
  lock();
  vector<Notification> copy = items;
  unlock();
  return copy;
}

bool NotifyStore::topFull(Notification& out) {
  lock();
  const Notification* best = nullptr;
  for (const Notification& n : items) {
    if (n.style != NotifyStyle::Full) continue;
    if (best == nullptr || n.priority > best->priority ||
        (n.priority == best->priority && n.seq > best->seq)) {
      best = &n;
    }
  }
  if (best != nullptr) out = *best;
  unlock();
  return best != nullptr;
}

bool NotifyStore::nextToast(Notification& out) {
  lock();
  const Notification* first = nullptr;
  for (const Notification& n : items) {
    if (n.style != NotifyStyle::Toast) continue;
    if (first == nullptr || n.seq < first->seq) first = &n;
  }
  if (first != nullptr) out = *first;
  unlock();
  return first != nullptr;
}

vector<Notification> NotifyStore::banners() {
  lock();
  vector<Notification> out;
  for (const Notification& n : items) {
    if (n.style == NotifyStyle::Banner) out.push_back(n);
  }
  unlock();
  sort(out.begin(), out.end(), [](const Notification& a, const Notification& b) {
    if (a.priority != b.priority) return a.priority > b.priority;
    return a.seq > b.seq;
  });
  return out;
}

int NotifyStore::count(NotifyStyle style) {
  lock();
  int c = 0;
  for (const Notification& n : items) {
    if (n.style == style) c++;
  }
  unlock();
  return c;
}

void NotifyStore::expire(const String& showingToast) {
  lock();
  uint32_t now = millis();
  for (size_t i = 0; i < items.size();) {
    const Notification& n = items[i];
    bool gone = n.deadlineMs != 0 && (int32_t)(now - n.deadlineMs) >= 0;
    if (n.style == NotifyStyle::Toast && n.id != showingToast &&
        now - n.createdMs > NOTIFY_TOAST_MAX_WAIT_MS) {
      gone = true;  // NotifyUi removes shown toasts long before this
    }
    if (gone) {
      changed(n.style);
      items.erase(items.begin() + i);
    } else {
      i++;
    }
  }
  unlock();
}

NotifyStore notifyStore;
