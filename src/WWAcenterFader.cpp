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

#include "WWAcenterFader.h"

WWAcenterFader::WWAcenterFader(strand_t  * s) : Fader(s)
{
  half_index = strand->numPixels / 2;
  onDelay_ms = 20;
  offDelay_ms = onDelay_ms * 20;
  offTimeout_ms = 60000;
  maxStep = 255 - half_index;
  setDirection(true);
  maxBrightness = 255;
  ft = STOP_AT_END;
  lastOnTime = millis();
}
pixelColor_t WWAcenterFader::fadeFunction(int step, int position)
{
  uint8_t p = position > half_index ? strand->numPixels - position - 1 : position;
  return scaleBrightness( pixelFromColorTemp(min_color_temp + (step + p) * 20),  limitByte(step + p));
}
void WWAcenterFader::ledDelay()
{
    int d = direction ? onDelay_ms : offDelay_ms;
    delay(d);
}
void WWAcenterFader::faderTask()
{
  if(millis() - lastOnTime > offTimeout_ms) fadeOff();
  Fader::faderTask();
}
void WWAcenterFader::turnOn()
{
  lastOnTime = millis();
  Fader::turnOn();
}
