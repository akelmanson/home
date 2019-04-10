// 200k 27.0

#include "config.h"
#include "PulseCounter.h"
#include "WiFi.h"
#include "MQTT.h"
#include "OTA.h"
// #include "Storage.h"
#include "common.h"
#include <string>
#include <sstream>

#define BLINK_GPIO GPIO_NUM_2

extern "C"
{
  void app_main();
}

void app_main()
{
  try
  {
    // ERROR_CHECK(gpio_pullup_dis(GPIO_NUM_22));
    // ERROR_CHECK(gpio_pulldown_en(GPIO_NUM_22));
    gpio_pad_select_gpio(BLINK_GPIO);
    ERROR_CHECK(gpio_set_direction(BLINK_GPIO, GPIO_MODE_OUTPUT));
    auto wifi = WiFi(WIFI_SSID, WIFI_PASSWORD, "dimmer-quarto");
    auto mqtt = MQTT(wifi, MQTT_URL);
    auto ota = OTA(mqtt);
    // auto storage = Storage();
    auto counter = PulseCounter(GPIO_NUM_23, GPIO_NUM_22, [&mqtt /*, &storage*/](count_t count) {
      // storage.set_u32("pulse_count", count);
      std::stringstream count_ss;
      count_ss << count;
      std::string count_s = count_ss.str();
      mqtt.publish("/in", count_s, 1, false);
    });
    // counter.count = storage.get_u32("pulse_count", 0);
    counter.run();
  }
  catch (int e)
  {
    printf("ERRO: %d", e);
  }
}
