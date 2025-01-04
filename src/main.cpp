// Libraries

#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <globalstate.h>
#include <leadscrew.h>
#include <leadscrew_io_impl.h>
#include <spindle.h>
#include <esp_task_wdt.h>

#include "buttons.h"
#include "config.h"
#include "display.h"

#ifndef ESP32
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
                    LEADSCREW_PULSE_DELAY_STEP_US, ELS_LEADSCREW_STEPPER_PPR*ELS_GEARBOX_RATIO,
                    ELS_LEADSCREW_PITCH_MM, ELS_SPINDLE_ENCODER_PPR);
ButtonHandler keyPad(&spindle, &leadscrew);
Display display(&spindle, &leadscrew);

// have to handle the leadscrew updates in a timer callback so we can update the
// screen independently without losing pulses
void timerCallback() {
  spindle.update();
  leadscrew.update();
}


void displayLoop() {
  keyPad.handle();

  static elapsedMicros lastPrint;
  if (lastPrint > 1000 * 500) {
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
  while (true) {
    displayLoop();
    esp_task_wdt_reset();
    vTaskDelay(100);
  }
}

void SpindleTask(void* parameter) {
  while (true) {
    timerCallback();
    esp_task_wdt_reset();
  }
}
#endif

#ifdef ELS_USE_BUTTON_ARRAY
void buttonInterrupt();

void initPad() {

  // Set pad H pins as input
  pinMode(ELS_PAD_H1, INPUT_PULLDOWN);
  pinMode(ELS_PAD_H2, INPUT_PULLDOWN);
  pinMode(ELS_PAD_H3, INPUT_PULLDOWN);

  // Set pad V pins as out, high
  pinMode(ELS_PAD_V1, OUTPUT);
  pinMode(ELS_PAD_V2, OUTPUT);
  pinMode(ELS_PAD_V3, OUTPUT);
  digitalWrite(ELS_PAD_V1, 1);
  digitalWrite(ELS_PAD_V2, 1);
  digitalWrite(ELS_PAD_V3, 1);

  attachInterrupt(digitalPinToInterrupt(ELS_PAD_H1), buttonInterrupt, HIGH);
  attachInterrupt(digitalPinToInterrupt(ELS_PAD_H2), buttonInterrupt, HIGH);
  attachInterrupt(digitalPinToInterrupt(ELS_PAD_H3), buttonInterrupt, HIGH);


}

void buttonInterrupt() {
  // First read the H states
  int a = digitalRead(ELS_PAD_H1) | (digitalRead(ELS_PAD_H2) << 1) | (digitalRead(ELS_PAD_H3) << 2);
  // Now, flip the input to V and set H high
  pinMode(ELS_PAD_V1, INPUT_PULLDOWN);
  pinMode(ELS_PAD_V2, INPUT_PULLDOWN);
  pinMode(ELS_PAD_V3, INPUT_PULLDOWN);
  pinMode(ELS_PAD_H1, OUTPUT);
  pinMode(ELS_PAD_H2, OUTPUT);
  pinMode(ELS_PAD_H3, OUTPUT);
  digitalWrite(ELS_PAD_H1, 1);
  digitalWrite(ELS_PAD_H2, 1);
  digitalWrite(ELS_PAD_H3, 1);
  // Now read the V states
  int b = digitalRead(ELS_PAD_V1) | (digitalRead(ELS_PAD_V2) << 1) | (digitalRead(ELS_PAD_V3) << 2);
  keyPad.keycode = a | b << 3;
  initPad();
}
#endif

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
  pinMode(ELS_SPINDLE_ENCODER_A, INPUT_PULLUP);  // encoder pin 1
  pinMode(ELS_SPINDLE_ENCODER_B, INPUT_PULLUP);  // encoder pin 2
#endif
  pinMode(ELS_LEADSCREW_STEP, OUTPUT);              // step output pin
  pinMode(ELS_LEADSCREW_DIR, OUTPUT);               // direction output pin
  pinMode(ELS_RATE_INCREASE_BUTTON, INPUT_PULLUP);  // rate Inc
  pinMode(ELS_RATE_DECREASE_BUTTON, INPUT_PULLUP);  // rate Dec
  pinMode(ELS_MODE_CYCLE_BUTTON, INPUT_PULLUP);     // mode cycle
  pinMode(ELS_THREAD_SYNC_BUTTON, INPUT_PULLUP);    // thread sync
  pinMode(ELS_HALF_NUT_BUTTON, INPUT_PULLUP);       // half nut
  pinMode(ELS_ENABLE_BUTTON, INPUT_PULLUP);         // enable toggle
  pinMode(ELS_LOCK_BUTTON, INPUT_PULLUP);           // lock toggle
  pinMode(ELS_JOG_LEFT_BUTTON, INPUT_PULLUP);       // jog left
  pinMode(ELS_JOG_RIGHT_BUTTON, INPUT_PULLUP);      // jog right

  // Display Initalisation

  display.init();

  leadscrew.setRatio(globalState->getCurrentFeedPitch());

  display.update();

#ifdef ESP32
  xTaskCreate(SpindleTask, "Spindle", 2048, NULL, 10, NULL);
  xTaskCreate(DisplayTask, "Display", 16192, NULL, 1, NULL);
  disableLoopWDT();
  esp_task_wdt_delete(xTaskGetHandle("IDLE0"));
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