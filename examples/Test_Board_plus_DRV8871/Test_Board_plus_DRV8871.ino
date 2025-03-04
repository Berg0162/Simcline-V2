/*********************************************************************
 This is programming code for ESP32 Espressif Wroom boards
 
      Tested with:  Adafruit Feather ESP32 V2 a.k.a. Huzzah
                    LilyGo ESP32 T-Display S3 (170x320)
 
 The code uses heavily the supplied ESP32 libraries for different components!          

 Many have invested time and resources providing open source code!
 
        MIT license, check LICENSE for more information
        All text must be included in any redistribution
*********************************************************************/

/*
*
* Requirements: DRV8871 Motor Driver Board connected to (a) functional Actuator and 
*               (b) an ESP32 board, with USB connection to the computer
* Test code can be run with manual input of data for Up/Down movement of Actuator
* or with the continuous input of data from a predetermined set of test values! 
*
*/

// -------------------------------------------------------------------------------------------
// ------ You need to set first the board specification to handle correct pin assigments -----
// Using notepad, edit and save the file "configBoard.h" to comply with your board properties
// NOTICE: configBoard.h is stored in the config directory of the Simcline-V2 library and
//         is activated during initialization of the Simcline-V2 library !!!
//         see: ../Documents/Arduino/libraries/Simcline-V2/src/config
// -------------------------------------------------------------------------------------------

// -------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress debug messages that help debugging...
// Uncomment "#define DEBUG" to activate debug messages from this main code section
#define DEBUG

// Include Simcline Library
#include <Simcline.h>

// -------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to help debugging...
// Comment out "#define MANUAL_INPUT_DATA" to activate: input of predetermined test values!
#define MANUAL_INPUT_DATA

#include "handleDRV8871.h"

void setup() {
  // Init USB connection first
  espBoard->initUSB(115200);
  // Print framework versions and board specifications 
  espBoard->info();    
  // Init and (optionally) scan for attached devices      
  espBoard->initWire(true);  
  // Board-specific setup
  espBoard->setup();      
 
  // Initialize the connected display
  presentation->initDisplay();

  presentation->ShowMessageWindow("SIMCLINE", "Test", "DRV8871", 500);

  LOG("-------- Simcline Test DRV8871 --------");
  LOG("   Board: %s", BOARD_NAME);
  LOG(" Display: %s", IDISPLAY);
  LOG(" Library: %s", CODE_VERSION);  

  initDRV8871();
}
  
void loop() {
#ifdef MANUAL_INPUT_DATA
    if(Serial.available()){
      input = Serial.read();
      switch (input)
      {
      case 117 : // u
        moveActuatorUp();
        delay(1000);
        brakeActuator();
        break;
      case 100 : // d
        moveActuatorDown();
        delay(1000);
        brakeActuator();
        break;
      case 48 : // 0
        brakeActuator();
        break;
      default :
        return;
      }
    } else return; 
#else
    moveActuator();
#endif
}

