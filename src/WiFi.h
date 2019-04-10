#pragma once

#include <string>
#include "esp_event_loop.h"

class WiFi
{
protected:
  std::string hostname;
  void wifi_init_sta(const char *ssid, const char *password);
  esp_err_t event_handler(system_event_t *event);

public:
  WiFi(const char *ssid, const char *password, const char *hostname);
  void wait_to_connect();
};
