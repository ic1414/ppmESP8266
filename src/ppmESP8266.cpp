#include "ppmESP8266.h"

#define TIMER_INTERRUPT_DEBUG         0
#define _TIMERINTERRUPT_LOGLEVEL_     0
#define USING_TIM_DIV1                false
#define USING_TIM_DIV16               false
#define USING_TIM_DIV256              true
#include <ESP8266TimerInterrupt.h>
// Init ESP8266 timer 1
ESP8266Timer ITimer;

PPMEncoder ppmEncoder;

// hi
void IRAM_ATTR TimerHandler();


void PPMEncoder::begin(uint8_t pin) {
  begin(pin, PPM_DEFAULT_CHANNELS, false);
}

void PPMEncoder::begin(uint8_t pin, uint8_t ch) {
  begin(pin, ch, false);
}

void PPMEncoder::begin(uint8_t pin, uint8_t ch, boolean inverted) {

  // Store on/off-State in variable to avoid another if in timing-critical interrupt
  onState = (inverted) ? HIGH : LOW;
  offState = (inverted) ? LOW : HIGH;

  pinMode(pin, OUTPUT);
  digitalWrite(pin, offState);

  state = true;
  elapsedUs = 0;
  currentChannel = 0;

  numChannels = ch;
  outputPin = pin;

  for (uint8_t ch = 0; ch < numChannels; ch++) {
    setChannelPercent(ch, 0);
  }

  // start interupt (microsecs)
  ITimer.attachInterruptInterval(20 * 1000, TimerHandler);
  delay(100);

}

void PPMEncoder::setChannel(uint8_t channel, uint16_t value) {
  channels[channel] = constrain(value, PPMEncoder::MIN, PPMEncoder::MAX);
}

void PPMEncoder::setChannelPercent(uint8_t channel, uint8_t percent) {
  percent = constrain(percent, 0, 100);
  setChannel(channel, map(percent, 0, 100, PPMEncoder::MIN, PPMEncoder::MAX));
}

void PPMEncoder::interrupt() {
  if (state) {
    digitalWrite(outputPin, onState);
    unsigned long t = PPM_PULSE_LENGTH_uS;
    ITimer.setInterval(t, TimerHandler);
  } else {
    digitalWrite(outputPin, offState);

    if (currentChannel >= numChannels) {
      elapsedUs = elapsedUs + PPM_PULSE_LENGTH_uS;
      unsigned long t = PPM_FRAME_LENGTH_uS - elapsedUs;
      ITimer.setInterval(t, TimerHandler);
      currentChannel = 0;
      elapsedUs = 0;
    } else {
      unsigned long t = (channels[currentChannel] - PPM_PULSE_LENGTH_uS);
      elapsedUs = elapsedUs + channels[currentChannel];
      ITimer.setInterval(t, TimerHandler);
      currentChannel++;
    }
  }

  state = !state;
}

void IRAM_ATTR TimerHandler() {
  ppmEncoder.interrupt();
}
