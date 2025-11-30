#include "ConfigTestApi.h"

#include "../../Util/Time.h"
#include "../HttpConstants.h"

struct QueryBusArrivalWorkerDto {
  String busStopCode;
  unique_ptr<BusArrivalApiResponse> apiResp;
};

void ConfigTestApi::queryBusArrivalWorker(void* pvParameters) {
  QueryBusArrivalWorkerDto* dtoPtr = (QueryBusArrivalWorkerDto*)(pvParameters);
  dtoPtr->apiResp = ltaDatamallClient.getBusArrival(dtoPtr->busStopCode);
  queryBusArrivalCompleted = true;
  vTaskDelete(NULL);
}

void ConfigTestApi::queryBusArrival(AsyncWebServerRequest* request,
                                    JsonVariant& jsonBody) {
  JsonObject reqBody = jsonBody.as<JsonObject>();
  if (!initDatamallClient()) {
    request->send(422, MimeType::JSON,
                  "{ \"error\": \"No API key configured\" }");
    return;
  }

  if (!queryBusArrivalCompleted) {
    request->send(422, MimeType::JSON,
                  "{ \"error\": \"API Request in flight\" }");
    return;
  }

  if (reqBody["busStopCode"].isUnbound()) {
    request->send(400, MimeType::JSON, "{ \"error\": \"No busStopCode\" }");
    return;
  }

  TaskHandle_t taskRef = NULL;
  QueryBusArrivalWorkerDto taskResult{reqBody["busStopCode"].as<String>(),
                                      nullptr};

  queryBusArrivalCompleted = false;
  xTaskCreate(queryBusArrivalWorker, "query-bus-arrival-worker", 8000,
              &taskResult, 1, &taskRef);

  while (!queryBusArrivalCompleted) {
    // Yield to not trigger watchdog
    vTaskDelay(pdMS_TO_TICKS(10));
  }
  vTaskDelete(taskRef);

  if (taskResult.apiResp == nullptr) {
    request->send(500, MimeType::JSON,
                  "{ \"error\": \"API Call to server failed\" }");
    return;
  }

  // Serialize Response
  long timeNow = TimeUtil::getUnixTime();
  BusArrivalApiResponse* apiCallResp = taskResult.apiResp.get();
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  root["busStop"] = apiCallResp->busStopCode;
  // TODO: Fix timestamp
  root["timestamp"] = timeNow;
  JsonArray services = root["services"].to<JsonArray>();

  for (int i = 0; i < apiCallResp->services.size(); i++) {
    JsonObject node = services.add<JsonObject>();
    node["service"] = apiCallResp->services[i].serviceNo;

    node["company"] = apiCallResp->services[i].company;
    JsonObject nextBus1 = node["nextBus1"].to<JsonObject>();
    nextBus1["arrival"] = apiCallResp->services[i].nextBus.arrivalTimestamp;
    nextBus1["secondsLeft"] =
        apiCallResp->services[i].nextBus.arrivalTimestamp - timeNow;
    nextBus1["busType"] =
        toReadableBusType(apiCallResp->services[i].nextBus.busType);
    nextBus1["busLoad"] =
        toReadableBusLoad(apiCallResp->services[i].nextBus.load);
    nextBus1["isWAB"] = apiCallResp->services[i].nextBus.isWheelchairAccessible;

    JsonObject nextBus2 = node["nextBus2"].to<JsonObject>();
    nextBus2["arrival"] = apiCallResp->services[i].nextBus2.arrivalTimestamp;
    nextBus2["secondsLeft"] =
        apiCallResp->services[i].nextBus2.arrivalTimestamp - timeNow;
    nextBus2["busType"] =
        toReadableBusType(apiCallResp->services[i].nextBus2.busType);
    nextBus2["busLoad"] =
        toReadableBusLoad(apiCallResp->services[i].nextBus2.load);
    nextBus2["isWAB"] =
        apiCallResp->services[i].nextBus2.isWheelchairAccessible;

    JsonObject nextBus3 = node["nextBus3"].to<JsonObject>();
    nextBus3["arrival"] = apiCallResp->services[i].nextBus3.arrivalTimestamp;
    nextBus3["secondsLeft"] =
        apiCallResp->services[i].nextBus3.arrivalTimestamp - timeNow;
    nextBus3["busType"] =
        toReadableBusType(apiCallResp->services[i].nextBus3.busType);
    nextBus3["busLoad"] =
        toReadableBusLoad(apiCallResp->services[i].nextBus3.load);
    nextBus3["isWAB"] =
        apiCallResp->services[i].nextBus3.isWheelchairAccessible;
  }

  AsyncResponseStream* response = request->beginResponseStream(MimeType::JSON);
  serializeJson(root, *response);
  request->send(response);
}

bool ConfigTestApi::initDatamallClient() {
  if (ltaDatamallClient.isConfigured()) {
    return true;
  }

  ltaDatamallClient.reconfigure();
  return ltaDatamallClient.isConfigured();
}

LtaDatamallClient ConfigTestApi::ltaDatamallClient;
volatile bool ConfigTestApi::queryBusArrivalCompleted = true;
