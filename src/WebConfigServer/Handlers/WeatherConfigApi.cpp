#include "WeatherConfigApi.h"

#include "../../Config/ConfigManager.h"
#include "../../NeaWeatherApi/NeaWeatherApiClient.h"
#include "../HttpConstants.h"

// Static client for fetching available towns
static NeaWeatherClient weatherClient;

void WeatherConfigApi::listWeatherRegions(AsyncWebServerRequest* request) {
  String region = configManager.getWeatherRegion();

  request->send(
      200, MimeType::JSON,
      "{ \"region\": [\"west\",\"east\",\"central\",\"north\",\"south\"] }");
}

void WeatherConfigApi::getWeatherRegion(AsyncWebServerRequest* request) {
  String region = configManager.getWeatherRegion();

  request->send(200, MimeType::JSON, "{ \"region\": \"" + region + "\" }");
}

void WeatherConfigApi::saveWeatherRegion(AsyncWebServerRequest* request,
                                         JsonVariant& jsonBody) {
  JsonObject reqBody = jsonBody.as<JsonObject>();

  if (reqBody["region"].isUnbound()) {
    request->send(400, MimeType::JSON, "{ \"success\": false }");
    return;
  }
  configManager.saveWeatherRegion(reqBody["region"].as<String>());
  request->send(200, MimeType::JSON, "{ \"success\": true }");
}

void WeatherConfigApi::getWeatherTown(AsyncWebServerRequest* request) {
  String town = configManager.getWeatherTown();

  if (town.length() > 0) {
    request->send(200, MimeType::JSON, "{ \"town\": \"" + town + "\" }");
  } else {
    request->send(200, MimeType::JSON, "{ \"town\": null }");
  }
}

void WeatherConfigApi::saveWeatherTown(AsyncWebServerRequest* request,
                                       JsonVariant& jsonBody) {
  JsonObject reqBody = jsonBody.as<JsonObject>();

  if (reqBody["town"].isUnbound()) {
    request->send(400, MimeType::JSON, "{ \"success\": false }");
    return;
  }

  String town = reqBody["town"].as<String>();
  town.trim();

  configManager.saveWeatherTown(town);

  request->send(200, MimeType::JSON, "{ \"success\": true }");
}

// Worker task for async API call
struct TownListWorkerContext {
  unique_ptr<TwoHourForecastResponse> response;
  TaskHandle_t callerTask;
};

static void townListWorker(void* pvParameters) {
  TownListWorkerContext* ctx =
      static_cast<TownListWorkerContext*>(pvParameters);

  ctx->response = weatherClient.getTwoHourForecast();

  if (ctx->callerTask != nullptr) {
    xTaskNotifyGive(ctx->callerTask);
  }

  vTaskDelete(NULL);
}

void WeatherConfigApi::listWeatherTowns(AsyncWebServerRequest* request) {
  TownListWorkerContext ctx;
  ctx.response = nullptr;
  ctx.callerTask = xTaskGetCurrentTaskHandle();

  ulTaskNotifyTake(pdTRUE, 0);

  TaskHandle_t taskHandle = NULL;
  BaseType_t created =
      xTaskCreate(townListWorker, "town-list", 8192, &ctx, 1, &taskHandle);

  if (created != pdPASS) {
    request->send(500, MimeType::JSON,
                  "{ \"error\": \"Task creation failed\" }");
    return;
  }

  // Wait up to 5 seconds
  BaseType_t notified = ulTaskNotifyTake(pdTRUE, pdMS_TO_TICKS(5000));

  if (notified == 0) {
    vTaskDelete(taskHandle);
    request->send(504, MimeType::JSON, "{ \"error\": \"Request timed out\" }");
    return;
  }

  if (ctx.response == nullptr) {
    request->send(502, MimeType::JSON,
                  "{ \"error\": \"NEA API request failed\" }");
    return;
  }

  // Build response
  AsyncResponseStream* response = request->beginResponseStream(MimeType::JSON);

  JsonDocument doc;
  JsonArray towns = doc["towns"].to<JsonArray>();
  for (const auto& forecast : ctx.response->forecasts) {
    towns.add(forecast.area);
  }

  serializeJson(doc, *response);
  request->send(response);
}