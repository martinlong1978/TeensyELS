#include <config.h>
#include <display.h>
#include <globalstate.h>

// Images
#include <icons/feedSymbol.h>
#include <icons/lockedSymbol.h>
#include <icons/pauseSymbol.h>
#include <icons/runSymbol.h>
#include <icons/threadSymbol.h>
#include <icons/unlockedSymbol.h>

// Add some basic bitmap scaling for double-resolution screens.
uint8_t spread(uint8_t in) {
  uint8_t a = (in & 0x1) | ((in & 0x2) << 1) | ((in & 0x4) << 2) | ((in & 0x8) << 3);
  return a | (a << 1);
}

void shiftByte(const uint8_t source[], uint8_t dest[], int sidx, int didx) {
  uint8_t s = source[sidx];
  uint8_t a = (s & 0xF0) >> 4;
  uint8_t b = s & 0x0F;
  dest[didx] = spread(a);
  dest[didx + 1] = spread(b);
}

void fillDest(const uint8_t source[], uint8_t dest[], int sidx, int didx, int bytes) {
  for (int i = 0; i < bytes; i++) {
    shiftByte(source, dest, sidx + i, didx + (i * 2));
  }
}

void ScaleBMP(const uint8_t source[], uint8_t dest[], int sizex, int sizey) {
  int bytesx = sizex / 8;
  for (int i = 0; i < sizey; i++) {
    fillDest(source, dest, i * bytesx, i * bytesx * 4, bytesx);
    fillDest(source, dest, i * bytesx, (bytesx * 2) + (i * bytesx * 4), bytesx);
  }
}

void Display::init() {
#if ELS_DISPLAY == SSD1306_128_64
  if (!this->m_ssd1306.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;);
  }
  m_ssd1306.clearDisplay();
#elif ELS_DISPLAY == ST7789_240_135
  tft.init();
  tft.setRotation(1);
  tft.fillScreen(TFT_BLACK);
#endif
}

void Display::update() {
#if ELS_DISPLAY == SSD1306_128_64
  m_ssd1306.clearDisplay();
#elif ELS_DISPLAY == ST7789_240_135
  //  tft.fillScreen(TFT_BLACK); // Rely on localised blanking to avoid blink, for now.
#endif

  drawMode();
  drawPitch();
  drawLocked();
  drawEnabled();
  drawSpindleRpm();

  drawStopStatus();

#if ELS_DISPLAY == SSD1306_128_64
  m_ssd1306.display();
#endif
}

void Display::drawSpindleRpm() {
  int rpm = m_spindle->getEstimatedVelocityInRPM();
  char rpmString[10];
  sprintf(rpmString, "%4dRPM", rpm);
#if ELS_DISPLAY == SSD1306_128_64
  m_ssd1306.setCursor(0, 0);
  m_ssd1306.setTextSize(1);
  m_ssd1306.setTextColor(WHITE);
  // pad the rpm with spaces so the RPM text stays in the same place
  m_ssd1306.print(rpmString);
#elif ELS_DISPLAY == ST7789_240_135
  // pad the rpm with spaces so the RPM text stays in the same place
  if (strcmp(rpmString, m_rpmString)) {
    tft.setCursor(0, 0);
    tft.setTextSize(2);
    tft.setTextColor(TFT_WHITE);
    tft.fillRect(0, 0, 85, 16, TFT_BLACK);
    tft.print(rpmString);
    strcpy(m_rpmString, rpmString);
  }
#endif
}

void Display::drawStopStatus() {
#if ELS_DISPLAY == SSD1306_128_64
  m_ssd1306.setCursor(0, 8);
  m_ssd1306.setTextSize(1);
  m_ssd1306.setTextColor(WHITE);
  if (m_leadscrew->getStopPositionState(LeadscrewStopPosition::LEFT) ==
    LeadscrewStopState::SET) {
    m_ssd1306.print("[");
  } else {
    m_ssd1306.print(" ");
  }
  if (m_leadscrew->getStopPositionState(LeadscrewStopPosition::RIGHT) ==
    LeadscrewStopState::SET) {
    m_ssd1306.print("]");
  } else {
    m_ssd1306.print(" ");
  }
#elif ELS_DISPLAY == ST7789_240_135
  tft.setCursor(0, 16);
  tft.setTextSize(2);
  tft.setTextColor(TFT_WHITE);
  if (m_leadscrew->getStopPositionState(LeadscrewStopPosition::LEFT) ==
    LeadscrewStopState::SET) {
    tft.print("[");
  } else {
    tft.print(" ");
  }
  if (m_leadscrew->getStopPositionState(LeadscrewStopPosition::RIGHT) ==
    LeadscrewStopState::SET) {
    tft.print("]");
  } else {
    tft.print(" ");
  }
#endif
}

void Display::drawSyncStatus() {
  GlobalThreadSyncState sync = GlobalState::getInstance()->getThreadSyncState();
#if ELS_DISPLAY == SSD1306_128_64
  m_ssd1306.setCursor(0, 16);
  m_ssd1306.setTextSize(2);
  m_ssd1306.setTextColor(WHITE);
  m_ssd1306.print("SYNC");
  // cross it out if not synced
  if (sync == GlobalThreadSyncState::UNSYNC) {
    m_ssd1306.drawLine(0, 16, 64, 16, WHITE);
  }
#elif ELS_DISPLAY == ST7789_240_135
  tft.setCursor(0, 32);
  tft.setTextSize(4);
  tft.setTextColor(TFT_WHITE);
  tft.print("SYNC");
  // cross it out if not synced
  if (sync == GlobalThreadSyncState::UNSYNC) {
    tft.drawLine(0, 32, 128, 32, TFT_WHITE);
  }
#endif
}

void Display::drawMode() {
  GlobalFeedMode mode = GlobalState::getInstance()->getFeedMode();

#if ELS_DISPLAY == SSD1306_128_64
  if (mode == GlobalFeedMode::FEED) {
    m_ssd1306.drawBitmap(57, 32, feedSymbol, 64, 32, WHITE);
  } else if (mode == GlobalFeedMode::THREAD) {
    m_ssd1306.drawBitmap(57, 32, threadSymbol, 64, 32, WHITE);
  }
#elif ELS_DISPLAY == ST7789_240_135
  if (mode == m_mode)return;
  m_mode = mode;
  tft.fillRect(104, 64, 128, 64, TFT_BLACK);
  if (mode == GlobalFeedMode::FM_FEED) {
    uint8_t scaled[128 * 64 / 2];
    ScaleBMP(feedSymbol, scaled, 128, 64);
    tft.drawBitmap(104, 64, scaled, 128, 64, TFT_WHITE);
  } else if (mode == GlobalFeedMode::FM_THREAD) {
    uint8_t scaled[128];
    ScaleBMP(threadSymbol, scaled, 128, 64);
    tft.drawBitmap(104, 64, scaled, 128, 64, TFT_WHITE);
  }
#endif
}

void Display::drawPitch() {
  GlobalState* state = GlobalState::getInstance();
  GlobalUnitMode unit = state->getUnitMode();
  GlobalFeedMode mode = state->getFeedMode();
  int feedSelect = state->getFeedSelect();
  char pitch[10];
  if (unit == GlobalUnitMode::METRIC) {
    if (mode == GlobalFeedMode::FM_THREAD) {
      sprintf(pitch, "%.2fmm", threadPitchMetric[feedSelect]);
    } else {
      sprintf(pitch, "%.2fmm", feedPitchMetric[feedSelect]);
    }
  } else {
    if (mode == GlobalFeedMode::FM_THREAD) {
      sprintf(pitch, "%dTPI", (int)threadPitchImperial[feedSelect]);
    } else {
      sprintf(pitch, "%dth", (int)(feedPitchImperial[feedSelect] * 1000));
    }
  }

#if ELS_DISPLAY == SSD1306_128_64
  m_ssd1306.setCursor(55, 8);
  m_ssd1306.setTextSize(2);
  m_ssd1306.setTextColor(WHITE);
  m_ssd1306.print(pitch);
#elif ELS_DISPLAY == ST7789_240_135
  if (!strcmp(pitch, m_pitchString))return;
  strcpy(m_pitchString, pitch);
  tft.fillRect(110, 16, 130, 20, TFT_BLACK);
  tft.setCursor(110, 16);
  tft.setTextSize(3);
  tft.setTextColor(TFT_WHITE);
  tft.print(pitch);
#endif
}

void Display::drawEnabled() {
  GlobalState* state = GlobalState::getInstance();
  GlobalMotionMode mode = state->getMotionMode();

#if ELS_DISPLAY == SSD1306_128_64
  m_ssd1306.fillRoundRect(26, 40, 20, 20, 2, WHITE);
  switch (mode) {
  case GlobalMotionMode::S_DISABLED:
    m_ssd1306.drawBitmap(28, 42, pauseSymbol, 16, 16, BLACK);
    break;
  case GlobalMotionMode::JOG:
    // todo bitmap for jogging
    m_ssd1306.setCursor(28, 42);
    m_ssd1306.setTextSize(2);
    m_ssd1306.setTextColor(BLACK);
    m_ssd1306.print("J");
    break;
  case GlobalMotionMode::ENABLED:
    m_ssd1306.drawBitmap(28, 42, runSymbol, 16, 16, BLACK);
    break;
  }
#elif ELS_DISPLAY == ST7789_240_135
  if (mode == m_motionMode)return;
  m_motionMode = mode;
  tft.fillRoundRect(52, 80, 40, 40, 4, TFT_WHITE);
  uint8_t scaled[128];
  switch (mode) {
  case GlobalMotionMode::MM_DISABLED:
    ScaleBMP(pauseSymbol, scaled, 16, 16);
    tft.drawBitmap(56, 84, scaled, 32, 32, TFT_BLACK);
    break;
  case GlobalMotionMode::MM_JOG:
    // todo bitmap for jogging
    tft.setCursor(56, 84);
    tft.setTextSize(4);
    tft.setTextColor(TFT_BLACK);
    tft.print("J");
    break;
  case GlobalMotionMode::MM_ENABLED:
    ScaleBMP(runSymbol, scaled, 16, 16);
    tft.drawBitmap(56, 84, scaled, 32, 32, TFT_BLACK);
    break;
  }
#endif
}

void Display::drawLocked() {
  GlobalButtonLock lock = GlobalState::getInstance()->getButtonLock();
#if ELS_DISPLAY == SSD1306_128_64
  m_ssd1306.fillRoundRect(2, 40, 20, 20, 2, WHITE);
  switch (lock) {
  case GlobalButtonLock::LOCKED:
    m_ssd1306.drawBitmap(4, 42, lockedSymbol, 16, 16, BLACK);
    break;
  case GlobalButtonLock::UNLOCKED:
    m_ssd1306.drawBitmap(4, 42, unlockedSymbol, 16, 16, BLACK);
    break;
  }
#elif ELS_DISPLAY == ST7789_240_135
  if (lock == m_locked)return;
  m_locked = lock;

  tft.fillRoundRect(4, 80, 40, 40, 4, TFT_WHITE);
  uint8_t scaled[128];
  switch (lock) {
  case GlobalButtonLock::LK_LOCKED:
    ScaleBMP(lockedSymbol, scaled, 16, 16);
    tft.drawBitmap(8, 84, scaled, 32, 32, TFT_BLACK);
    break;
  case GlobalButtonLock::LK_UNLOCKED:
    ScaleBMP(unlockedSymbol, scaled, 16, 16);
    tft.drawBitmap(8, 84, scaled, 32, 32, TFT_BLACK);
    break;
  }
#endif
}
