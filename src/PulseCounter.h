#include "driver/pcnt.h"
#include <functional>

typedef int32_t count_t;
typedef std::function<void(count_t count)> callback_fn;

class PulseCounter
{
protected:
  gpio_num_t pinA;
  gpio_num_t pinB;
  int currentPosition = -1;
  pcnt_unit_t unit;
  callback_fn callback;
  xQueueHandle pcnt_evt_queue; // A queue to handle pulse counter events

  void IRAM_ATTR handle_interrupt();
  void pcnt_example_init(void);

public:
  count_t count = 0;
  count_t lastProcessedCount = 0;
  PulseCounter(gpio_num_t pinA, gpio_num_t pinB, callback_fn callback) : pinA(pinA), pinB(pinB), callback(callback){};
  void run();
  ~PulseCounter();
};