#include <config.h>

#if ELS_DISPLAY == ST7789_240_135_LVGL
#include <display.h>
#include <globalstate.h>
// Images
//#include <icons/feedSymbol.h>
#include <icons/lockedSymbol.h>
#include <icons/pauseSymbol.h>
#include <icons/runSymbol.h>
#include <icons/threadSymbol.h>
#include <icons/unlockedSymbol.h>
#include <icons/right.h>
#include <icons/left.h>

static uint32_t my_tick(void) {
  return millis();
}

void Display::initvars() {

}

void Display::init() {
  // initvars();
  // tft.init();
  // tft.setRotation(3);
  // tft.fillScreen(TFT_BLACK);

  if (!initialised) {
    draw_buf = (uint32_t*)malloc(DRAW_BUF_SIZE);
    initialised = true;
  }

  lv_init();
  lv_tick_set_cb(my_tick);
  disp = lv_tft_espi_create(TFT_WIDTH, TFT_HEIGHT, draw_buf, DRAW_BUF_SIZE);
  lv_display_set_rotation(disp, LV_DISPLAY_ROTATION_90);

  pitchLabel = lv_label_create(lv_screen_active());
  rpmLabel = lv_label_create(lv_screen_active());
  endStopLabel = lv_label_create(lv_screen_active());

  lv_obj_set_style_text_font(pitchLabel, &lv_font_montserrat_36, 0);
  lv_obj_set_style_text_font(rpmLabel, &lv_font_montserrat_26, 0);
  lv_obj_set_style_text_font(endStopLabel, &lv_font_montserrat_26, 0);

  lv_label_set_text(pitchLabel, "0.25mm");
  lv_label_set_text(rpmLabel, "3000rpm");
  lv_label_set_text(endStopLabel, "[]");

  lv_obj_set_pos(rpmLabel, 160, 0);
  lv_obj_set_pos(pitchLabel, 160, 100);
  lv_obj_set_pos(endStopLabel, 0, 0);

  LV_IMAGE_DECLARE(feedSymbol);
  feedSymbolObj = lv_image_create(lv_screen_active());
  lv_image_set_src(feedSymbolObj, &feedSymbol);
  lv_obj_set_pos(feedSymbolObj, 160, 150);
  lv_obj_set_style_bg_color(feedSymbolObj, lv_color_hex(0x000000), 0);
  //lv_label_set_text(label, "Hello Arduino");
  //lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); 

  //lv_obj_t* label2 = lv_label_create(lv_screen_active());
  //lv_label_set_text(label2, "Hello again");
  //lv_obj_align(label2, LV_ALIGN_CENTER, 0, 0);
}

void Display::update() {
  //  tft.fillScreen(TFT_BLACK); // Rely on localised blanking to avoid blink, for now.
  if (GlobalState::getInstance()->getDisplayReset()) {
    init();
  }
  lv_timer_handler();

  int bytes = GlobalState::getInstance()->getOTABytes();
  int length = GlobalState::getInstance()->getOTALength();
  if (GlobalState::getInstance()->hasOTA()) {
    drawOTA();

  } else {
    GlobalSystemMode mode = GlobalState::getInstance()->getSystemMode();
    if (mode != m_systemMode) {
      initvars();
      //tft.fillScreen(TFT_BLACK);
      m_systemMode = mode;
    }
    switch (mode) {
    case SM_NORMAL:
      drawMode();
      drawPitch();
      drawLocked();
      drawEnabled();
      drawSpindleRpm();
      drawSyncStatus();
      drawStopStatus();
      break;
    case SM_JOG:
      drawLocked();
      drawEnabled();
      drawJogSpeed();
      break;
    }
  }
#ifdef ESP32
  writeLed();
#endif
}

void Display::drawOTA() {
  //   if (!updating) {
  //   tft.fillRect(0, 0, 240, 135, TFT_BLACK);
  //   tft.setCursor(10, 10);
  //   tft.setTextSize(3);
  //   tft.setTextColor(TFT_WHITE);
  //   tft.print("UPDATING");
  //   updating = true;
  // }
  // tft.drawRect(0, 70, 240, 40, TFT_WHITE);
  // int percent = (((float)(bytes * 240)) / ((float)length));
  // if (bytes > 0)tft.fillRect(0, 70, percent, 40, TFT_WHITE);

}

void Display::drawSpindleRpm() {
  int rrpm = m_spindle->getEstimatedVelocityInRPM();
  int rpm = abs(rrpm);
  char rpmString[10];
  sprintf(rpmString, "%4dRPM", rpm);
  lv_label_set_text(rpmLabel, rpmString);
  lv_obj_set_style_text_color(rpmLabel, lv_color_hex(rpm < 0 ? 0xFF0000 : 0x000000), 0);
}

void Display::drawStopStatus() {
  // tft.setCursor(0, 0);
  // tft.setTextSize(3);
  // tft.setTextColor(TFT_WHITE);
  // tft.fillRect(0, 0, 100, 32, TFT_BLACK);
  // if (m_leadscrew->getStopPositionState(LeadscrewStopPosition::LEFT) ==
  //   LeadscrewStopState::SET) {
  //   tft.print("[");
  // } else {
  //   tft.print(" ");
  // }
  // if (m_leadscrew->getStopPositionState(LeadscrewStopPosition::RIGHT) ==
  //   LeadscrewStopState::SET) {
  //   tft.print("]");
  // } else {
  //   tft.print(" ");
  // }
}

void Display::drawSyncStatus() {
  // GlobalThreadSyncState sync = GlobalState::getInstance()->getThreadSyncState();
  // if (sync == m_sync)return;
  // m_sync = sync;
  // if (sync == GlobalThreadSyncState::SS_UNSYNC) {
  //   tft.drawLine(0, 32, 70, 48, TFT_RED);
  //   tft.setCursor(0, 32);
  //   tft.setTextSize(3);
  //   tft.setTextColor(TFT_RED);
  //   tft.print("SYNC");
  // } else {
  //   tft.drawLine(0, 32, 70, 48, TFT_BLACK);
  //   tft.setCursor(0, 32);
  //   tft.setTextSize(3);
  //   tft.setTextColor(TFT_WHITE);
  //   tft.print("SYNC");
  // }
}

void Display::drawMode() {
  // GlobalFeedMode mode = GlobalState::getInstance()->getFeedMode();

  // if (mode == m_mode)return;
  // m_mode = mode;
  // tft.fillRect(104, 64, 128, 64, TFT_BLACK);
  // if (mode == GlobalFeedMode::FM_FEED) {
  //   uint8_t scaled[128 * 64 / 2];
  //   ScaleBMP(feedSymbol, scaled, 128, 64);
  //   tft.drawBitmap(104, 64, scaled, 128, 64, TFT_WHITE);
  // } else if (mode == GlobalFeedMode::FM_THREAD) {
  //   uint8_t scaled[128 * 64 / 2];
  //   ScaleBMP(threadSymbol, scaled, 128, 64);
  //   tft.drawBitmap(104, 64, scaled, 128, 64, TFT_WHITE);
  // }
}

void Display::drawPitch() {
  // GlobalState* state = GlobalState::getInstance();
  // GlobalUnitMode unit = state->getUnitMode();
  // GlobalFeedMode mode = state->getFeedMode();
  // int feedSelect = state->getFeedSelect();
  // char pitch[10];
  // if (unit == GlobalUnitMode::METRIC) {
  //   if (mode == GlobalFeedMode::FM_THREAD) {
  //     sprintf(pitch, "%.2fmm", threadPitchMetric[feedSelect]);
  //   } else {
  //     sprintf(pitch, "%.2fmm", feedPitchMetric[feedSelect]);
  //   }
  // } else {
  //   if (mode == GlobalFeedMode::FM_THREAD) {
  //     sprintf(pitch, "%dTPI", (int)threadPitchImperial[feedSelect]);
  //   } else {
  //     sprintf(pitch, "%dth", (int)(feedPitchImperial[feedSelect] * 1000));
  //   }
  // }

  // if (!strcmp(pitch, m_pitchString))return;
  // strcpy(m_pitchString, pitch);
  // tft.fillRect(110, 32, 130, 21, TFT_BLACK);
  // tft.setCursor(110, 32);
  // tft.setTextSize(3);
  // tft.setTextColor(TFT_WHITE);
  // tft.print(pitch);
}


void Display::drawJogSpeed() {
  // GlobalState* state = GlobalState::getInstance();
  // char pitch[10];
  // sprintf(pitch, "%d%s", (int)(state->getJogSpeed() * 100), "%");

  // if (!strcmp(pitch, m_jogString))return;
  // strcpy(m_jogString, pitch);
  // tft.fillRect(110, 22, 140, 40, TFT_BLACK);
  // tft.setCursor(110, 32);
  // tft.setTextSize(4);
  // tft.setTextColor(TFT_WHITE);
  // tft.print(pitch);
}

void Display::drawEnabled() {
  // GlobalState* state = GlobalState::getInstance();
  // GlobalMotionMode mode = state->getMotionMode();

  // if (mode == m_motionMode)return;
  // m_motionMode = mode;
  // uint8_t scaled[128];
  // GlobalButtonLock lock = GlobalState::getInstance()->getButtonLock();
  // switch (mode) {
  // case GlobalMotionMode::MM_DISABLED:
  // case GlobalMotionMode::MM_DECELLERATE:
  //   tft.fillRoundRect(52, 95, 40, 40, 4, TFT_WHITE);
  //   ScaleBMP(pauseSymbol, scaled, 16, 16);
  //   tft.drawBitmap(56, 99, scaled, 32, 32, TFT_BLACK);
  //   break;
  // case GlobalMotionMode::MM_JOG_LEFT:
  // case GlobalMotionMode::MM_INTERACTIVE_JOG_LEFT:
  //   tft.fillRoundRect(52, 95, 40, 40, 4, TFT_YELLOW);
  //   ScaleBMP(left, scaled, 16, 16);
  //   tft.drawBitmap(56, 99, scaled, 32, 32, TFT_BLACK);
  //   break;
  // case GlobalMotionMode::MM_JOG_RIGHT:
  // case GlobalMotionMode::MM_INTERACTIVE_JOG_RIGHT:
  //   tft.fillRoundRect(52, 95, 40, 40, 4, TFT_YELLOW);
  //   ScaleBMP(right, scaled, 16, 16);
  //   tft.drawBitmap(56, 99, scaled, 32, 32, TFT_BLACK);

  //   break;
  // case GlobalMotionMode::MM_ENABLED:
  //   tft.fillRoundRect(52, 95, 40, 40, 4, TFT_GREEN);
  //   ScaleBMP(runSymbol, scaled, 16, 16);
  //   tft.drawBitmap(56, 99, scaled, 32, 32, TFT_BLACK);
  //   break;
  // }
  // updateLed();
}

#ifdef ESP32   // TODO Make portable
void Display::writeLed() {
#ifdef ELS_UI_ENCODER
  int64_t time = micros() / 250000;
  EncoderColour c = time % 2 == 1 ? firstColour : secondColour;
  digitalWrite(ELS_IND_GREEN, (c & 2) == 2);
  digitalWrite(ELS_IND_RED, c & 1);
#endif
}
#endif


void Display::updateLed() {
#ifdef ELS_UI_ENCODER

  GlobalState* state = GlobalState::getInstance();
  GlobalMotionMode mode = state->getMotionMode();
  GlobalButtonLock lock = GlobalState::getInstance()->getButtonLock();

  switch (mode) {
  case GlobalMotionMode::MM_DISABLED:
    firstColour = lock == LK_LOCKED ? EC_RED : EC_NONE;
    secondColour = lock == LK_LOCKED ? EC_RED : EC_NONE;
    break;
  case GlobalMotionMode::MM_JOG_LEFT:
  case GlobalMotionMode::MM_JOG_RIGHT:
    firstColour = EC_YELLOW;
    secondColour = EC_YELLOW;
    break;
  case GlobalMotionMode::MM_ENABLED:
    firstColour = lock == LK_LOCKED ? EC_RED : EC_GREEN;
    secondColour = EC_GREEN;
    break;
  }
#endif

}

void Display::drawLocked() {
  // GlobalButtonLock lock = GlobalState::getInstance()->getButtonLock();
  // if (lock == m_locked)return;
  // m_locked = lock;

  // tft.fillRoundRect(0, 95, 40, 40, 4, lock == LK_LOCKED ? TFT_RED : TFT_GREEN);
  // uint8_t scaled[128];
  // switch (lock) {
  // case GlobalButtonLock::LK_LOCKED:
  //   ScaleBMP(lockedSymbol, scaled, 16, 16);
  //   tft.drawBitmap(4, 99, scaled, 32, 32, TFT_BLACK);
  //   break;
  // case GlobalButtonLock::LK_UNLOCKED:
  //   ScaleBMP(unlockedSymbol, scaled, 16, 16);
  //   tft.drawBitmap(4, 99, scaled, 32, 32, TFT_BLACK);
  //   break;
  // }
  // updateLed();
}
#endif