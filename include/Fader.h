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

enum fadeType {CONTINUOUS, STOP_AT_END, OSCILLATE};
const pixelColor_t pixelOff = pixelFromRGBW(0,0,0,0);

class Fader
{
protected:
  strand_t * strand;
  volatile fadeType ft;
  volatile int maxStep;
  volatile bool direction;
  virtual pixelColor_t fadeFunction(int step, int position);
  virtual void ledDelay();
  virtual void nextStep();
  virtual void faderTask();
private:
  volatile int step;

public:
  void update();
  volatile bool fading, updating;
  volatile int stepSize;
  uint8_t maxBrightness;
  volatile ulong delay_ms;

  explicit Fader(strand_t * s);
  void turnOff();
  virtual void turnOn();
  void fadeOff();
  void setStep(int s);
  inline void setFadeType(fadeType f) {ft = f;}
  inline void setDirection(bool d) {direction = d;}
  inline void setFade(bool f) {fading = f;}
  inline void setMaxBrightness(uint8_t mb) {maxBrightness = mb;}
  inline void setStepSize(uint8_t s ) {stepSize = s;}
  pixelColor_t scaleMaxBrightness(pixelColor_t p, uint8_t brightness);
};
static inline int limitRange(int val, int min, int max)
{
  val = val < min ? min : val;
  val = val > max ? max : val;
  return val;
}
static inline uint8_t limitByte(int val) { return limitRange(val, 0, 255); }
static inline pixelColor_t scaleBrightness(pixelColor_t p, uint8_t brightness)
{
  if(brightness == 255) return p;
  if(brightness == 0) return pixelOff;

  p.r =  p.r * brightness / 255; 
  p.g =  p.g * brightness / 255; 
  p.b =  p.b * brightness / 255; 
  p.w =  p.w * brightness / 255; 
  return p;
}
