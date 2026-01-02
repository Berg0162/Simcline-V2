/*********************************************************************
 This is programming code for ESP32 Espressif Wroom boards
 
      Tested with:  Adafruit Feather ESP32 V2 a.k.a. Huzzah
                    LilyGo ESP32 T-Display S3 (170x320)

 The code uses heavily the supplied: 

  ESP32 NimBLE libraries !!          
      see: https://github.com/h2zero/NimBLE-Arduino NimBLE Version 2.x

  TFT_eSPI Display library
      see: https://github.com/Bodmer/TFT_eSPI 

 Many have invested time and resources providing open source code!
 
        MIT license, check LICENSE for more information
        All text must be included in any redistribution
*********************************************************************/


/* -----------------------------------------------------------------------------------------------------
 *             This code should work with all indoor cycling trainers that fully support,
 *        Fitness Machine Service, Cycling Power Service and Cycling Speed & Cadence Service
 * ------------------------------------------------------------------------------------------------------
 * NOTICE: that you need to have set first all config file settings in accordance with your specific setup!
 *         see: ../Documents/Arduino/libraries/Simcline-V2/src/config
 *
 *  The code links a BLE Server (a Peripheral to Zwift) and a BLE Client (a Central to the Trainer) with a bridge 
 *  in between, the ESP32 being man-in-the-middle (MITM). The ESP32 is an integral part of the Simcline,
 *  that interprets the exchanged road grade and moves the front wheel up and down with the change in inclination.
 *  The ESP32-bridge can control, filter and alter the bi-directional interchanged data!
 *  The client-side (central) scans and connects with the Trainer relevant services: CPS and FTMS. It collects 
 *  all cyling data of the services and passes these on to the server-side....  
 *  The client-side supplies the Indoor Trainer with target and resistance control data.
 *  The server-side (peripheral) advertises and enables connection with cycling apps like Zwift and collects the app's  
 *  control commands, target and resistance data. It passes these on to the client-side....  
 *  The server-side supplies the app with the generated cycling data in return. 
 *  
 *  The client plus server (MITM) are transparent to the Indoor Trainer as well as to the training app Zwift or alike!
 *  
 *  Requirements: Zwift app or alike, ESP32 board + TFT/Oled display and a supported Indoor Trainer
 *  0) Upload and Run this code on your ESP32 board and connected display
 *  1) Start the Serial Monitor to catch debugging info and check the TFT/Oled display
 *  2) The code will do basic testing of electronic parts, settings and connected display
 *  3) Start/Power On the Indoor Trainer  
 *  4) Your ESP32 and Trainer (with <name>) will pair as reported in the output
 *  5) Start Zwift on your computer or tablet and wait....
 *  6) Search on the Zwift pairing screens for your ESP32 a.k.a. <SIM32>
 *  7) Pair: Power Source, Resistance and Cadence one after another with <SIM32>
 *  8) Optionally one can pair as well devices for heartrate and/or steering (Sterzo)
 *  9) Start the default Zwift ride or any ride you wish
 * 10) Make Serial Monitor output window visible on top of the Zwift window 
 * 11) Hop on the bike: do the work and feel resistance change with the road
 * 12) Inspect the info presented by Serial Monitor.....
 *  
 *   This device is identified with the name <SIM32>. You will see this only when connecting to Zwift on the 
 *   pairing screens! Notice: Zwift extends device names with additional numbers for identification!
 *  
*/ 

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress debug messages that help debugging...
// Uncomment "#define DEBUG" to activate debug messages from this main code section
#define DEBUG

#include <Simcline.h>

// ------------------------------------------------------------------------------------------------
#include <ManInTheMiddle.h>
// Create instance of MITM class
MITM* mitm = MITM::getInstance();

// -------------------------------- xControlLoop task definitions ---------------------------------
// Central Control Loop task to check for buttons pressed, connection status and change of road grade
TaskHandle_t xControlLoopHandle = NULL;
void xControlLoop(void* arg);

// ------------------------------------------------------------------------------------------------
void checkMITMdataChanged(void);
void loop() { // Arduino default loop() is no longer used: delete!!!
    vTaskDelete(NULL);  
}

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

  LOG("-------------- MITM xtr ------------");
  LOG("   Board: %s", BOARD_NAME);
  LOG(" Display: %s", IDISPLAY);
  LOG(" Library: %s", CODE_VERSION);
#if defined(ENABLE_WAHOOCPS)
  LOG(" Enabled: WAHOOCPS");
  //Show Name and SW version on TFT
  presentation->ShowMessageWindow("MITM", "Wahoo CPS", CODE_VERSION, 500);
#elif defined(ENABLE_TACXFEC)
  LOG(" Enabled: CPS and TACXFEC");
  //Show Name and SW version on TFT
  presentation->ShowMessageWindow("MITM", "Tacx FEC", CODE_VERSION, 500);
#elif defined(ENABLE_FTMS)
  LOG(" Enabled: CPS and FTMS");
  //Show Name and SW version on TFT
  presentation->ShowMessageWindow("MITM", "FTMS", CODE_VERSION, 500);
#elif defined(ENABLE_ZVS)
  LOG(" Enabled: CPS and ZVS");
  presentation->ShowMessageWindow("MITM", "ZVS", CODE_VERSION, 500);
#endif
#ifdef ENABLE_HRM
  LOG(" Enabled: HRM");
#endif
#ifdef ENABLE_CSC
  LOG(" Enabled: CSC");
#endif

  LOG("Initialize Operations!");
  operations->init();
  operations->setEnableUnknownDev(); // Allow unknown devices to connect for the first time!

  LOG("Create Central Control Loop!");
  // Start a Central Control task to check for buttons pressed, connection status and change of road grade
  xTaskCreatePinnedToCore(xControlLoop, "xControlLoop", 8192, NULL, 5, &xControlLoopHandle, xTaskCoreID1); //Core #1
  presentation->ShowMessageWindow("Pairing!", "Trainer", "Laptop", 0);

  LOG("Starting NimBLE MITM!");
  // Start NimBLE Man-In-The-Middle and pair with Trainer and Laptop
  mitm->start();
} // End of setup

void checkMITMdataChanged(void) {
      if(operations->isGradeChanged()) {
          float grade = operations->getNewGrade();
          presentation->ShowRoadGrade(grade);
      }
      presentation->ShowIconsOnTopBar(operations->isTrainerConnected(), operations->isLaptopConnected(), operations->isSmartphoneConnected());
}

// Central Control task to check for BLE connection status and change of road grade
// Zwift changes road grade NOT at a constant pace but only when relevant!
// Interval time does rarely fall below 800 ms.
void xControlLoop(void *arg) {
  const TickType_t xDelay = 900 / portTICK_PERIOD_MS; // Block for 900ms
  while(1) {
    checkMITMdataChanged();
    vTaskDelay(xDelay);
  }
}
