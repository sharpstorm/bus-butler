#include "ConfigTestApi.h"

#include "../../Config/ConfigManager.h"
#include "../../Util/Time.h"
#include "../HttpConstants.h"
#include "esp_task_wdt.h"

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
    esp_task_wdt_reset();
  }

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

// Weather Query Implementation
struct QueryWeatherWorkerDto {
  String town;
  String region;
  unique_ptr<TwoHourForecastResponse> twoHourForecast;
  unique_ptr<TwentyFourHourForecastResponse> twentyFourHourForecast;
  volatile bool isCompleted;
};

void ConfigTestApi::queryWeatherWorker(void* pvParameters) {
  QueryWeatherWorkerDto* dtoPtr = (QueryWeatherWorkerDto*)(pvParameters);

  dtoPtr->twoHourForecast = weatherClient.getTwoHourForecast();
  esp_task_wdt_reset();
  dtoPtr->twentyFourHourForecast = weatherClient.getTwentyFourHourForecast();
  esp_task_wdt_reset();
  dtoPtr->isCompleted = true;
  vTaskDelete(NULL);
}

void ConfigTestApi::queryWeather(AsyncWebServerRequest* request) {
  if (!queryWeatherCompleted) {
    request->send(422, MimeType::JSON,
                  "{ \"error\": \"API Request in flight\" }");
    return;
  }

  String town = configManager.getWeatherTown();
  String region = configManager.getWeatherRegion();

  if (town.length() == 0 || region.length() == 0) {
    request->send(422, MimeType::JSON,
                  "{ \"error\": \"Weather town or region not configured\" }");
    return;
  }

  TaskHandle_t taskRef = NULL;
  QueryWeatherWorkerDto taskResult{town, region, nullptr, nullptr, false};

  xTaskCreate(queryWeatherWorker, "query-weather-worker", 8000, &taskResult, 1,
              &taskRef);

  while (!taskResult.isCompleted) {
    // Yield to not trigger watchdog
    vTaskDelay(pdMS_TO_TICKS(20));
    esp_task_wdt_reset();
  }

  if (taskResult.twoHourForecast == nullptr ||
      taskResult.twentyFourHourForecast == nullptr) {
    request->send(500, MimeType::JSON,
                  "{ \"error\": \"API Call to weather service failed\" }");
    return;
  }

  // Serialize Response
  JsonDocument doc;
  JsonObject root = doc.to<JsonObject>();
  root["town"] = town;
  root["region"] = region;

  // Two Hour Forecast
  JsonObject twoHour = root["twoHourForecast"].to<JsonObject>();
  WeatherCondition townForecast =
      weatherClient.getForecastForArea(taskResult.twoHourForecast.get(), town);
  twoHour["forecast"] = weatherConditionToString(townForecast);

  // Twenty Four Hour Forecast
  JsonObject twentyFourHour = root["twentyFourHourForecast"].to<JsonObject>();
  TwentyFourHourForecastResponse* tfResp =
      taskResult.twentyFourHourForecast.get();

  JsonArray periods = twentyFourHour["periods"].to<JsonArray>();

  // Morning period
  JsonObject morningObj = periods.add<JsonObject>();
  morningObj["timePeriod"] = "Morning";
  WeatherCondition morningForecast = WeatherCondition::Unknown;
  if (region == "west")
    morningForecast = tfResp->morning.regionWest;
  else if (region == "east")
    morningForecast = tfResp->morning.regionEast;
  else if (region == "central")
    morningForecast = tfResp->morning.regionCentral;
  else if (region == "south")
    morningForecast = tfResp->morning.regionSouth;
  else if (region == "north")
    morningForecast = tfResp->morning.regionNorth;
  morningObj["forecast"] = weatherConditionToString(morningForecast);

  // Afternoon period
  JsonObject afternoonObj = periods.add<JsonObject>();
  afternoonObj["timePeriod"] = "Afternoon";
  WeatherCondition afternoonForecast = WeatherCondition::Unknown;
  if (region == "west")
    afternoonForecast = tfResp->afternoon.regionWest;
  else if (region == "east")
    afternoonForecast = tfResp->afternoon.regionEast;
  else if (region == "central")
    afternoonForecast = tfResp->afternoon.regionCentral;
  else if (region == "south")
    afternoonForecast = tfResp->afternoon.regionSouth;
  else if (region == "north")
    afternoonForecast = tfResp->afternoon.regionNorth;
  afternoonObj["forecast"] = weatherConditionToString(afternoonForecast);

  // Night period
  JsonObject nightObj = periods.add<JsonObject>();
  nightObj["timePeriod"] = "Night";
  WeatherCondition nightForecast = WeatherCondition::Unknown;
  if (region == "west")
    nightForecast = tfResp->night.regionWest;
  else if (region == "east")
    nightForecast = tfResp->night.regionEast;
  else if (region == "central")
    nightForecast = tfResp->night.regionCentral;
  else if (region == "south")
    nightForecast = tfResp->night.regionSouth;
  else if (region == "north")
    nightForecast = tfResp->night.regionNorth;
  nightObj["forecast"] = weatherConditionToString(nightForecast);

  // Night period
  JsonObject midnightObj = periods.add<JsonObject>();
  midnightObj["timePeriod"] = "Midnight";
  WeatherCondition midnightForecast = WeatherCondition::Unknown;
  if (region == "west")
    midnightForecast = tfResp->midnight.regionWest;
  else if (region == "east")
    midnightForecast = tfResp->midnight.regionEast;
  else if (region == "central")
    midnightForecast = tfResp->midnight.regionCentral;
  else if (region == "south")
    midnightForecast = tfResp->midnight.regionSouth;
  else if (region == "north")
    midnightForecast = tfResp->midnight.regionNorth;
  midnightObj["forecast"] = weatherConditionToString(midnightForecast);

  AsyncResponseStream* response = request->beginResponseStream(MimeType::JSON);
  serializeJson(root, *response);
  request->send(response);
}

NeaWeatherClient ConfigTestApi::weatherClient;
volatile bool ConfigTestApi::queryWeatherCompleted = true;
