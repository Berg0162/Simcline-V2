#ifndef CONFIGDEBUG_H_
#define CONFIGDEBUG_H_

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress ALL DEBUG messages that help to debug code sections
// Uncomment general "#define GLOBAL_DEBUG" to activate the Simcline-V2 debug master switch...
#define GLOBAL_DEBUG
// ------------------------------------------------------------------------------------------------

#include "Utilities.h"

// If global GLOBAL_DEBUG is defined and local file DEBUG is defined
#if defined(GLOBAL_DEBUG) && defined(DEBUG)
  #define LOG(...) UTILS::getInstance()->logF(__VA_ARGS__)  // Use logF instead of Serial.printf()
#else
  #undef DEBUG
  #define LOG(...)
#endif

#endif // CONFIGDEBUG_H_