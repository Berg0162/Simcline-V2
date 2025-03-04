/*********************************************************************

 The code uses heavily the supplied: 

  ESP32 NimBLE libraries !!          
      see: https://github.com/h2zero/NimBLE-Arduino 

 Many have invested time and resources providing open source code!
 
        MIT license, check LICENSE for more information
        All text must be included in any redistribution
*********************************************************************/

#include "ManInTheMiddle.h"

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress DEBUG messages that help debugging...
// Uncomment general "#define DEBUG" to activate debug messages for this class
#define DEBUG
// ------------------------------------------------------------------------------------------------
// Include these debug utility macros in all cases!
#include "config/configDebug.h"

// Include presentation class for display
#include "Presentation.h"
// Include operations control settings class
#include "Operations.h"
// include NimBLE Client class
#include "ClientSide.h"
// include NimBLE Server class
#include "ServerSide.h"


// Initialize the static members
MITM* MITM::instance = nullptr;

// Constructor implementation
MITM::MITM() { }

// Destructor implementation
MITM::~MITM() { }

MITM* MITM::getInstance() {
    if (instance == nullptr) {
        instance = new MITM();
    }
    return instance;
}

void MITM::start(void) {
  // Setup the Server-side FIRST!
  ServerSide::getInstance()->init();
  /*  -------------------------------------------------------------------------------
  *   Notice: Client connects FIRST to a peripheral/Trainer and transfers client-side 
  *   (trainer) properties to its server-side to mimic the Trainer at best against
  *   the Centra/Zwift training app!
  *   ------------------------------------------------------------------------------- 
  */
  // Start the Client-side and find a Peripheral/Trainer first to connect with!
  ClientSide::getInstance()->startScanning();
  // Wait until the Peripheral/Trainer is successfuly connected or has a timeout
  const long TIMEOUT = millis() + 10000; // Within 10 seconds it should have been connected!
  while(!OPS::getInstance()->isTrainerConnected()) {
    delay(100);
    if(millis() > TIMEOUT) {
      //LOG(">>> Scanning for Peripheral/Trainer --> Timeout!");
      break;
    }
  }
  // Preferably the client is connected --> Advertise the Server-side and find
  // a Central (Laptop or Phone) to connect with!
  ServerSide::getInstance()->startAdvertising();
};

