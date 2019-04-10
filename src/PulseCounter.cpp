/* Pulse counter module - Example
   For other examples please check:
   https://github.com/espressif/esp-idf/tree/master/examples
   This example code is in the Public Domain (or CC0 licensed, at your option.)
   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include <functional>
#include <algorithm>
#include "freertos/FreeRTOS.h"
#include "freertos/portmacro.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/periph_ctrl.h"
#include "driver/ledc.h"
#include "driver/gpio.h"
#include "driver/pcnt.h"
#include "esp_attr.h"
#include "esp_log.h"
#include "soc/gpio_sig_map.h"
#include "PulseCounter.h"
#include "common.h"

typedef struct
{
  int16_t val;
} pcnt_evt_t;

void IRAM_ATTR PulseCounter::handle_interrupt()
{
  pcnt_evt_t evt;
  auto levelB = gpio_get_level(pinB);
  auto levelA = gpio_get_level(pinA);
  evt.val = levelB == levelA ? -1 : 1;
  if (levelA == currentPosition)
  {
    return;
  }
  currentPosition = levelA;
  portBASE_TYPE HPTaskAwoken = pdFALSE;
  xQueueSendToBackFromISR(pcnt_evt_queue, &evt, &HPTaskAwoken);
  if (HPTaskAwoken == pdTRUE)
  {
    portYIELD_FROM_ISR();
  }
}

void PulseCounter::run()
{
  gpio_config_t config = {
      .pin_bit_mask = (1ULL << pinA),
      .mode = GPIO_MODE_INPUT,
      .pull_up_en = GPIO_PULLUP_DISABLE,
      .pull_down_en = GPIO_PULLDOWN_ENABLE,
      .intr_type = GPIO_INTR_ANYEDGE,
  };
  ERROR_CHECK(gpio_config(&config));
  ERROR_CHECK(gpio_install_isr_service(0));
  ERROR_CHECK(gpio_isr_handler_add(pinA, [](void *p) { ((PulseCounter *)p)->handle_interrupt(); }, this));
  pcnt_evt_queue = xQueueCreate(100, sizeof(pcnt_evt_t));
  pcnt_evt_t evt;
  portBASE_TYPE res;

  while (1)
  {
    res = xQueueReceive(pcnt_evt_queue, &evt, 10000 / portTICK_PERIOD_MS);
    if (res == pdFALSE)
    {
      continue;
    }
    count += evt.val;
    // auto newCountValue = std::max(std::min(count + evt.val, 8), 0);
    // if (newCountValue == lastProcessedCount)
    // {
    //   continue;
    // }
    // count = newCountValue;
    if (uxQueueMessagesWaiting(pcnt_evt_queue) > 0)
    {
      continue;
    }
    this->callback(count);
    count = 0;
    // lastProcessedCount = count;
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}

PulseCounter::~PulseCounter()
{
  gpio_isr_handler_remove(pinA);
}
