#include <Arduino.h>
#include "esp32_digital_led_lib.h"
#include "color_temp.h"

enum fadeType {CONTINUOUS, STOP_AT_END, OSCILLATE};

class Fader
{
protected:
  strand_t * strand;
  fadeType ft;
  int maxStep;
  virtual pixelColor_t fadeFunction(int step, int position) = 0;
  bool direction;
private:
  int step;
public:
  bool fading;
  int fadeStep;
  uint8_t maxBrightness;
  unsigned long delay_ms;
  virtual void ledDelay()
  {
    delay(delay_ms);
  }
  explicit Fader(strand_t * s): strand(s)
  {
    fading=false;
    direction=false;
    step=0;
    delay_ms = 10;
  }
  virtual void faderTask()
  {
    if(fading)
    {
      for(int i = 0; i < strand->numPixels; i++)
      {
        strand->pixels[i]= fadeFunction(step, i);
      }
      update();
      ledDelay();
      nextStep();
    }
  }
  virtual void nextStep()
  {
    step+= direction ? fadeStep : -fadeStep;
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
  void update()
  {
    digitalLeds_drawPixels(&strand, 1);
  }
  void turnOff()
  {
    digitalLeds_resetPixels(&strand, 1);
    update();
    fading = false;
  }
  virtual void turnOn()
  {
    direction = true;
    fading = true;
  }
  void fadeOff()
  {
    direction = false;
    fading = true;
  }
  void setStep(int s)
  {
    s = s > 0 ? s : 0;
    step = s;
    update();
  }
  void setDirection(bool d) {direction = d;}
  void setFade(bool f) {fading = f;}
  void setMaxBrightness(uint8_t mb) {maxBrightness = mb;}
};

#define min_color_temp 2500

class WWAcenterFader : public Fader
{
private:
  int half_index;
  int onDelay_ms = 20;
  int offDelay_ms = onDelay_ms * 20;
  int offTimeout_ms = 60000;
  ulong lastOnTime;
public:
  pixelColor_t fadeFunction(int step, int position)
  {
    int p = position > half_index ? strand->numPixels - position - 1 : position;
    return pixelFromColorTemp(min_color_temp + (step + p) * 20, step + p);
  }
  void ledDelay()
  {
      int d = direction ? onDelay_ms : offDelay_ms;
      delay(d);
  }
  void faderTask()
  {
    if(millis() - lastOnTime > offTimeout_ms) fadeOff();
    Fader::faderTask();
  }
  void turnOn()
  {
    lastOnTime = millis();
    Fader::turnOn();
  }
  explicit WWAcenterFader(strand_t  * s) : Fader(s)
  {
    half_index = strand->numPixels / 2;
    maxStep = 255 - half_index;
    fadeStep = 1;
    setDirection(true);
    setMaxBrightness(255);
    delay_ms = onDelay_ms;
    ft = STOP_AT_END;
  }
};

/*
void fade_from_middle(strand_t * strand, bool on_or_off, int d)
{
    int half_index = strand->numPixels / 2;

    pixelColor_t current_color;
    int color_temp, brightLimit;
    int loop_limit = 256 - half_index;

    for(int i = 0; i < loop_limit; i++)
    {
        for(int j = 0; j < half_index; j++)
        {
          brightLimit = on_or_off ? i + j : loop_limit - i + j;
          color_temp = min_color_temp + brightLimit * 20;
          current_color = pixelFromColorTemp(color_temp, brightLimit);
          strand->pixels[j] = current_color;
          strand->pixels[strand->numPixels - j -1] = current_color;
        }
        digitalLeds_drawPixels(&strand, STRANDCNT);
        delay(d);
    }
}
*/
