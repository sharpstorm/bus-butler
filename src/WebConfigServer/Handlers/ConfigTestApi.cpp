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
  root["timestamp"] = timeNow;
  JsonArray services = root["services"].to<JsonArray>();

  for (int i = 0; i < apiCallResp->services.size(); i++) {
    JsonObject node = services.add<JsonObject>();
    BusArrivalServiceDetail* serviceArrivalDetails =
        &(apiCallResp->services[i]);
    node["service"] = serviceArrivalDetails->serviceNo;
    node["company"] = serviceArrivalDetails->company;

    if (serviceArrivalDetails->nextBus.arrivalTimestamp > 0) {
      JsonObject nextBus1 = node["nextBus1"].to<JsonObject>();
      formatBusResponse(nextBus1, serviceArrivalDetails->nextBus, timeNow);
    }
    if (serviceArrivalDetails->nextBus2.arrivalTimestamp > 0) {
      JsonObject nextBus2 = node["nextBus2"].to<JsonObject>();
      formatBusResponse(nextBus2, serviceArrivalDetails->nextBus2, timeNow);
    }
    if (serviceArrivalDetails->nextBus3.arrivalTimestamp > 0) {
      JsonObject nextBus3 = node["nextBus3"].to<JsonObject>();
      formatBusResponse(nextBus3, serviceArrivalDetails->nextBus3, timeNow);
    }
  }

  AsyncResponseStream* response = request->beginResponseStream(MimeType::JSON);
  serializeJson(root, *response);
  request->send(response);
}

void ConfigTestApi::formatBusResponse(const JsonObject& output,
                                      const BusArrivalDetail& arrivalDetails,
                                      long timeNow) {
  output["arrival"] = arrivalDetails.arrivalTimestamp;
  output["secondsLeft"] = arrivalDetails.arrivalTimestamp - timeNow;
  output["busType"] = toReadableBusType(arrivalDetails.busType);
  output["busLoad"] = toReadableBusLoad(arrivalDetails.load);
  output["isWAB"] = arrivalDetails.isWheelchairAccessible;
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
