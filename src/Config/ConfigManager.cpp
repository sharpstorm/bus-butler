#include "ConfigManager.h"

ConfigManager::ConfigManager() {}

void ConfigManager::saveLtaDatamallApiKey(String apiKey) {
  preferences.begin(CONFIG_NAMESPACE, false);
  preferences.putString(CONFIG_BUS_API_KEY, apiKey);
  preferences.end();
}

String ConfigManager::getLtaDatamallApiKey() {
  preferences.begin(CONFIG_NAMESPACE, false);
  String apiKey = preferences.getString(CONFIG_BUS_API_KEY, "");
  preferences.end();

  return apiKey;
}

ConfigManager configManager;
