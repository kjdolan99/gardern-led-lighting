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
  fading=true;
  direction=true;
  step=1;
  delay_ms = 10;
  maxStep = 255;
  stepSize = 1;
  maxBrightness = strand->brightLimit;
  ft = CONTINUOUS;
  startFaderTask(this);
};
Fader::~Fader() { stopFaderTask(this); }
pixelColor_t Fader::fadeFunction(int step, int position)
{
  return pixelFromRGB(step, step, step);
}
void Fader::updatePixels()
{
  for(int i = 0; i < strand->numPixels; i++)
    {
      strand->pixels[i]= fadeFunction(step, i);
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
  if(fading)
  {
    updating = true;
    nextStep();
    updatePixels();
    digitalLeds_drawPixels(&strand, 1);
    updating = false;
  }
}
ulong Fader::ledDelay() { return delay_ms; }
void Fader::turnOff()
{
  digitalLeds_resetPixels(&strand, 1);
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
  step = limitRange(s, 0, maxStep);
}
pixelColor_t Fader::scaleMaxBrightness(pixelColor_t p, uint8_t brightness)
{
   p = brightness == 0 ? pixelOff: brightness == 255 ? p: scaleBrightness(p, brightness);
   p = maxBrightness == 0 ? pixelOff: maxBrightness == 255 ? p: scaleBrightness(p, maxBrightness);
   return p;
}
