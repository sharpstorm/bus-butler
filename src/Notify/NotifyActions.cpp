#include "NotifyActions.h"

#include <HTTPClient.h>
#include <WiFi.h>

#include "NotifyStore.h"

namespace NotifyActions {

struct Job {
  String id;
  uint32_t revision;
  size_t index;
  String url;
  String method;
};

static QueueHandle_t queue = nullptr;
static TaskHandle_t task = nullptr;

static void runJob(const Job& job) {
  bool ok = false;
  if (WiFi.isConnected()) {
    WiFiClient client;
    HTTPClient http;
    http.setConnectTimeout(4000);
    http.setTimeout(10000);
    if (http.begin(client, job.url)) {
      http.addHeader("User-Agent", "BusButler");
      int code = job.method == "GET" ? http.GET() : http.POST((uint8_t*)"", 0);
      ok = code >= 200 && code < 300;
      if (!ok) Serial.printf("Notification action %s: HTTP %d\n", job.url.c_str(), code);
      http.end();
    }
  }
  notifyStore.setActionState(job.id, job.revision, job.index,
                             ok ? ActionState::Ok : ActionState::Failed);
}

static void taskMain(void*) {
  while (true) {
    Job* job = nullptr;
    if (xQueueReceive(queue, &job, portMAX_DELAY) == pdTRUE && job != nullptr) {
      runJob(*job);
      delete job;
    }
  }
}

bool run(const String& id, uint32_t revision, size_t index, const String& url,
         const String& method) {
  if (queue == nullptr) {
    queue = xQueueCreate(4, sizeof(Job*));
    xTaskCreatePinnedToCore(taskMain, "notify-actions", 6144, nullptr, 1,
                            &task, 0);
  }
  Job* job = new Job{id, revision, index, url, method};
  // Before queueing: the task may finish before we'd get to it after
  notifyStore.setActionState(id, revision, index, ActionState::Running);
  if (xQueueSend(queue, &job, 0) != pdTRUE) {
    delete job;
    notifyStore.setActionState(id, revision, index, ActionState::Idle);
    return false;
  }
  return true;
}

}  // namespace NotifyActions
