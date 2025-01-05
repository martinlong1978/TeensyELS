#include "buttonpad.h"

#include <config.h>
#include <globalstate.h>

ButtonPad::ButtonPad(Spindle* spindle, Leadscrew* leadscrew, KeyArray* pad)
  : m_spindle(spindle),
  m_leadscrew(leadscrew),
  m_pad(pad) {
}

void ButtonPad::handle() {

  ButtonInfo press = m_pad->consumeButton();
  switch (press.button) {
  case ELS_RATE_INCREASE_BUTTON:
    rateIncreaseHandler(press);
    break;
  case ELS_RATE_DECREASE_BUTTON:
    rateDecreaseHandler(press);
    break;
  case ELS_MODE_CYCLE_BUTTON:
    modeCycleHandler(press);
    break;
  case ELS_THREAD_SYNC_BUTTON:
    threadSyncHandler(press);
    break;
  case ELS_HALF_NUT_BUTTON:
    halfNutHandler(press);
    break;
  case ELS_ENABLE_BUTTON:
    enableHandler(press);
    break;
  case ELS_LOCK_BUTTON:
    lockHandler(press);
    break;
  case ELS_JOG_LEFT_BUTTON:
  case ELS_JOG_RIGHT_BUTTON:
    jogHandler(press);
    break;
  default:
    break;
  }
}

void ButtonPad::rateIncreaseHandler(ButtonInfo press) {

  GlobalButtonLock lockState = GlobalState::getInstance()->getButtonLock();
  if (lockState == GlobalButtonLock::LK_LOCKED) {
    Serial.println("Locked, ingoring rat inc");
    return;
  }

  if (press.buttonState == BS_CLICKED) {
    GlobalState::getInstance()->nextFeedPitch();
    m_leadscrew->setRatio(GlobalState::getInstance()->getCurrentFeedPitch());
  }
}

void ButtonPad::rateDecreaseHandler(ButtonInfo press) {

  GlobalButtonLock lockState = GlobalState::getInstance()->getButtonLock();
  if (lockState == GlobalButtonLock::LK_LOCKED) {
    Serial.println("Locked, ingoring rat dec");
    return;
  }

  if (press.buttonState == BS_CLICKED) {
    GlobalState::getInstance()->prevFeedPitch();
    m_leadscrew->setRatio(GlobalState::getInstance()->getCurrentFeedPitch());
  }
}

void ButtonPad::halfNutHandler(ButtonInfo press) {

  GlobalButtonLock lockState = GlobalState::getInstance()->getButtonLock();
  if (lockState == GlobalButtonLock::LK_LOCKED) {
    Serial.println("Locked, ingoring halfnut");
    return;
  }

  // honestly I don't know what this button should do after the refactor...

  /*if (event == Button::SINGLE_CLICKED_EVENT &&
      globalState->getFeedMode() == GlobalFeedMode::THREAD) {
    readyToThread = true;
    globalState->setMotionMode(GlobalMotionMode::ENABLED);
    globalState->setThreadSyncState(GlobalThreadSyncState::SYNC);
    pulsesBackToSync = 0;
  }*/
}

void ButtonPad::enableHandler(ButtonInfo press) {
  GlobalButtonLock lockState = GlobalState::getInstance()->getButtonLock();
  GlobalMotionMode motionMode = GlobalState::getInstance()->getMotionMode();
  if (lockState == GlobalButtonLock::LK_LOCKED) {
    Serial.println("Locked, ingoring enable");
    return;
  }

  if (press.buttonState == BS_CLICKED) {
    if (motionMode == GlobalMotionMode::MM_ENABLED) {
      Serial.println("Enable button clicked, disabling");
      GlobalState::getInstance()->setMotionMode(GlobalMotionMode::MM_DISABLED);
    }
    if (motionMode == GlobalMotionMode::MM_DISABLED) {
      Serial.println("Enable button clicked, enabling");
      GlobalState::getInstance()->setMotionMode(GlobalMotionMode::MM_ENABLED);
    }
  }
}

void ButtonPad::lockHandler(ButtonInfo press) {
  GlobalState* globalState = GlobalState::getInstance();

  if (press.buttonState == BS_CLICKED) {
    if (globalState->getButtonLock() == GlobalButtonLock::LK_LOCKED) {
      Serial.println("Unlocking");
      globalState->setButtonLock(GlobalButtonLock::LK_UNLOCKED);
    } else {
      Serial.println("Locking");
      globalState->setButtonLock(GlobalButtonLock::LK_LOCKED);
    }
  }
}

void ButtonPad::threadSyncHandler(ButtonInfo press) {
  GlobalButtonLock lockState = GlobalState::getInstance()->getButtonLock();
  if (lockState == GlobalButtonLock::LK_LOCKED) {
    Serial.println("Locked, ingoring thread sync");
    return;
  }

  if (press.buttonState == BS_CLICKED) {
    if (GlobalState::getInstance()->getMotionMode() ==
      GlobalMotionMode::MM_ENABLED) {
      Serial.println("Sync button clicked, enabled, unsyncing");
      GlobalState::getInstance()->setThreadSyncState(
        GlobalThreadSyncState::SS_UNSYNC);
    } else {
      Serial.println("Sync button clicked, disabled, syncing");
      GlobalState::getInstance()->setThreadSyncState(
        GlobalThreadSyncState::SS_SYNC);
    }
  }
}

void ButtonPad::modeCycleHandler(ButtonInfo press) {

  GlobalState* globalState = GlobalState::getInstance();
  GlobalButtonLock lockState = globalState->getButtonLock();

  if (lockState == GlobalButtonLock::LK_LOCKED) {
    Serial.println("Locked, ingoring mode");
    return;
  }
  
  // pressing mode button swaps between feed and thread
  if (press.buttonState == BS_CLICKED) {
    switch (GlobalState::getInstance()->getFeedMode()) {
    case GlobalFeedMode::FM_FEED:
      GlobalState::getInstance()->setFeedMode(GlobalFeedMode::FM_THREAD);
      break;
    case GlobalFeedMode::FM_THREAD:
      GlobalState::getInstance()->setFeedMode(GlobalFeedMode::FM_FEED);
      break;
    }
    m_leadscrew->setRatio(globalState->getCurrentFeedPitch());
  }

  // holding mode button swaps between metric and imperial
  if (press.buttonState == BS_HELD) {
    switch (GlobalState::getInstance()->getUnitMode()) {
    case GlobalUnitMode::METRIC:
      GlobalState::getInstance()->setUnitMode(GlobalUnitMode::IMPERIAL);
      break;
    case GlobalUnitMode::IMPERIAL:
      GlobalState::getInstance()->setUnitMode(GlobalUnitMode::METRIC);
      break;
    }
    m_leadscrew->setRatio(globalState->getCurrentFeedPitch());
  }
}

void ButtonPad::jogDirectionHandler(ButtonInfo press) {
  GlobalState* globalState = GlobalState::getInstance();
  GlobalButtonLock lockState = globalState->getButtonLock();
  GlobalMotionMode motionMode = globalState->getMotionMode();

  // no jogging functionality allowed during lock or enable
  if (lockState == GlobalButtonLock::LK_LOCKED ||
    motionMode == GlobalMotionMode::MM_ENABLED) {
    Serial.println("Locked, ingoring jog");
    return;
  }

  // single click should jog to the stop position
  if (press.buttonState == BS_CLICKED) {
    switch (press.button) {
    case ELS_JOG_LEFT_BUTTON:
      if (m_leadscrew->getStopPositionState(LeadscrewStopPosition::LEFT) != LeadscrewStopState::UNSET) {
        m_leadscrew->setExpectedPosition(m_leadscrew->getStopPosition(LeadscrewStopPosition::LEFT));
        globalState->setThreadSyncState(GlobalThreadSyncState::SS_UNSYNC);
      }
      break;
    case ELS_JOG_RIGHT_BUTTON:
      if (m_leadscrew->getStopPositionState(LeadscrewStopPosition::RIGHT) != LeadscrewStopState::UNSET) {
        m_leadscrew->setExpectedPosition(m_leadscrew->getStopPosition(LeadscrewStopPosition::RIGHT));
        globalState->setThreadSyncState(GlobalThreadSyncState::SS_SYNC);
      }
      break;
    }
  }

  /**
   * holding the jog button will set/unset the stop position
   */
  if (press.buttonState == BS_HELD) {
    switch (press.button) {
    case ELS_JOG_LEFT_BUTTON:
      if (m_leadscrew->getStopPositionState(LeadscrewStopPosition::LEFT) ==
        LeadscrewStopState::UNSET) {
        m_leadscrew->setStopPosition(LeadscrewStopPosition::LEFT);

      } else {
        m_leadscrew->unsetStopPosition(LeadscrewStopPosition::LEFT);
      }
      break;
    case ELS_JOG_RIGHT_BUTTON:
      if (m_leadscrew->getStopPositionState(LeadscrewStopPosition::RIGHT) ==
        LeadscrewStopState::UNSET) {
        m_leadscrew->setStopPosition(LeadscrewStopPosition::RIGHT);
      } else {
        m_leadscrew->unsetStopPosition(LeadscrewStopPosition::RIGHT);
      }
      break;
    }
  }
}

void ButtonPad::jogHandler(ButtonInfo press) {
  GlobalMotionMode motionMode = GlobalState::getInstance()->getMotionMode();

  jogDirectionHandler(press);

  // common jog functionality
  // if neither jog button is held, reset the motion mode
  if (press.buttonState == BS_RELEASED &&
    motionMode == GlobalMotionMode::MM_JOG) {
    GlobalState::getInstance()->setMotionMode(GlobalMotionMode::MM_DISABLED);
  }
}