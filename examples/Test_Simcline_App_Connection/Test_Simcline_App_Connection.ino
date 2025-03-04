
/*********************************************************************
 This is programming code for ESP32 Espressif Wroom boards
 
      Tested with:  Adafruit Feather ESP32 V2 a.k.a. Huzzah
                    LilyGo ESP32 T-Display S3 (170x320)
 
 The code uses heavily the supplied ESP32 NimBLE libraries !!          
      see: https://github.com/h2zero/NimBLE-Arduino NimBLE version 2.x
 Many have invested time and resources providing open source code!
 
        MIT license, check LICENSE for more information
        All text must be included in any redistribution
*********************************************************************/

/* 
 *  This ESP32 code enables the relevant services and advertises...
 *  It allows to connect the ESP32 board and the Simcline Companion app!
 *  Requirements: Simcline (Android) app installed on your phone and an ESP32 board
 *  1) Upload and Run this code on the ESP32 board
 *  2) Start the Serial Monitor to catch debugging info
 *  3) Start the Simcline app and wait for Devices Pairing Screen
 *  4) Touch the start BLE scanning button/icon in the app...
 *  5) Your phone will connect with device <SIM32> when found!
 *  6) Check the Simcline app menu and select an option:
 *  7) Make your preferred settings and/or test manual control
 *  8) Inspect the info presented by Serial Monitor
 *   
 *   This device is identified with the name <SIM32>. You will see this only when connecting!
 *  
*/ 

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress debug messages that help debugging...
// Uncomment "#define DEBUG" to activate debug messages from this main code section
#define DEBUG

#include <Simcline.h>

//-------------------------------------------------------------------------------------------------
#include <ext/ServerSideXP.h>
//Create instance of ServerSideXP class, derived from ServerSide class
ServerSideXP* serverxp = ServerSideXP::getInstance();

// -------------------------------- xControlLoop task definitions ---------------------------------
// Central Control Loop task to check for optional change of road grade
TaskHandle_t xControlLoopHandle = NULL;
void xControlLoop(void* arg);

// ------------------------------------------------------------------------------------------------
void loop() { // Arduino default loop() is no longer used: delete!!!
    vTaskDelete(NULL);  
}

// ------------------------------------------------------------------------------------------------
void setup() {
  // Init USB connection first
  espBoard->initUSB(115200);
  // Print framework versions and board specifications 
  espBoard->info();    
  // Board-specific setup
  espBoard->setup();      

  LOG("Initialize Display!");
  // Initialize the connected display
  presentation->initDisplay();

  LOG("------ ServerXP with PHONE -------");
  LOG("   Board: %s", BOARD_NAME);
  LOG(" Display: %s", IDISPLAY);
  LOG(" Library: %s", CODE_VERSION);
  LOG("  NimBLE: NUS a.k.a. Nordic UART Service"); 
  
  LOG("Initialize Operations!");
  operations->init();

  LOG("Create Central Control Loop!");
  // Start a Central Control task to check connection status and change of road grade
  xTaskCreatePinnedToCore(xControlLoop, "xControlLoop", 8192, NULL, 5, &xControlLoopHandle, xTaskCoreID1); //Core #1

  // Initialize the derived ServerSideXP!
  LOG("Starting the ServerXP!"); 
  serverxp->start();

  LOG(">>> Connect with Simcline app on your Smartphone!");
} // End of setup

// Central Control task to check connection status and change of road grade
void xControlLoop(void *arg) {
  const TickType_t xDelay = 300 / portTICK_PERIOD_MS; // Block for 300ms
  while(1) {
    if(operations->isGradeChanged()) {
      float grade = operations->getNewGrade();
      presentation->ShowRoadGrade(grade);
    }
    vTaskDelay(xDelay);
  }
}
