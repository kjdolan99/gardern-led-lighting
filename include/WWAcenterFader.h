/*
 * Fader that sweeps through the white color temperature range
 * using amber, warm, and cool white LEDs. (SK6812 WWA)
 * Brightness and color temperature increase linearly towards the center of the strip.
 *
 * Copyright (c) 2019 Kevin Dolan
 */

/*
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "Fader.h"
#include "color_temp.h"

class WWAcenterFader : public Fader
{
private:
  uint16_t min_color_temp;
  int half_index;
  volatile ulong onDelay_ms;
  volatile ulong offDelay_ms;
  volatile ulong offTimeout_ms;
  volatile ulong lastOnTime;
protected:
  pixelColor_t fadeFunction(int step, int position);
  ulong ledDelay();
public:
  void update();
  explicit WWAcenterFader(strand_t  * s);
  void turnOn();
};

#define color_temp_min  1900
#define color_temp_max  7010 //color_temp_step = 10 with this limit
#define color_temp_step 10 // (color_temp_max - color_temp_min) / 511

static inline pixelColor_t pixelFromColorTemp(uint16_t colorTemp)
{
  uint32_t color_temp_index;
  colorTemp = colorTemp < color_temp_min ? color_temp_min : colorTemp;
  colorTemp = colorTemp > color_temp_max ? color_temp_max : colorTemp;

  color_temp_index = (colorTemp - color_temp_min) / color_temp_step;

  pixelColor_t v;
  v.r = color_temp_table[color_temp_index][0];
  v.g = color_temp_table[color_temp_index][1];
  v.b = color_temp_table[color_temp_index][2];
  v.w = 0;
  return v;
}
