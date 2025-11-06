#include <config.h>

#if ELS_DISPLAY == ST7789_240_135_LVGL
#include <display.h>
#include <globalstate.h>

#define COLOUR_BRIGHTGREEN lv_color_hex(0x00FF00)
#define COLOUR_GREEN lv_color_hex(0x008800)
#define COLOUR_YELLOW lv_color_hex(0x0084FF)
#define COLOUR_RED lv_color_hex(0x0000FF)
#define COLOUR_DISABLED lv_color_hex(0xCCCCCC)

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
  leftStopRectObj = lv_obj_create(lv_screen_active());
  rightStopRectObj = lv_obj_create(lv_screen_active());
  lockedRectObj = lv_obj_create(lv_screen_active());
  enableRectObj = lv_obj_create(lv_screen_active());
  syncRectObj = lv_obj_create(lv_screen_active());
  pitchSlider = lv_slider_create(lv_screen_active());

  lv_obj_set_size(pitchSlider, 280, 10);
  lv_obj_set_pos(pitchSlider, 20, 70);

  feedSymbolObj = lv_image_create(lv_screen_active());
  leftStopObj = lv_image_create(leftStopRectObj);
  rightStopObj = lv_image_create(rightStopRectObj);
  lockedObj = lv_image_create(lockedRectObj);
  enableObj = lv_image_create(enableRectObj);
  syncObj = lv_image_create(syncRectObj);

  lv_obj_set_style_text_font(pitchLabel, &lv_font_montserrat_48, 0);
  lv_obj_set_style_text_font(rpmLabel, &lv_font_montserrat_26, 0);

  lv_label_set_text(pitchLabel, "0.25mm");
  lv_label_set_text(rpmLabel, "3000rpm");

  lv_obj_set_size(leftStopRectObj, 40, 40);
  lv_obj_set_size(rightStopRectObj, 40, 40);
  lv_obj_set_size(lockedRectObj, 40, 40);
  lv_obj_set_size(enableRectObj, 40, 40);
  lv_obj_set_size(syncRectObj, 40, 40);

  lv_obj_set_style_bg_color(leftStopRectObj, (COLOUR_GREEN), 0);
  lv_obj_set_style_bg_color(rightStopRectObj, (COLOUR_DISABLED), 0);
  lv_obj_set_style_bg_color(lockedRectObj, (COLOUR_RED), 0);
  lv_obj_set_style_bg_color(enableRectObj, (COLOUR_DISABLED), 0);
  lv_obj_set_style_bg_color(syncRectObj, (COLOUR_DISABLED), 0);


  lv_obj_set_pos(rpmLabel, 160, 17);
  lv_obj_align(pitchLabel, LV_ALIGN_CENTER, 0, 0);
  lv_obj_set_pos(leftStopRectObj, 10, 10);
  lv_obj_set_pos(rightStopRectObj, 60, 10);
  lv_obj_set_pos(lockedRectObj, 10, 190);
  lv_obj_set_pos(enableRectObj, 60, 190);
  lv_obj_set_pos(syncRectObj, 110, 190);
  lv_obj_set_pos(feedSymbolObj, 170, 166);


  lv_obj_set_style_pad_all(rightStopRectObj, 2, 0);
  lv_obj_set_style_pad_all(leftStopRectObj, 2, 0);
  lv_obj_set_style_pad_all(lockedRectObj, 2, 0);
  lv_obj_set_style_pad_all(enableRectObj, 2, 0);
  lv_obj_set_style_pad_all(syncRectObj, 2, 0);


  lv_image_set_src(feedSymbolObj, &threadSymbol);
  lv_image_set_src(leftStopObj, &leftstop);
  lv_image_set_src(rightStopObj, &rightstop);
  lv_image_set_src(lockedObj, &locked);
  lv_image_set_src(enableObj, &pauseSymbol);
  lv_image_set_src(syncObj, &syncSymbol);


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
    drawMode();
    drawPitch();
    drawLocked();
    drawEnabled();
    drawSpindleRpm();
    drawSyncStatus();
    drawStopStatus();
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
  if (m_leadscrew->getStopPositionState(LeadscrewStopPosition::LEFT) ==
    LeadscrewStopState::SET) {
    lv_obj_set_style_bg_color(leftStopRectObj, COLOUR_GREEN, 0);
  } else {
    lv_obj_set_style_bg_color(leftStopRectObj, COLOUR_DISABLED, 0);
  }
  if (m_leadscrew->getStopPositionState(LeadscrewStopPosition::RIGHT) ==
    LeadscrewStopState::SET) {
    lv_obj_set_style_bg_color(rightStopRectObj, COLOUR_GREEN, 0);
  } else {
    lv_obj_set_style_bg_color(rightStopRectObj, COLOUR_DISABLED, 0);
  }
}

void Display::drawSyncStatus() {
  GlobalThreadSyncState sync = GlobalState::getInstance()->getThreadSyncState();
  if (sync == GlobalThreadSyncState::SS_UNSYNC) {
    lv_obj_set_style_bg_color(syncRectObj, COLOUR_DISABLED, 0);
  } else {
    lv_obj_set_style_bg_color(syncRectObj, COLOUR_GREEN, 0);
  }
}

void Display::drawMode() {
  GlobalFeedMode mode = GlobalState::getInstance()->getFeedMode();
  if (mode == GlobalFeedMode::FM_FEED) {
    lv_image_set_src(feedSymbolObj, &feedSymbol);
  } else if (mode == GlobalFeedMode::FM_THREAD) {
    lv_image_set_src(feedSymbolObj, &threadSymbol);
  }
}

void Display::drawPitch() {
  GlobalState* state = GlobalState::getInstance();
  GlobalUnitMode unit = state->getUnitMode();
  GlobalFeedMode mode = state->getFeedMode();
  GlobalSystemMode sysmode = GlobalState::getInstance()->getSystemMode();

  int feedSelect = state->getFeedSelect();
  char pitch[10];
  if (sysmode == SM_JOG) {
    sprintf(pitch, "%d%s", (int)(state->getJogSpeed() * 100), "%");
    lv_slider_set_min_value(pitchSlider, 0);
    lv_slider_set_max_value(pitchSlider, sizeof(jogSpeeds) / sizeof(int));
    lv_slider_set_value(pitchSlider, state->getJogIndex() + 1, LV_ANIM_OFF);
  } else if (unit == GlobalUnitMode::METRIC) {
    if (mode == GlobalFeedMode::FM_THREAD) {
      sprintf(pitch, "%.2fmm", threadPitchMetric[feedSelect]);

      lv_slider_set_min_value(pitchSlider, 0);
      lv_slider_set_max_value(pitchSlider, sizeof(threadPitchMetric) / sizeof(float));
      lv_slider_set_value(pitchSlider, feedSelect + 1, LV_ANIM_OFF);
    } else {
      sprintf(pitch, "%.2fmm", feedPitchMetric[feedSelect]);

      lv_slider_set_min_value(pitchSlider, 0);
      lv_slider_set_max_value(pitchSlider, sizeof(feedPitchMetric) / sizeof(float));
      lv_slider_set_value(pitchSlider,feedSelect + 1, LV_ANIM_OFF);
    }
  } else {
    if (mode == GlobalFeedMode::FM_THREAD) {
      sprintf(pitch, "%dTPI", (int)threadPitchImperial[feedSelect]);
 
      lv_slider_set_min_value(pitchSlider, 0);
      lv_slider_set_max_value(pitchSlider, sizeof(threadPitchImperial) / sizeof(float));
      lv_slider_set_value(pitchSlider, feedSelect + 1, LV_ANIM_OFF);
    } else {
      sprintf(pitch, "%dth", (int)(feedPitchImperial[feedSelect] * 1000));
 
      lv_slider_set_min_value(pitchSlider, 0);
      lv_slider_set_max_value(pitchSlider, sizeof(feedPitchImperial) / sizeof(float));
      lv_slider_set_value(pitchSlider, feedSelect + 1, LV_ANIM_OFF);
    }
  }

  lv_label_set_text(pitchLabel, pitch);
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
  GlobalState* state = GlobalState::getInstance();
  GlobalMotionMode mode = state->getMotionMode();

  GlobalButtonLock lock = GlobalState::getInstance()->getButtonLock();
  switch (mode) {
  case GlobalMotionMode::MM_DISABLED:
    lv_obj_set_style_bg_color(enableRectObj, COLOUR_DISABLED, 0);
    lv_image_set_src(enableObj, &pauseSymbol);
    break;
  case GlobalMotionMode::MM_DECELLERATE:
    lv_obj_set_style_bg_color(enableRectObj, COLOUR_YELLOW, 0);
    lv_image_set_src(enableObj, &pauseSymbol);
    break;
  case GlobalMotionMode::MM_JOG_LEFT:
  case GlobalMotionMode::MM_INTERACTIVE_JOG_LEFT:
    lv_obj_set_style_bg_color(enableRectObj, COLOUR_YELLOW, 0);
    lv_image_set_src(enableObj, &left);
    break;
  case GlobalMotionMode::MM_JOG_RIGHT:
  case GlobalMotionMode::MM_INTERACTIVE_JOG_RIGHT:
    lv_obj_set_style_bg_color(enableRectObj, COLOUR_YELLOW, 0);
    lv_image_set_src(enableObj, &right);
    break;
  case GlobalMotionMode::MM_ENABLED:
    lv_obj_set_style_bg_color(enableRectObj, COLOUR_GREEN, 0);
    lv_image_set_src(enableObj, &right);
    break;
  }
  updateLed();
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
  GlobalButtonLock lock = GlobalState::getInstance()->getButtonLock();
  switch (lock) {
  case GlobalButtonLock::LK_LOCKED:
    lv_obj_set_style_bg_color(lockedRectObj, COLOUR_RED, 0);
    lv_image_set_src(lockedObj, &locked);
    break;
  case GlobalButtonLock::LK_UNLOCKED:
    lv_obj_set_style_bg_color(lockedRectObj, COLOUR_GREEN, 0);
    lv_image_set_src(lockedObj, &unlocked);
    break;
  }
  updateLed();
}
#endif