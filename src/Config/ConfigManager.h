#pragma once

#include "Preferences.h"

#define CONFIG_NAMESPACE "butler-config"

#define CONFIG_BUS_API_KEY "bus-api-key"

class ConfigManager {
 private:
  Preferences preferences;

 public:
  ConfigManager();

  void saveLtaDatamallApiKey(String apiKey);
  String getLtaDatamallApiKey();
};

extern ConfigManager configManager;
