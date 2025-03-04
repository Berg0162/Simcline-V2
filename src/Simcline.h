#ifndef SIMCLINE_H_
#define SIMCLINE_H_

#include <Arduino.h>
#include <string.h>
#include <Wire.h>

static const char* CODE_VERSION = "1.0.0";

// SIMCLINE HEADER FILE----------------------------------------------------------------------------
// Include these debug utility macros in all cases!
#include "config/configDebug.h"
// ------------------------------------------------------------------------------------------------
// Include board specific specifications for pin assignments and setup
#include "config/configBoard.h"
// Create instance of the ESP32Board class
ESP32Board* espBoard = new ESP32Board();
// ------------------------------------------------------------------------------------------------
// Include display selection, specifications and setup
#include "config/configDisplay.h"
// ------------------------------------------------------------------------------------------------
#include "Presentation.h"
// Create instance of the Presentation class and inject the specific pDisplay object
Presentation* presentation = Presentation::getInstance(pDisplay); // !! First Call !!
// ------------------------------------------------------------------------------------------------
// Include the mechanical and logical configuration settings of the Simcline
#include "config/configSimcline.h"
// ------------------------------------------------------------------------------------------------
// Include all NimBLE related constants
#include "config/configNimBLE.h"
// ------------------------------------------------------------------------------------------------
// Include some general component
#include "Utilities.h"
// Create instance of the UTILS class
UTILS* utils = UTILS::getInstance();
// ------------------------------------------------------------------------------------------------
// Include Simcline Operations and status class
#include "Operations.h"
// Create instance of the Operations class
OPS* operations = OPS::getInstance();
// SIMCLINE HEADER FILE----------------------------------------------------------------------------
#endif // SIMCLINE_H_