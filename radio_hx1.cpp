#include "config.h"
#include "pin.h"
#include "radio_hx1.h"
#if (ARDUINO + 1) >= 100
#  include <Arduino.h>
#else
#  include <WProgram.h>
#endif


void RadioHx1::setup()
{
  // Configure pins
  pinMode(PTT_PIN, OUTPUT);
  pin_write(PTT_PIN, LOW);
  pinMode(AUDIO_PIN, OUTPUT);
}

void RadioHx1::ptt_on()
{
  pin_write(PTT_PIN, HIGH);
  delay(100);
}

void RadioHx1::ptt_off()
{
  pin_write(PTT_PIN, LOW);
}
