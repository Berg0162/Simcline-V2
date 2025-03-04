/*********************************************************************
 This is programming code for ESP32 Espressif Wroom boards
 
      Tested with:  Adafruit Feather ESP32 V2 a.k.a. Huzzah + SSD1306
                    LilyGo ESP32 T-Display S3 (170x320)
 
 Many have invested time and resources providing open source code!
 
        MIT license, check LICENSE for more information
        All text must be included in any redistribution
*********************************************************************/

/*
*   Requirements: An ESP32 board with USB connection to the computer
*/

// ------------------------------------------------------------------------------------------------
// -------- You need to set first the board specification to handle correct pin assigments --------
// Using notepad, edit and save the file "configBoard.h" to comply with your board properties
// NOTICE: configBoard.h is stored in the config directory of the Simcline-V2 library and
//         is activated during initialization of the Simcline-V2 library !!!
//         see: ../Documents/Arduino/libraries/Simcline-V2/src/config
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress DEBUG messages that help debugging...
// Uncomment general "#define DEBUG" to activate debug messages from this main code section
#define DEBUG

#include <Simcline.h>

// Uncomment and Edit ONLY when you want to store a new Mac Addresses in NVS for later use!
// TRAINER Fixed Device Address --------Public Type (0) --------- Random Type (1)----------
//#define NEW_TRAINERADDRESS  "00:01:02:03:04:05 0" // Example Mac string of Public Type
// LAPTOP Fixed Device Address --------Public Type (0) --------- Random Type (1)-----------
//#define NEW_LAPTOPADDRESS  "00:01:02:03:04:05 1"  // Example Mac string of Random Type

NimBLEAddress trainerAddress;
NimBLEAddress laptopAddress;
std::string address; // Address part
uint8_t type; // Type part
const std::string line = "---------------------------------------------------------------";

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

  LOG("--- Board Test Non-Volatile Storage ---");
  LOG("   Board: %s", BOARD_NAME);
  LOG(" Display: %s", IDISPLAY);
  LOG(" Library: %s", CODE_VERSION);
  LOG("%s", line.c_str());
  LOG("NVS Settings at Startup:");
  operations->init();
  LOG("%s", line.c_str());

  /* -----------------  CAUTION  --------------------------- *
  * Activating clearSIMprefsNVS() will wipe out all the      *
  * current content of the <SIMprefs> Name Space in NVS .... *
  * this relates to:                                         *
  *    <NimBLE Settings>  AND  <Simcline Settings>           *
  * ---------------------------------------------------------*/
  /* --------------------------------------------------------*
  *   utils->clearSIMprefsNVS();                             *
  * ---------------------------------------------------------*/

  // In the following sections you can set defined new Mac Addresses in NVS!
  // First fill the appropriate values for Trainer and Laptop, run and
  // have the new Mac Addresses stored in NVS for later use by the Simcline code.
  // This test is limited to the socalled <NimBLE Settings>! 
  //
  // Run another "test"-program for testing the Smartphone-Simcline connection over BLE.
  // Use the Simcline App on your smartphone to change the <Simcline Settings> in NVS.

#ifdef NEW_TRAINERADDRESS
  LOG("New Trainer Mac Address: [%s]", NEW_TRAINERADDRESS);
  if(utils->parseMacAddressString(NEW_TRAINERADDRESS, address, type) ) 
    trainerAddress = NimBLEAddress(address.c_str(), type); // Create NimBLEAddress with Type
  else return;
  utils->setMacAddressNVS("Trainer", trainerAddress);
#endif

#ifdef NEW_LAPTOPADDRESS
  LOG("New Laptop Mac Address: [%s]", NEW_LAPTOPADDRESS);
  if(utils->parseMacAddressString(NEW_LAPTOPADDRESS, address, type) ) 
    laptopAddress = NimBLEAddress(address.c_str(), type); // Create NimBLEAddress with Type
  else return;
  utils->setMacAddressNVS("Laptop", laptopAddress);
#endif
  LOG("%s", line.c_str());
  LOG("Check the <NimBLE Settings> Again:");
  trainerAddress = utils->getMacAddressNVS("Trainer");
  laptopAddress = utils->getMacAddressNVS("Laptop");
  LOG("----------------------- Test complete --------------------------");
  LOG("The current <NimBLE Settings> will be used in the Simcline code!");
}

void loop() 
{}
