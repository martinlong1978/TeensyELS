#include "spindle.h"

#include <config.h>
#include <els_elapsedMillis.h>
#include <math.h>

#ifndef ELS_SPINDLE_DRIVEN
#ifdef ESP32
Spindle::Spindle(int pinA, int pinB) : m_encoder() {
#else
Spindle::Spindle(int pinA, int pinB) : m_encoder(pinA, pinB) {
#endif
#else
Spindle::Spindle() {
#endif

#ifdef ESP32
  Serial.begin(115200);
  ESP32Encoder::useInternalWeakPullResistors = puType::none;
  m_encoder.attachFullQuad(pinA, pinB);
  gpio_pullup_en((gpio_num_t)pinA);
  gpio_pullup_en((gpio_num_t)pinB);

#endif
  m_unconsumedPosition = 0;
  m_lastPulseMicros = 0;
  m_lastFullPulseDurationMicros = 0;
  m_currentPosition = 0;
}

void Spindle::update() {
  // read the encoder and update the current position
  // todo: we should keep the absolute position of the spindle, cbf right now
#ifdef ESP32
  int position = m_encoder.getCount();
  //Serial.printf("Enc: %d\n", position);
  m_encoder.clearCount();
  incrementCurrentPosition(position);
#else
  int position = m_encoder.read();
  incrementCurrentPosition(position);
  m_encoder.write(0);
#endif
}

void Spindle::setCurrentPosition(int position) {
  // update the unconsumed position by finding the delta between the old and new
  // positions
  int positionDelta = position - m_currentPosition;
  m_unconsumedPosition += positionDelta;

  m_currentPosition = position % ELS_SPINDLE_ENCODER_PPR;
}

void Spindle::incrementCurrentPosition(int amount) {
  setCurrentPosition(getCurrentPosition() + amount);
  if (amount != 0) {
    m_lastFullPulseDurationMicros = m_lastPulseMicros / abs(amount);
    m_lastPulseMicros = 0;
  }
}

float Spindle::getEstimatedVelocityInRPM() {
  return getEstimatedVelocityInPulsesPerSecond() / ELS_SPINDLE_ENCODER_PPR;
}

int Spindle::consumePosition() {
  int position = m_unconsumedPosition;
  m_unconsumedPosition = 0;
  return position;
}