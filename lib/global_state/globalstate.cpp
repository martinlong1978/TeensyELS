#ifndef PIO_UNIT_TESTING
#include <Wire.h>
#endif
#include <globalstate.h>

GlobalState* GlobalState::m_instance = nullptr;
GlobalState* GlobalState::getInstance() {
  if (m_instance == nullptr) {
    m_instance = new GlobalState();
  }
  return m_instance;
}


void GlobalState::printState() {
  switch (m_motionMode) {
  case MM_DISABLED:
    break;
  case MM_ENABLED:
    break;
  case MM_JOG_LEFT:
    break;
  case MM_JOG_RIGHT:
    break;
  }
  switch (m_feedMode) {
  case FM_FEED:
    break;
  case FM_THREAD:
    break;
  }
  switch (m_unitMode) {
  case METRIC:
    break;
  case IMPERIAL:
    break;
  }
  switch (m_threadSyncState) {
  case SS_SYNC:
    break;
  case SS_UNSYNC:
    break;
  }
}

void GlobalState::setFeedMode(GlobalFeedMode mode) {
  m_feedMode = mode;

  // when switching feed modes ensure that the default for the next mode is
  // selected via setFeedSelect - depends on the fallback in the function
  setFeedSelect(-1);
}

GlobalFeedMode GlobalState::getFeedMode() { return m_feedMode; }

int GlobalState::getFeedSelect() { return m_feedSelect; }
int GlobalState::getCurrentFeedSelectArraySize() {
  // this just ensures that the feedSelect doesn't go out of bounds for the
  // current arry
  if (m_unitMode == METRIC) {
    if (m_feedMode == FM_THREAD) {
      return ARRAY_SIZE(threadPitchMetric);
    } else {
      return ARRAY_SIZE(feedPitchMetric);
    }
  } else {
    if (m_feedMode == FM_THREAD) {
      return ARRAY_SIZE(threadPitchImperial);
    } else {
      return ARRAY_SIZE(feedPitchImperial);
    }
  }

  // invalid - should never get here!
  return -1;
}

void GlobalState::setButtonLock(GlobalButtonLock lock) { m_buttonLock = lock; }

GlobalButtonLock GlobalState::getButtonLock() { return m_buttonLock; }

void GlobalState::setFeedSelect(int select) {
  if (select >= 0 && select < getCurrentFeedSelectArraySize()) {
    m_feedSelect = select;
  } else {
    // if we're out of bounds, just set the default
    if (m_feedMode == FM_THREAD) {
      if (m_unitMode == METRIC) {
        m_feedSelect = DEFAULT_METRIC_THREAD_PITCH_IDX;
      } else {
        m_feedSelect = DEFAULT_IMPERIAL_THREAD_PITCH_IDX;
      }
    } else {
      if (m_unitMode == METRIC) {
        m_feedSelect = DEFAULT_METRIC_FEED_PITCH_IDX;
      } else {
        m_feedSelect = DEFAULT_IMPERIAL_FEED_PITCH_IDX;
      }
    }
  }
}

float GlobalState::getCurrentFeedPitch() {
  if (m_unitMode == METRIC) {
    if (m_feedMode == FM_THREAD) {
      return threadPitchMetric[m_feedSelect];
    } else {
      return feedPitchMetric[m_feedSelect];
    }
  }

  // special cases for imperial
  if (m_feedMode == FM_THREAD) {
    // threads are defined in TPI, not pitch
    return (1.0 / threadPitchImperial[m_feedSelect]) * 25.4;
  }
  // feeds are defined in thou/rev, not mm/rev
  return feedPitchImperial[m_feedSelect] * 25.4 / 1000;
}

int GlobalState::nextFeedPitch() {
  if (m_feedSelect != getCurrentFeedSelectArraySize() - 1) {
    setFeedSelect(m_feedSelect + 1);
  }

  return m_feedSelect;
}

int GlobalState::prevFeedPitch() {
  if (m_feedSelect != 0) {
    setFeedSelect(m_feedSelect - 1);
  }

  return m_feedSelect;
}

void GlobalState::setMotionMode(GlobalMotionMode mode) { m_motionMode = mode; }

GlobalMotionMode GlobalState::getMotionMode() { return m_motionMode; }

void GlobalState::setUnitMode(GlobalUnitMode mode) { m_unitMode = mode; }

GlobalUnitMode GlobalState::getUnitMode() { return m_unitMode; }

void GlobalState::setThreadSyncState(GlobalThreadSyncState state) {
  m_threadSyncState = state;
}

GlobalThreadSyncState GlobalState::getThreadSyncState() {
  return m_threadSyncState;
}
