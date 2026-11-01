#pragma once

#include <Arduino.h>

/*
  Runs a notification's action (an HTTP request to the URL the pusher gave)
  on its own small task, so the screen never waits on the network. The
  action's state (running / ok / failed) is written back to the store.
*/
namespace NotifyActions {

// Returns false if the queue is full.
bool run(const String& id, uint32_t revision, size_t index, const String& url,
         const String& method);

}  // namespace NotifyActions
