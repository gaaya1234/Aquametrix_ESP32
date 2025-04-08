#include "init.h"

/* If modem, use MODEM_MODE. If wifi, use WIFI_MODE */
#ifdef MODEM_MODE
/* See all AT commands, if wanted */
// #define DUMP_AT_COMMANDS
#ifdef DUMP_AT_COMMANDS
#include <StreamDebugger.h>
StreamDebugger debugger(SerialAT, Serial);
TinyGsm modem(debugger);
TinyGsmClient client(modem);
#else
TinyGsm modem(SerialAT);
TinyGsmClient client(modem);
#endif
#endif

SIM7600G modem_lte(LED_PIN, MODEM_PWRKEY, MODEM_FLIGHT);
