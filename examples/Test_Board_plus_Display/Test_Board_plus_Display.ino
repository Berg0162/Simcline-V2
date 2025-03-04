/*********************************************************************
 This is programming code for ESP32 Espressif Wroom boards
 
      Tested with:  Adafruit Feather ESP32 V2 a.k.a. Huzzah + SSD1306
                    LilyGo ESP32 T-Display S3 (170x320)
 
 Many have invested time and resources providing open source code!
 
        MIT license, check LICENSE for more information
        All text must be included in any redistribution
*********************************************************************/

/*
*  Requirements: A display of your choice connected to your ESP32 board, 
*               with USB connection to the computer
*/

// ------------------------------------------------------------------------------------------------
// -------- You need to set first the board specification to handle correct pin assigments --------
// Using notepad, edit and save the file "configBoard.h" to comply with your board properties
// NOTICE: configBoard.h is stored in the config directory of the Simcline-V2 library and
//         is activated during initialization of the Simcline-V2 library !!!
//         see: ../Documents/Arduino/libraries/Simcline-V2/src/config
// ------------------------------------------------------------------------------------------------
// Edit the local file "YOURDisplay.h" to implement for the display of your choice all the IDisplay 
// class methods as indicated in this file! 
// Run and test the results with your board and display.... Repeat until the results are 
// satisfactory! Then you have to copy the local "YOURDisplay.h" file to the Simcline-V2 library!
// NOTICE: "YOURDisplay.h" is stored in the display directory of the Simcline-V2 library and
//         is activated during initialization of the Simcline-V2 library !!!
//         see: ../Documents/Arduino/libraries/Simcline-V2/src/display
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress debug messages that help debugging...
// Uncomment "#define DEBUG" to activate debug messages from this main code section
#define DEBUG

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE for exclusive use of this local "YOURDisplay.h" file
#define USELOCALDISPLAYFILE  
// Work with the local "YOURDisplay.h" file  -> Edit -> Compile -> Upload -> Run -> Test
// To have this file included in other programs --> transfer the local display file to the library !!
//         see: ../Documents/Arduino/libraries/Simcline-V2/src/display
// ------------------------------------------------------------------------------------------------

// Include Simcline Library
#include <Simcline.h>

const bool UP = true;
const bool DOWN = false; 

// Optional use of 2 internal/external buttons to dynamically dim the display brightness
#if defined(PIN_BUTTON_1) && defined(PIN_BUTTON_2)
void checkButtons(void) {
    if(digitalRead(PIN_BUTTON_1)==0 && digitalRead(PIN_BUTTON_2)==1 ) { // button #1 pressed    
        LOG("Button 1 pressed -> down!");
        presentation->setDisplayBrightness(DOWN);
    }
    if(digitalRead(PIN_BUTTON_2)==0 && digitalRead(PIN_BUTTON_1)==1) { // button #2 pressed
        LOG("Button 2 pressed -> up!");
        presentation->setDisplayBrightness(UP);
    }
}
#endif

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

    LOG("----- SIMCLINE Test Your Display  -----");
    LOG("   Board: %s", BOARD_NAME);
    LOG(" Display: %s", IDISPLAY);
    LOG(" Library: %s", CODE_VERSION);
    presentation->setDisplayBrightness(UP);
    delay(1000);
    presentation->setDisplayBrightness(DOWN);
    delay(1000);
    for (uint8_t i = 0; i < 10; i++) {
      // Repeat to test blink state of icon(s)
      presentation->ShowIconsOnTopBar(false, false, false);
      delay(200);
    }
    presentation->ShowMessageWindow("Text Line ", "Text Line ", "Text Line ", 2000); // Check Max length!
    presentation->ShowIconsOnTopBar(true, false, true);
    delay(2000);
    presentation->ShowRoadGrade(5.6);
    presentation->ShowIconsOnTopBar(true, true, false);
    delay(1000);
    presentation->ShowRoadGrade(-1.6);
    presentation->ShowIconsOnTopBar(true, true, false);
    delay(1000);
    presentation->ShowRoadGrade(-3.3);
    presentation->ShowIconsOnTopBar(true, true, false);
    delay(1000);
    presentation->ShowRoadGrade(-6.3);
    presentation->ShowIconsOnTopBar(true, true, false);
    delay(1000);
    presentation->ShowRoadGrade(0.6);
    presentation->ShowIconsOnTopBar(true, true, false);
    delay(1000);
    presentation->ShowRoadGrade(2.5);
    presentation->ShowIconsOnTopBar(true, true, false);
    delay(1000);
    presentation->ShowRoadGrade(10.8);
    presentation->ShowIconsOnTopBar(true, true, false);
}

void loop() {
#if defined(PIN_BUTTON_1) && defined(PIN_BUTTON_2)
    checkButtons();
#endif
}