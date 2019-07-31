/*
 * Fader base class to generate lighting effects using ESP32 Digital LED Library
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

#include <Arduino.h>
#include "esp32_digital_led_lib.h"
#include "Fader.h"

Fader::Fader(strand_t * s): strand(s)
{
  fading=false;
  direction=false;
  step=1;
  delay_ms = 10;
}
pixelColor_t Fader::fadeFunction(int step, int position)
{
  return pixelFromRGB(step, step, step);
}
void Fader::ledDelay() { delay(delay_ms); }

void Fader::faderTask()
{
  if(fading)
  {
    for(int i = 0; i < strand->numPixels; i++)
    {
      strand->pixels[i]= fadeFunction(step, i);
    }
    ledDelay();
    nextStep();
  }
}
void Fader::nextStep()
{
  step+= direction ? stepSize : -stepSize;
  if(step >= maxStep || step <= 0)
  {
    switch(ft)
    {
      case CONTINUOUS:
      step = direction ? 0 : maxStep - 1;
      break;
      case STOP_AT_END:
      {
        fading = false;
        step = direction ? maxStep - 1 : 0;
      }
      break;
      case OSCILLATE:
      {
        step = direction ? maxStep - 1 : 0;
        direction = !direction;
      }
      break;
    }
  }
}
void Fader::update()
{
  faderTask();
  digitalLeds_drawPixels(&strand, 1);
}
void Fader::turnOff()
{
  digitalLeds_resetPixels(&strand, 1);
  update();
  fading = false;
}
void Fader::turnOn()
{
  direction = true;
  fading = true;
}
void Fader::fadeOff()
{
  direction = false;
  fading = true;
}
void Fader::setStep(int s)
{
  s = s > 0 ? s : 0;
  step = s;
  update();
}
void Fader::setFadeType(fadeType f) {ft = f;}
void Fader::setDirection(bool d) {direction = d;}
void Fader::setFade(bool f) {fading = f;}
void Fader::setMaxBrightness(uint8_t mb) {maxBrightness = mb;}
void Fader::setStepSize(uint8_t s ) {stepSize = s;}
static int limitRange(int val, int min, int max)
{
  val = val < min ? min : val;
  val = val > max ? max : val;
  return val;
}
uint8_t Fader::limitByte(int val) { return limitRange(val, 0, 255); }

static pixelColor_t scaleBrightness(pixelColor_t p, uint8_t brightness)
{
  if(brightness == 255) return p;
  if(brightness == 0) return pixelOff;
  pixelColor_t v;
  v.r = p.r != 0 ? p.r != 255 ?  p.r * brightness / 255 : 255 : 0;
  v.g = p.g != 0 ? p.g != 255 ?  p.g * brightness / 255 : 255 : 0;
  v.b = p.b != 0 ? p.b != 255 ?  p.b * brightness / 255 : 255 : 0;
  v.w = p.w != 0 ? p.w != 255 ?  p.w * brightness / 255 : 255 : 0;
  return v;
}
pixelColor_t Fader::scaleWithMaxBrightness(pixelColor_t p, uint8_t brightness)
{
  pixelColor_t v;
   v = brightness != 0 ? brightness != 255 ?  scaleBrightness(p, brightness) : v : pixelOff;
   v = maxBrightness != 0 ? maxBrightness != 255 ?  scaleBrightness(p, maxBrightness) : v : pixelOff;
   return v;
}
