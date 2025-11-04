
#include <config.h>
#include <globalstate.h>
#include <leadscrew.h>
#include <spindle.h>


#if ELS_DISPLAY == SSD1306_128_64

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
// some displays can have different addresses, this is what we attempt to init
#define SCREEN_ADDRESS 0x3C

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#elif  ELS_DISPLAY == ST7789_240_135
#include <TFT_eSPI.h>
#include <SPI.h>

#elif  ELS_DISPLAY == ST7789_240_135_LVGL
#include <lvgl.h>
#include <TFT_eSPI.h>
#include <SPI.h>


#else

#error "Please choose a valid display. Refer to config.h for options"

#endif

class Display {
private:
  Spindle* m_spindle;
  Leadscrew* m_leadscrew;
  GlobalState* m_globalState;
  GlobalSystemMode m_systemMode;
#ifdef ELS_UI_ENCODER
  EncoderColour firstColour = EC_NONE;
  EncoderColour secondColour = EC_NONE;
#endif
  bool updating = false;
#if ELS_DISPLAY == ST7789_240_135
  char m_rpmString[10];
  char m_pitchString[10];
  char m_jogString[10];
  GlobalFeedMode m_mode = GlobalFeedMode::FM_UNSET;
  GlobalMotionMode m_motionMode = GlobalMotionMode::MM_UNSET;
  GlobalButtonLock m_locked = GlobalButtonLock::LK_UNSET;
  GlobalThreadSyncState m_sync = GlobalThreadSyncState::SS_UNSET;
#endif
#if ELS_DISPLAY == ST7789_240_135_LVGL
  lv_display_t* disp;
  #define DRAW_BUF_SIZE (TFT_WIDTH * TFT_HEIGHT / 10 * (LV_COLOR_DEPTH / 8))
  uint32_t *draw_buf;
  bool initialised = false;

  lv_obj_t* rpmLabel;
  lv_obj_t* pitchLabel;
  lv_obj_t* endStopLabel;
  lv_obj_t* feedSymbolObj;

#endif
public:
#if ELS_DISPLAY == SSD1306_128_64
  Adafruit_SSD1306 m_ssd1306;
#elif ELS_DISPLAY == ST7789_240_135
  TFT_eSPI tft = TFT_eSPI();
#endif
  Display(Spindle* spindle, Leadscrew* leadscrew) {
    this->m_spindle = spindle;
    this->m_leadscrew = leadscrew;
    this->m_globalState = GlobalState::getInstance();
#if ELS_DISPLAY == SSD1306_128_64
    this->m_ssd1306 =
      Adafruit_SSD1306(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, PIN_DISPLAY_RESET);
#elif ELS_DISPLAY == ST7789_240_135
#endif
  }

  void init();
  void update();

protected:
  void initvars();
  void drawMode();
  void drawPitch();
  void drawEnabled();
  void drawLocked();
  void drawSpindleRpm();
  void drawStopStatus();
  void drawSyncStatus();
  void updateLed();
  void writeLed();
  void drawJogSpeed();
  void drawOTA();
};