#include "MQTT.h"
#include <esp_ota_ops.h>

class OTA
{
protected:
  esp_ota_handle_t update_handle = 0;
  const esp_partition_t *update_partition = NULL;

  void begin();
  void write(const void *data, size_t size);
  void end();

public:
  OTA(MQTT &mqtt);
};