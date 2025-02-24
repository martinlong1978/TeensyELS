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
  Serial.begin(921600);
  ESP32Encoder::useInternalWeakPullResistors = puType::none;
  m_encoder.attachFullQuad(pinA, pinB);
  gpio_pullup_en((gpio_num_t)pinA);
  gpio_pullup_en((gpio_num_t)pinB);

#endif
  m_unconsumedPosition = 0;
#ifdef ESP32
  m_lastPulseTimestamp = esp_timer_get_time();
#else
  m_lastPulseMicros = 0;
#endif
  m_lastFullPulseDurationMicros = 0;
  m_currentPosition = 0;
}

void Spindle::update() {
  // read the encoder and update the current position
  // todo: we should keep the absolute position of the spindle, cbf right now
#ifdef ESP32
  int64_t position = m_encoder.getAndClearCount();
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
#ifdef ESP32
  int64_t t = esp_timer_get_time();
  int pos = getCurrentPosition() + amount;
  setCurrentPosition(pos);
  int newpos = getCurrentPosition();
  if (pos != newpos) // spindle pos has wrapped
  {
    m_lastRevPosition -= pos - newpos;
  }
  if (amount != 0) {
    m_lastPulseTimestamp = t;
    if (abs(newpos - m_lastRevPosition) > ELS_SPINDLE_ENCODER_PPR) {
      // Update stats for last full revolution. 
      m_lastRevSize = newpos - m_lastRevPosition;
      m_lastRevPosition = newpos;
      m_lastRevMicros = t - m_lastRevTimestamp;
      m_lastRevTimestamp = t;
    }
  }
#else
  setCurrentPosition(getCurrentPosition() + amount);
  if (amount != 0) {
    m_lastFullPulseDurationMicros = m_lastPulseMicros / abs(amount);
    m_lastPulseMicros = 0;
  }
#endif
}

float Spindle::getEstimatedVelocityInRPM() {
#ifdef ESP32
  if (m_lastRevMicros == 0)return 0;
  if(esp_timer_get_time() - m_lastRevTimestamp > 1000000)return 0;
  return abs((m_lastRevSize * 60000000) / (m_lastRevMicros * ELS_SPINDLE_ENCODER_PPR));
#else
  return (getEstimatedVelocityInPulsesPerSecond() * 60) / ELS_SPINDLE_ENCODER_PPR;
#endif
}

int Spindle::consumePosition() {
  int position = m_unconsumedPosition;
  m_unconsumedPosition = 0;
  return position;
}