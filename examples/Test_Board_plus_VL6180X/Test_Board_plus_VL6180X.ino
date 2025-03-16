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
* Requirements: VL6180X Time of Flight Sensor connected to an ESP32 board, 
*               with USB connection to the computer
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

// Include code for low level range reading with VL6180X
#include <lifter/Lifter.h> 
// Create instance of Lifter
Lifter* lift = new Lifter();

uint16_t currentReading; // Variable to hold VL6180X range reading

void setup(void) {
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

  presentation->ShowMessageWindow("SIMCLINE", "Test", "VL6180X", 500);

  LOG("---- SIMCLINE Test VL6180X Sensor  ----");
  LOG("   Board: %s", BOARD_NAME);
  LOG(" Display: %s", IDISPLAY);
  LOG(" Library: %s", CODE_VERSION);

  // Initialize Lifter Class!
  LOG("Initialize Basic VL6180X sensor!");
  if(!lift->Init()) { // Lifter init failed....
       presentation->ShowMessageWindow("Testing", "VL6180X", "Timeout!", 500);
  }  
}

void loop(void) {
  delay(100); // Respect sampling rate of about 10 Hz  
  currentReading = lift->getVL6180XRangeReading();  
  LOG("Pos: %d", currentReading); 
}
