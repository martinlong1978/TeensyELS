// Libraries

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <globalstate.h>
#include <leadscrew.h>
#include <leadscrew_io_impl.h>
#include <spindle.h>

#include "CommsManager.h"

#include "buttons.h"
#include "buttonpad.h"
#include "config.h"
#include "display.h"
#include "keyarray.h"

#ifdef ESP32
#include <esp_task_wdt.h>
#else
IntervalTimer timer;
#endif

GlobalState* globalState = GlobalState::getInstance();
#ifdef ELS_SPINDLE_DRIVEN
Spindle spindle;
#else
Spindle spindle(ELS_SPINDLE_ENCODER_A, ELS_SPINDLE_ENCODER_B);
#endif
LeadscrewIOImpl leadscrewIOImpl;
Leadscrew leadscrew(&spindle, &leadscrewIOImpl,
  LEADSCREW_INITIAL_PULSE_DELAY_US,
  LEADSCREW_PULSE_DELAY_STEP_US, ELS_LEADSCREW_STEPPER_PPR* ELS_GEARBOX_RATIO,
  ELS_LEADSCREW_PITCH_MM, ELS_SPINDLE_ENCODER_PPR);

#ifdef ESP32  
KeyArray keyArray(&leadscrew);
ButtonPad keyPad(&spindle, &leadscrew, &keyArray);
CommsManager commsManager;
#else
ButtonHandler keyPad(&spindle, &leadscrew);
#endif
Display display(&spindle, &leadscrew);
int64_t lastcycle;
int cyclecount;

// have to handle the leadscrew updates in a timer callback so we can update the
// screen independently without losing pulses
void timerCallback() {
  cyclecount++;
  int64_t t = esp_timer_get_time();
  if (t - lastcycle > 1000000) {
    Serial.printf("%d\n", cyclecount);
    lastcycle = t;
    cyclecount = 0;
  }
  spindle.update();
  leadscrew.update();
}


void displayLoop() {
  keyPad.handle();

  static elapsedMicros lastPrint;
  if (false) {//lastPrint > 1000 * 500) {
    lastPrint = 0;
    globalState->printState();
    Serial.print("Micros: ");
    Serial.println(micros());
    leadscrew.printState();
    Serial.print("Spindle position: ");
    Serial.println(spindle.getCurrentPosition());
    Serial.print("Spindle unconsumed:");
    Serial.println(spindle.consumePosition());
    Serial.print("Spindle velocity: ");
    Serial.println(spindle.getEstimatedVelocityInRPM());
    Serial.print("Spindle velocity pulses: ");
    Serial.println(spindle.getEstimatedVelocityInPulsesPerSecond());
    keyPad.printState();
  }
  display.update();
}

#ifdef ESP32
void DisplayTask(void* parameter) {
  uint64_t m = 1;
  while (true) {
    displayLoop();
    esp_task_wdt_reset();
    uint64_t c = esp_timer_get_time();
    uint64_t delay = (250000 - (c - m)) / 1000;
    if (delay > 0) {
      vTaskDelay((delay > 250 ? 250 : delay) / portTICK_PERIOD_MS);
    }
    m = c + 250000;
  }
}

void SpindleTask(void* parameter) {
  while (true) {
    timerCallback();
    esp_task_wdt_reset();
  }
}
#endif

void comms_loop(void *parameters) { commsManager.loop(); }

void setup() {

  // config - compile time checks for safety
  CHECK_BOUNDS(DEFAULT_METRIC_THREAD_PITCH_IDX, threadPitchMetric,
    "DEFAULT_METRIC_THREAD_PITCH_IDX out of bounds");
  CHECK_BOUNDS(DEFAULT_METRIC_FEED_PITCH_IDX, feedPitchMetric,
    "DEFAULT_METRIC_FEED_PITCH_IDX out of bounds");
  CHECK_BOUNDS(DEFAULT_IMPERIAL_THREAD_PITCH_IDX, threadPitchImperial,
    "DEFAULT_IMPERIAL_THREAD_PITCH_IDX out of bounds");
  CHECK_BOUNDS(DEFAULT_IMPERIAL_FEED_PITCH_IDX, feedPitchImperial,
    "DEFAULT_IMPERIAL_FEED_PITCH_IDX out of bounds");

  // Pinmodes

#ifndef ELS_SPINDLE_DRIVEN
//  pinMode(ELS_SPINDLE_ENCODER_A, INPUT_PULLUP); // encoder pin 1
//  pinMode(ELS_SPINDLE_ENCODER_B, INPUT_PULLUP); // encoder pin 2
#endif

#ifdef USE_RMT
  rmt_obj_t* leadscreRMT = rmtInit(ELS_LEADSCREW_STEP, true, RMT_MEM_64);
  leadscrew.setRMT(leadscreRMT);
  rmtSetTick(leadscreRMT, 2500);

#else
  pinMode(ELS_LEADSCREW_STEP, OUTPUT); // step output pin
#endif
  pinMode(ELS_LEADSCREW_DIR, OUTPUT);  // direction output pin

#ifdef ELS_UI_ENCODER
  //  pinMode(ELS_UI_ENCODER_A, INPUT); // encoder pin 1
  //  pinMode(ELS_UI_ENCODER_B, INPUT); // encoder pin 2

#ifdef ELS_IND_GREEN
  pinMode(ELS_IND_GREEN, OUTPUT);
  pinMode(ELS_IND_RED, OUTPUT);
#endif
#endif

  pinMode(ELS_STEPPER_ENA, OUTPUT);
  digitalWrite(ELS_STEPPER_ENA, 0);

#ifdef ELS_USE_BUTTON_ARRAY
  keyArray.initPad();
#else
  pinMode(ELS_RATE_INCREASE_BUTTON, INPUT_PULLUP);  // rate Inc
  pinMode(ELS_RATE_DECREASE_BUTTON, INPUT_PULLUP);  // rate Dec
  pinMode(ELS_MODE_CYCLE_BUTTON, INPUT_PULLUP);     // mode cycle
  pinMode(ELS_THREAD_SYNC_BUTTON, INPUT_PULLUP);    // thread sync
  pinMode(ELS_HALF_NUT_BUTTON, INPUT_PULLUP);       // half nut
  pinMode(ELS_ENABLE_BUTTON, INPUT_PULLUP);         // enable toggle
  pinMode(ELS_LOCK_BUTTON, INPUT_PULLUP);           // lock toggle
  pinMode(ELS_JOG_LEFT_BUTTON, INPUT_PULLUP);       // jog left
  pinMode(ELS_JOG_RIGHT_BUTTON, INPUT_PULLUP);      // jog right
#endif

  // Display Initalisation

  display.init();

  leadscrew.setRatio(globalState->getCurrentFeedPitch());

  display.update();

#ifdef ESP32
  commsManager.setup();

  TaskHandle_t spindleTask;
  TaskHandle_t displayTask;
  TaskHandle_t commsTask;
  xTaskCreate(SpindleTask, "Spindle", 2048, NULL, 10, &spindleTask);
  xTaskCreate(DisplayTask, "Display", 8000, NULL, 1, &displayTask);
  xTaskCreate(comms_loop, "Comms", 16000, NULL, 1, &commsTask);
  disableLoopWDT();
  esp_task_wdt_delete(xTaskGetHandle("IDLE0"));
  esp_task_wdt_delete(xTaskGetHandle("IDLE1"));
  esp_task_wdt_delete(spindleTask);
  esp_task_wdt_delete(displayTask);
  esp_task_wdt_delete(commsTask);

#else
  timer.begin(timerCallback, LEADSCREW_TIMER_US);
#endif

  delay(2000);

  Serial.print("Initial pulse delay: ");
  Serial.println(LEADSCREW_INITIAL_PULSE_DELAY_US);
  Serial.print("Pulse delay step: ");
  Serial.println(LEADSCREW_PULSE_DELAY_STEP_US);
}

void loop() {
#ifdef ESP32  
  vTaskDelay(1000);
#else
  displayLoop();
#endif
}