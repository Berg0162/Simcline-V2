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
* Requirements: Fully equiped and operational Simcline (TOF-sensor/display/driver/Actuator),
*               hardware components tested separately with an ESP32 board, like 
*               an Adafruit Feather ESP32 V2 or a LilyGo ESP32 T-Display-S3!
*
* Diagnostics can be run with manual input of data for Up/Down movement of Actuator
* or with the continuous input of data from a predetermined array of 8 test values! 
*
*/ 

// -------------------------------------------------------------------------------------------
// ------ You need to set first the board specification to handle correct pin assigments -----
// Using notepad, edit and save the file "configBoard.h" to comply with your board properties
// NOTICE: configBoard.h is stored in the config directory of the Simcline-V2 library and
//         is activated during initialization of the Simcline-V2 library !!!
//         see: ../Documents/Arduino/libraries/Simcline-V2/src/config
// -------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress debug messages that help debugging...
// Uncomment "#define DEBUG" to activate debug messages from this main code section
#define DEBUG

#include <Simcline.h>

// ------------------------------------------------------------------------------------------------
// Include code for low level measuring (VL6180X), control and execution of UP and down movement
#include <lifter/Lifter.h> 
// Create instance of Lifter
Lifter* lift = new Lifter();
// Global variable for correct working of the mechanical motor functions
boolean IsBasicMotorFunctions = false; 

// -------------------------------- xControlLoop task definitions ---------------------------------
// Central Control Loop task to check for buttons pressed, connection status and change of road 0
TaskHandle_t xControlLoopHandle = NULL;
void xControlLoop(void* arg);

// ------------------------------------------------------------------------------------------------
void checkMITMdataChanged(void);
void loop() { // Arduino default loop() is no longer used: delete!!!
    vTaskDelete(NULL);  
}

#include "handleInputValues.h"

void setup() {
  // Init USB connection first
  espBoard->initUSB(115200);
  // Print framework versions and board specifications 
  espBoard->info();    
  // Init and (optionally) scan for attached devices      
  espBoard->initWire(true);  
  // Board-specific setup
  espBoard->setup();      

  LOG("Initialize Display!");
  // Initialize the connected display
  presentation->initDisplay();

  presentation->ShowMessageWindow("Simcline", "Diagnose", CODE_VERSION, 500);

  LOG("--- SIMCLINE Functioning Diagnostics ---");
  LOG("   Board: %s", BOARD_NAME);
  LOG(" Display: %s", IDISPLAY);
  LOG(" Library: %s", CODE_VERSION);

  LOG("Initialize Operations!");
  operations->init(false); // Use the Default Simcline Settings!

  // Initialize Lifter Class data, variables, test and set to work !
  LOG("Setup Lifter and Actuator!");
  if(lift->Init()) { 
    // Test Actuator and VL8106X for proper functioning
    LOG("Test Actuator Motor Functions!");
    presentation->ShowMessageWindow("Testing", "Lifting", "Functions", 100);
    if( lift->TestBasicMotorFunctions() ) {
        presentation->ShowMessageWindow("Testing", "Functions", "Done!", 500);
        LOG("Simcline Basic Motor Funtions are working!");
        // Is working properly --> Start Motor Control Task, running on core 1
        if(lift->StartControl(xTaskCoreID1)) { //Run on Core #1
            IsBasicMotorFunctions = true;
            // Put Simcline in neutral: 0% flat road position
            operations->levelGrade();
        }
    } 
    if(!IsBasicMotorFunctions) { // Test Failed OR start control failed!
        presentation->ShowMessageWindow("Testing", "Functions", "Failed!", 500);
        LOG("Simcline >> ERROR << Basic Motor Funtions are NOT working!!");
    }
  } else { // Lift init failed....
      presentation->ShowMessageWindow("Testing", "VL6180X", "Timeout!", 500);
  }

  if(!IsBasicMotorFunctions) return; // No use of testing --> stop

  LOG("Create Central Control Loop!");
  // Start a Central Control task to check for change of road grade and user input
  xTaskCreatePinnedToCore(xControlLoop, "xControlLoop", 8192, NULL, 5, &xControlLoopHandle, xTaskCoreID1); //Core #1
  delay(100); // Allow some time to settle  
  presentation->ShowMessageWindow("Ready!", "for", "Input", 0);
  delay(1000);

#ifdef MANUAL_INPUT_DATA 
  LOG("Type 'u' for UP or 'd' for DOWN movement or '0' (zero) for neutral position, and close with <ENTER> to confirm!");
#endif
} // End of setup.

void checkMITMdataChanged(void) {
  float grade;
  if(operations->getNewGradeIfChanged(grade)) {
    targetPosition = operations->GetNewTargetPosition(grade);
    if(IsBasicMotorFunctions) {  
      lift->SetTargetPosition(targetPosition);
    } 
    presentation->ShowRoadGrade(grade);
    LOG("Target pos: %04d Grade: %5.1f %%", targetPosition, grade); 
  }
}

// Central Control task to check for change of road grade and user input
void xControlLoop(void *arg) {
  // Give enough time to reach the targeted position before setting a new target position
  // Notice:  The time spent to reach a target position is fully dependent of the properties of the Actuator 
  //          New target position (deliberately) overrules the current, even when old postion is not reached!
#ifdef MANUAL_INPUT_DATA
  const TickType_t xDelay = 100 / portTICK_PERIOD_MS; // Block for 100ms
#else
  const TickType_t xDelay = 10000 / portTICK_PERIOD_MS; // Block for 10 seconds 
#endif
  while(1) {
    getInputValue();
    checkMITMdataChanged();
    vTaskDelay(xDelay);
  }
}
