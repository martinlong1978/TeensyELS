#define DEBUG_ON 1

#if !defined(PIO_UNIT_TESTING) && defined(ESP32)
#define DEBUG_USE_TELNET 1
#else
#define DEBUG_USE_PRINTF 1
#endif

#include <string>
#include "mydebugmacros.h"

#ifndef PIO_UNIT_TESTING
#include "./myescapecodes.h"
#ifdef ESP32
#include "ESPTelnet.h"
extern ESPTelnet telnet;
#endif
extern EscapeCodes ansi;
#endif

