#ifndef KEYARRAY_H
#define KEYARRAY_H
#include <leadscrew.h>
#include <spindle.h>

void buttonInterrupt();
void IRAM_ATTR timerInterrupt();

enum ButtonState { BS_NONE = 0, BS_PRESSED = 1, BS_CLICKED = 2,  BS_HELD = 3, BS_RELEASED = 4, BS_DOUBLE_CLICKED = 5 };

typedef struct buttonInfo {
    int button;
    int buttonState;
} ButtonInfo;

class KeyArray {
 private:
    volatile ButtonInfo buttonState;
    volatile unsigned long keycodeMillis;
    hw_timer_t *Timer0_Cfg;
    void setupKeys();
public: 
    void initPad();
    void handle();
    void handleTimer();
    ButtonInfo consumeButton();
};

extern KeyArray keyArray;
#endif