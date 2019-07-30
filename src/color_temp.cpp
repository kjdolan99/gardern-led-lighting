
#include "esp32_digital_led_lib.h"
#include "color_temp.h"

#define color_temp_min  1900
#define color_temp_max  7010 //color_temp_step = 10 with this limit
#define color_temp_step 10 // (color_temp_max - color_temp_min) / 511

uint32_t color_temp_index;
pixelColor_t pixelFromColorTemp(uint16_t colorTemp, uint8_t brightLimit)
{
  colorTemp = colorTemp < color_temp_min ? color_temp_min : colorTemp;
  colorTemp = colorTemp > color_temp_max ? color_temp_max : colorTemp;

  color_temp_index = (colorTemp - color_temp_min) / color_temp_step;

  pixelColor_t v;
  v.r = color_temp_table[color_temp_index][0] * brightLimit / 256;
  v.g = color_temp_table[color_temp_index][1] * brightLimit / 256;
  v.b = color_temp_table[color_temp_index][2] * brightLimit / 256;
  v.w = 0;
  return v;
}
