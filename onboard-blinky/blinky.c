#include "ets_sys.h"
#include "osapi.h"
#include "gpio.h"
#include "os_type.h"


void ICACHE_FLASH_ATTR user_init()
{
  // init gpio subsytem
  gpio_init();

  gpio_output_set((0 << 1), 0, 0, 0);
  gpio_output_set((0 << 3), 0, 0, 0);
  gpio_output_set((0 << 15), 0, 0, 0);
  gpio_output_set((0 << 13), 0, 0, 0);

}
