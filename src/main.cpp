/*
 * ESP32 Project to drive SK6812 WWA LEDs to light my garden.
 * Uses the ESP 32 Digital LED library
 *
 * Modifications Copyright (c) 2019 Kevin Dolan
 *
 * Original Copyright (c) 2017-2019 Martin F. Falatic
 *
 * Based on public domain code created 19 Nov 2016 by Chris Osborn <fozztexx@fozztexx.com>
 * http://insentricity.com
 *
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
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

#include "WiFiCreds.h"
//const char* ssid = "..........";
//const char* password = "..........";

#include "esp32_digital_led_lib.h"
#include "WWAcenterFader.h"

#if defined(ARDUINO) && ARDUINO >= 100
  // No extras
#elif defined(ARDUINO) // pre-1.0
  // No extras
#elif defined(ESP_PLATFORM)
  #include "arduinoish.hpp"
#endif


#define COUNT_OF(x) ((sizeof(x)/sizeof(0[x])) / ((size_t)(!(sizeof(x) % sizeof(0[x])))))

// **Required** if debugging is enabled in library header
// TODO: Is there any way to put this in digitalLeds_addStrands() and avoid undefined refs?
#if DEBUG_ESP32_DIGITAL_LED_LIB
  int digitalLeds_debugBufferSz = 1024;
  char * digitalLeds_debugBuffer = static_cast<char*>(calloc(digitalLeds_debugBufferSz, sizeof(char)));
#endif

int MOTION_PIN = 35;


#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmissing-field-initializers"  // It's noisy here with `-Wall`

strand_t STRANDS[] = { // Avoid using any of the strapping pins on the ESP32, anything >=32, 16, 17... not much left.
  {.rmtChannel = 0, .gpioNum = 14, .ledType = LED_SK6812_V1, .brightLimit = 75, .numPixels =  150}
};

//strand_t STRAND0 = {.rmtChannel = 1, .gpioNum = 14, .ledType = LED_WS2812B_V3, .brightLimit = 24, .numPixels =  93,
//   .pixels = nullptr, ._stateVars = nullptr};

#pragma GCC diagnostic pop

int STRANDCNT = COUNT_OF(STRANDS);


//**************************************************************************//
boolean initStrands()
{
  /****************************************************************************
     If you have multiple strands connected, but not all are in use, the
     GPIO power-on defaults for the unused strand data lines will typically be
     high-impedance. Unless you are pulling the data lines high or low via a
     resistor, this will lead to noise on those unused but connected channels
     and unwanted driving of those unallocated strands.
     This optional gpioSetup() code helps avoid that problem programmatically.
  ****************************************************************************/

  digitalLeds_initDriver();

  for (int i = 0; i < STRANDCNT; i++) {
    gpioSetup(STRANDS[i].gpioNum, OUTPUT, LOW);
  }

  strand_t * strands[8];
  for (int i = 0; i < STRANDCNT; i++) {
    strands[i] = &STRANDS[i];
  }
  int rc = digitalLeds_addStrands(strands, STRANDCNT);
  if (rc) {
    Serial.print("Init rc = ");
    Serial.println(rc);
    return false;
  }

  for (int i = 0; i < STRANDCNT; i++) {
    strand_t * pStrand = strands[i];
    Serial.print("Strand ");
    Serial.print(i);
    Serial.print(" = ");
    Serial.print((uint32_t)(pStrand->pixels), HEX);
    Serial.println();
    #if DEBUG_ESP32_DIGITAL_LED_LIB
      dumpDebugBuffer(-2, digitalLeds_debugBuffer);
    #endif
  }

  return true;
}


// Hacky debugging method
// espPinMode((gpio_num_t)5, OUTPUT);
// gpio_set_level((gpio_num_t)5, 0);
// gpio_set_level((gpio_num_t)5, 1);  gpio_set_level((gpio_num_t)5, 0);

//Motion Sensor setup

volatile int interruptCounter = 0;
int numberOfInterrupts = 0;

portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

//Interrupt handler for the motion sensor
void IRAM_ATTR handleInterrupt() {
  portENTER_CRITICAL_ISR(&mux);
  interruptCounter++;
  portEXIT_CRITICAL_ISR(&mux);
}

//**************************************************************************//
WWAcenterFader * fader;
void setup()
{
  Serial.begin(115200);
  Serial.println("Initializing...");

  if (!initStrands()) {
    Serial.println("Init FAILURE: halting");
    while (true) {
      delay(100);
    }
  }
  //Setup the motion sensor input
  pinMode(MOTION_PIN, INPUT);
  attachInterrupt(digitalPinToInterrupt(MOTION_PIN), handleInterrupt, RISING);

  fader = new WWAcenterFader(&STRANDS[0]);
  fader->turnOn();

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.waitForConnectResult() != WL_CONNECTED) {
    Serial.println("Connection Failed! Rebooting...");
    delay(5000);
    ESP.restart();
  }

  ArduinoOTA
    .onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";

      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      Serial.println("Start updating " + type);
    })
    .onEnd([]() {
      Serial.println("\nEnd");
    })
    .onProgress([](unsigned int progress, unsigned int total) {
      Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    })
    .onError([](ota_error_t error) {
      Serial.printf("Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
      else if (error == OTA_END_ERROR) Serial.println("End Failed");
    });

  ArduinoOTA.begin();

  Serial.println("Ready");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  delay(100);
  Serial.println("Init complete");
}

//**************************************************************************//
void loop()
{
 //Handle Motion Events
  if(interruptCounter>0){

		      portENTER_CRITICAL(&mux);
		      interruptCounter=0;
		      portEXIT_CRITICAL(&mux);

		      numberOfInterrupts++;
		      //Motion Sensor Debuging
		      //Serial.print("An interrupt has occurred. Total: ");
		      //Serial.println(numberOfInterrupts);

          fader->turnOn();
		  }
  ArduinoOTA.handle();

  #if DEBUG_ESP32_DIGITAL_LED_LIB
    dumpDebugBuffer(0, digitalLeds_debugBuffer);
  #endif
}

//**************************************************************************//
