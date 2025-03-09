#ifndef DebugMacros_h
#define DebugMacros_h

/* ------------------------------------------------- */

#ifndef DEBUG_SERIAL
  #define DEBUG_SERIAL            Serial
#endif
#ifndef DEBUG_TELNET
  #define DEBUG_TELNET            telnet
#endif
#ifndef DEBUG_PREFIX
  #define DEBUG_PREFIX            "[DEBUG]\t"
#endif

/* ------------------------------------------------- */

#ifndef DEBUG_ON
  #define DEBUG_INFO
  #define DEBUG_MSG(x)
  #define DEBUG_VAR(...)
  #define DEBUG_WHERE
#else 
  #if DEBUG_USE_SERIAL
    #define DEBUG_SER_INFO      DEBUG_SERIAL.print(DEBUG_PREFIX); DEBUG_SERIAL.println("Compiled " __DATE__ ", " __TIME__)
    #define DEBUG_SER_MSG(x)    DEBUG_SERIAL.print(DEBUG_PREFIX); DEBUG_SERIAL.println(x)
    #define DEBUG_SER_F(...)    DEBUG_TELNET.print(DEBUG_PREFIX); DEBUG_SERIAL.printf(__VA_ARGS__)
    #define DEBUG_SER_C(...)    DEBUG_TELNET.print(DEBUG_PREFIX); DEBUG_SERIAL.printf(__VA_ARGS__)
    #define DEBUG_SER_HOME()    DEBUG_TELNET.print(ansi.home())
    #define DEBUG_SER_VAR(...)  DEBUG_SERIAL.print(DEBUG_PREFIX); DEBUG_SERIAL.print(F(#__VA_ARGS__ " = ")); DEBUG_SERIAL.println(__VA_ARGS__)
    #define DEBUG_SER_WHERE     DEBUG_SERIAL.print(DEBUG_PREFIX); DEBUG_SERIAL.print(F(__FILE__ " - ")); DEBUG_SERIAL.print(__PRETTY_FUNCTION__); DEBUG_SERIAL.print(": "); DEBUG_SERIAL.println(__LINE__)
  #else
    #define DEBUG_SER_INFO
    #define DEBUG_SER_MSG(x)
    #define DEBUG_SER_F(...)
    #define DEBUG_SER_C(...)
    #define DEBUG_SER_HOME()
    #define DEBUG_SER_VAR(...)
    #define DEBUG_SER_WHERE
  #endif

  #if DEBUG_USE_TELNET
    #define DEBUG_TEL_INFO      DEBUG_TELNET.print(DEBUG_PREFIX); DEBUG_TELNET.println("Compiled " __DATE__ ", " __TIME__)
    //#define DEBUG_TEL_MSG(x)    DEBUG_TELNET.print(DEBUG_PREFIX); DEBUG_TELNET.println(x)
    #define DEBUG_TEL_F(...)    DEBUG_TELNET.print(ansi.clearLine()); DEBUG_TELNET.printf(__VA_ARGS__)
    #define DEBUG_TEL_C(...)    DEBUG_TELNET.printf(__VA_ARGS__)
    #define DEBUG_TEL_HOME()    DEBUG_TELNET.print(ansi.home())
    //#define DEBUG_TEL_VAR(...)  DEBUG_TELNET.print(DEBUG_PREFIX); DEBUG_TELNET.print(F(#__VA_ARGS__  " = ")); telnet.println(String(__VA_ARGS__))
    #define DEBUG_TEL_WHERE     DEBUG_TELNET.print(DEBUG_PREFIX); DEBUG_TELNET.print(F(__FILE__ " - ")); DEBUG_TELNET.print(__PRETTY_FUNCTION__); DEBUG_TELNET.print(": "); DEBUG_TELNET.println(String(__LINE__))
  #else
    #define DEBUG_TEL_INFO
    #define DEBUG_TEL_MSG(x)
    #define DEBUG_TEL_VAR(...)
    #define DEBUG_TEL_F(...)
    #define DEBUG_TEL_C(...)
    #define DEBUG_TEL_HOME()
    #define DEBUG_TEL_WHERE
  #endif

  #if DEBUG_USE_PRINTF
  #define DEBUG_PRN_INFO      printf(DEBUG_PREFIX); printf("Compiled " __DATE__ ", " __TIME__)
//#define DEBUG_PRN_MSG(x)    print(DEBUG_PREFIX); println(x)
  #define DEBUG_PRN_F(...)    printf(__VA_ARGS__)
  #define DEBUG_PRN_C(...)    printf(__VA_ARGS__)
  #define DEBUG_PRN_HOME()    printf(ansi.home())
//#define DEBUG_PRN_VAR(...)  print(DEBUG_PREFIX); print(F(#__VA_ARGS__  " = ")); println(String(__VA_ARGS__))
  #define DEBUG_PRN_WHERE     printf(DEBUG_PREFIX); printf(F(__FILE__ " - ")); printf(__PRETTY_FUNCTION__); printf(": "); printf(String(__LINE__))
#else
  #define DEBUG_PRN_INFO
  #define DEBUG_PRN_MSG(x)
  #define DEBUG_PRN_VAR(...)
  #define DEBUG_PRN_F(...)
  #define DEBUG_PRN_C(...)
  #define DEBUG_PRN_HOME()
  #define DEBUG_PRN_WHERE
#endif

  #define DEBUG_INFO            DEBUG_SER_INFO; DEBUG_TEL_INFO; DEBUG_PRN_INFO
//  #define DEBUG_MSG(x)          DEBUG_SER_MSG(x); DEBUG_TEL_MSG(x)
//  #define DEBUG_VAR(...)        DEBUG_SER_VAR(__VA_ARGS__); DEBUG_TEL_VAR(__VA_ARGS__)
#define DEBUG_F(...)          DEBUG_SER_F(__VA_ARGS__); DEBUG_TEL_F(__VA_ARGS__); DEBUG_PRN_F(__VA_ARGS__)
#define DEBUG_C(...)          DEBUG_SER_C(__VA_ARGS__); DEBUG_TEL_C(__VA_ARGS__); DEBUG_PRN_C(__VA_ARGS__)
#define DEBUG_WHERE           DEBUG_SER_WHERE; DEBUG_TEL_WHERE; DEBUG_PRN_WHERE
#define DEBUG_HOME()          DEBUG_SER_HOME(); DEBUG_TEL_HOME(); DEBUG_PRN_HOME()
#endif

/* ------------------------------------------------- */
#endif
/* ------------------------------------------------- */
