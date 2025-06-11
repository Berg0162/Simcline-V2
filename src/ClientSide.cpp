#include "ClientSide.h"

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress DEBUG messages that help debugging...
// Uncomment general "#define DEBUG" to activate debug messages for this class
#define DEBUG
// Include these debug utility macros in all cases!
#include "config/configDebug.h"
// ------------------------------------------------------------------------------------------------

//UUID16_SVC_CYCLING_POWER   NimBLEUUID((uint16_t)0x1818)
//UUID16_SVC_FITNESS_MACHINE NimBLEUUID((uint16_t)0x1826)

#include "Utilities.h"

// Include operations control settings class
#include "Operations.h"
// Include presentation class for display
#include "Presentation.h"

#include "GenericAccess.h"
#include "DeviceInformation.h"
#include "CyclingPower.h"
#ifdef ENABLE_FTMS
  #include "FitnessMachine.h"
  #define SELECTSERVICE NimBLEUUID((uint16_t)0x1826)
#else 
  #define SELECTSERVICE NimBLEUUID((uint16_t)0x1818)
#endif
#ifdef ENABLE_HRM
  #include "HeartRateMonitor.h"
#endif
#ifdef ENABLE_CSC
  #include "CyclingSpeedCadence.h"
#endif
#ifdef ENABLE_TACXFEC
  #include "FitnessEquipmentCycling.h"
#endif
// Initialize the static members
NimBLEClient* ClientSide::pClient = nullptr;
ClientSide* ClientSide::instance = nullptr;

// Client Connect and Disconnect callbacks defined
class client_Connection_Callbacks:public NimBLEClientCallbacks {
private:
    ClientSide* clientInstance;  // LOCAL Pointer to store the ClientSide class instance
public:
    // Constructor to accept a pointer to the ClientSide class instance
    client_Connection_Callbacks(ClientSide* instance) : clientInstance(instance) {}
protected: 
  void onConnectfail(NimBLEClient* pClient, int reason) {
#ifdef DEBUG
#ifdef CONFIG_NIMBLE_CPP_ENABLE_RETURN_CODE_TEXT
    LOG(" -> Failed Reason [%d][%s]", reason, NimBLEUtils::returnCodeToString(reason));
#endif  
#endif  
  }
  void onConnect(NimBLEClient* pClient) override {
      clientInstance->clientConnectionCallbacksOnConnect(pClient);
  }
  void onDisconnect(NimBLEClient* pClient, int reason) override {
      clientInstance->clientConnectionCallbacksOnDisconnect(pClient, reason);
  }
  bool onConnParamsUpdateRequest(NimBLEClient* pClient, ble_gap_upd_params *params) {
#ifdef DEBUG
      LOG(" -> Client Rec'd Connection Parameter Update Request!");
      /** Minimum value for connection interval in 1.25ms units */
      uint16_t clientConnectionMinInterval = params->itvl_min;
      LOG("Min Interval: [%d]", clientConnectionMinInterval);
      /** Maximum value for connection interval in 1.25ms units */
      uint16_t clientConnectionMaxInterval = params->itvl_max;
      LOG("Max Interval: [%d]", clientConnectionMaxInterval);
      /** Connection latency */
      uint16_t clientConnectionLatency = params->latency;
      LOG("Latency: [%d]", clientConnectionLatency);
      /** Supervision timeout in 10ms units */
      uint16_t clientConnectionSupTimeout = params->supervision_timeout;
      LOG("Sup. Timeout: [%d]", clientConnectionSupTimeout);
      /** Minimum length of connection event in 0.625ms units */
      uint16_t clientMinLenEvent = params->min_ce_len;
      LOG("Min Length Event: [%d]", clientMinLenEvent);
      /** Maximum length of connection event in 0.625ms units */
      uint16_t clientMaxLenEvent = params->max_ce_len;  
      LOG("Max Length Event: [%d]", clientMaxLenEvent);
#endif
    return true; // That is OK!  
  }
};

/**
 * Scan for BLE servers and find the first one that advertises the service we are looking for.
 */

class clientScanCallbacks: public NimBLEScanCallbacks {

private:
    ClientSide* clientInstance;  // LOCAL Pointer to store the ClientSide class instance
public:
    // Constructor to accept a pointer to the ClientSide class instance
    clientScanCallbacks(ClientSide* instance) : clientInstance(instance) {}
protected:
  void onResult(const NimBLEAdvertisedDevice* advertisedDevice) override {
    clientInstance->clientScanCallbacksOnResult(advertisedDevice);
  }
}; // clientScanCallbacks

// Constructor
ClientSide::ClientSide() {
  operations = OPS::getInstance();
}

// Destructor
ClientSide::~ClientSide() {}

ClientSide* ClientSide::getInstance() {
    if (instance == nullptr) {
        instance = new ClientSide();
    }
    return instance;
}

void ClientSide::clientScanCallbacksOnResult(const NimBLEAdvertisedDevice* advertisedDevice) {
    // We have found a server device, now see if it contains the FTMS service we are looking for.
    if (advertisedDevice->haveServiceUUID() && advertisedDevice->isAdvertisingService(SELECTSERVICE)) {
      NimBLEAddress remoteAddress = advertisedDevice->getAddress();
      std::string deviceName = "Unknown";
      if(advertisedDevice->haveName()) { deviceName = advertisedDevice->getName(); }
      LOG("Found advertising Peripheral with Device Name: %s Mac Address: [%s]", deviceName.c_str(), \
                   UTILS::getInstance()->toString(remoteAddress).c_str());
      // OK Server has FTMS service exposed, now check for right mac adress
      if(remoteAddress != operations->Trainer.PeerAddress) { // Does NOT equal known Trainer MAC address
      	if(operations->enableUnknownDev) { // Only when allowed handle this one time exception
          operations->Trainer.PeerAddress = remoteAddress; // Set Unknown Mac Address to Trainer.PeerAddress
          LOG("Unknown Mac Address registered for Peripheral (Trainer)");
    	    UTILS::getInstance()->setMacAddressNVS("Trainer", remoteAddress); 	// Store Trainer Mac Address in NVS
      	} else {
          LOG(">>> NO MATCH! Keep Scanning for Trainer Mac Address: [%s]", \
                        UTILS::getInstance()->toString(operations->Trainer.PeerAddress).c_str());
          return;
	      }
      }      
      trainerDevice = advertisedDevice;
      pNimBLEScan->stop();  // Stop Scanning
      // Create dedicated xTask and make the client connection work!
      xTaskCreatePinnedToCore(this->xTaskClientConnectServer, "Client Connect", 4096, (void *)this, 15, \
                              &this->xTaskClientConnectServerHandle, xTaskCoreID0);      
    } // Found our server
} // onResult

bool ClientSide::clientConnectServer(void) {
    // Handle first time connect AND a reconnect. One fixed Peripheral (trainer) to account for!
    if(pClient == nullptr) { // First time -> create new pClient and service database!
      pClient = NimBLEDevice::createClient(); 
      pClient->setClientCallbacks(new client_Connection_Callbacks(this));
      // First Time Connect to the FTMS Trainer (Server/Peripheral)
      hasConnectPassed = pClient->connect(trainerDevice, true);   // Delete attribute objects and Create service database
    } else if(pClient == NimBLEDevice::getDisconnectedClient()) { // Allow for a streamlined reconnect
        // Reconnect to the disconnected FTMS Trainer (Server/Peripheral)
        hasConnectPassed = pClient->connect(trainerDevice, false);  // Just refresh the service database 
      } 

    if(!hasConnectPassed) 
      return hasConnectPassed; // Connect failed!

    LOG("Now checking all Services and Characteristics!");
    LOG("If Mandatory Services Fail --> the Client will disconnect!");
    // Discover all relevant Services and Char's
    if( !GAS::getInstance()->client_GenericAccess_Connect(pClient) )
      return hasConnectPassed = false;  
    if( !DIS::getInstance()->client_DeviceInformation_Connect(pClient) )
      return hasConnectPassed = false; 
    if( !CPS::getInstance()->client_CyclingPower_Connect(pClient) )
      return hasConnectPassed = false; 
#ifdef ENABLE_FTMS
    if( !FTMS::getInstance()->client_FitnessMachine_Connect(pClient) )
      return hasConnectPassed = false;
#endif
#ifdef ENABLE_CSC
    if( !CSC::getInstance()->client_CyclingSpeedCadence_Connect(pClient) )
      return hasConnectPassed = false; 
#endif 
#ifdef ENABLE_HRM
    if( !HRM::getInstance()->client_HeartRate_Connect(pClient) )
      return hasConnectPassed = false;
#endif
#ifdef ENABLE_TACXFEC
    if( !FEC::getInstance()->client_FitnessEquipmentCycling_Connect(pClient) )
      return hasConnectPassed = false;
#endif

    // Must do when Connect and Reconnect
    operations->Trainer.IsConnected = true;
    // In case of a trainer RECONNECT and Laptop is (still) connected -> subscribe to all!
    if(operations->Laptop.IsConnected) 
      clientSubscribeToAll(true);
    return hasConnectPassed;
};

void ClientSide::xTaskClientConnectServer(void *parameter) {
      ClientSide* clientInstance = (ClientSide *)parameter; // LOCAL Pointer to store the ClientSide class instance
      if(!clientInstance->clientConnectServer()) {
        LOG(">>> Failed to connect Peripheral (Trainer)!");
        NimBLEDevice::deleteClient(pClient); // If connected: disconnect, delete client object and clear from list
        clientInstance->pClient = nullptr;   // Clear to null
      }
      vTaskDelete(clientInstance->xTaskClientConnectServerHandle);
};

void ClientSide::clientConnectionCallbacksOnConnect(NimBLEClient* pClient) {
    // Get some connection parameters of the peer device.
    const uint16_t clientConnectionHandle = pClient->getConnHandle();
    operations->Trainer.conn_handle = clientConnectionHandle;
    pClient->updateConnParams(ConnectionMinInterval, ConnectionMaxInterval, ConnectionLatency, ConnectionTimeout);
    delay(10);    // Allow some time to settle....
#ifdef EXTENDEDDATALEN
    if( pClient->setDataLen(ExtendedDataLen) ) // Send Extended Data Len Request to the peer (Trainer)
      LOG("Successful Extended Data Len Request!");
    delay(10);    // Allow some time to settle....
#endif
    Presentation::getInstance()->ShowMessageWindow("Client", "Trainer", "Connected!", 0);
#ifdef DEBUG
    const uint16_t client_MTU = pClient->getConnInfo().getMTU();
    const NimBLEAddress remoteAddress = pClient->getConnInfo().getIdAddress();
    LOG("Client connected to Peripheral (Trainer) with Name: [%s] MAC Address: [%s]  MTU: [%d] Handle: [%d]", \
                  trainerDevice->getName().c_str(), UTILS::getInstance()->toString(remoteAddress).c_str(), client_MTU, clientConnectionHandle);
    LOG(" -> Updated Connection Parameters: Min Interval: [%d] Max Interval: [%d] Latency: [%d] Supervision Timeout: [%d]",\
                        ConnectionMinInterval, ConnectionMaxInterval, ConnectionLatency, ConnectionTimeout); 
#endif
};

void ClientSide::clientConnectionCallbacksOnDisconnect(NimBLEClient* pClient, int reason) {
    hasSubscribedToAll = false;
    operations->Trainer.IsConnected = false;
    operations->Trainer.conn_handle = BLE_HS_CONN_HANDLE_NONE; 
    LOG("Client Disconnected from Peripheral (Trainer) with Name: [%s] Mac Address: [%s]",  \
		              operations->Trainer.PeerName.c_str(), UTILS::getInstance()->toString(operations->Trainer.PeerAddress).c_str());
#ifdef CONFIG_NIMBLE_CPP_ENABLE_RETURN_CODE_TEXT
    LOG(" -> Failed Reason [%d][%s]", reason, NimBLEUtils::returnCodeToString(reason));
#endif 
    Presentation::getInstance()->ShowMessageWindow("Trainer", "Lost!", "Scanning!", 0);
    // When the ESP32 Client of the MITM looses connection with the Server (trainer) it needs to signal at its
    // ESP32 Server-side (to the training app) that it is in idle mode until the client reconnects with the trainer!
#ifdef ENABLE_FTMS
    if(operations->Laptop.IsConnected) 
      FTMS::getInstance()->server_Idle_State_Notify();
#endif
    // Create dedicated xTask and start scanning
    if(hasConnectPassed)  { // Only when first connect was successful!
        LOG(" --> Client Restarts Scanning again!");
        xTaskCreatePinnedToCore(this->xTaskClientStartScanning, "Start Scanning", 4096, (void*)this, 10, \
                                &this->xTaskClientStartScanningHandle, xTaskCoreID0); 
    }
};

void ClientSide::xTaskClientStartScanning(void *parameter) {
    ClientSide* clientInstance = (ClientSide *)parameter;
    if(!clientInstance->pNimBLEScan->isScanning()) {
        // Start scanning for undetermined time span -> 0
        // isContinue = false -> clear previous scan results, restart = false -> NO restart if in progress!
        clientInstance->pNimBLEScan->start(0, false, false);  
    }
    vTaskDelete(clientInstance->xTaskClientStartScanningHandle);
};

void ClientSide::xTaskClientSubscribeAll(void *parameter) { 
  ClientSide* clientInstance = (ClientSide *)parameter;
  CPS::getInstance()->client_CP_Subscribe();
#ifdef ENABLE_FTMS
  FTMS::getInstance()->client_FTMS_Subscribe();
#endif
#ifdef ENABLE_CSC
  CSC::getInstance()->client_CSC_Subscribe(); 
#endif 
#ifdef ENABLE_HRM
  HRM::getInstance()->client_HR_Subscribe();
#endif
#ifdef ENABLE_TACXFEC
  FEC::getInstance()->client_FEC_Subscribe();
#endif
  clientInstance->hasSubscribedToAll = true; // Set true after all have been subscribed to !!
  LOG("Client Subscribed to Peripheral (Trainer)!");
  vTaskDelete(clientInstance->xTaskClientSubscribeUnsubscribeHandle);
}; // Subscribe

void ClientSide::xTaskClientUnSubscribeAll(void *parameter) { 
  ClientSide* clientInstance = (ClientSide *)parameter;
  clientInstance->hasSubscribedToAll = false; // Set false now and unsubscribe to all !!
  CPS::getInstance()->client_CP_Unsubscribe();
#ifdef ENABLE_FTMS
  FTMS::getInstance()->client_FTMS_Unsubscribe();
#endif
#ifdef ENABLE_CSC 
  CSC::getInstance()->client_CSC_Unsubscribe();
#endif
#ifdef ENABLE_HRM
  HRM::getInstance()->client_HR_Unsubscribe();
#endif
#ifdef ENABLE_TACXFEC
  FEC::getInstance()->client_FEC_Unsubscribe();
#endif
  LOG("Client Unsubscribed from Peripheral (Trainer)!");
  vTaskDelete(clientInstance->xTaskClientSubscribeUnsubscribeHandle);
}; // UnSubscribe

void ClientSide::clientSubscribeToAll(bool isEnable) {
  if(operations->Trainer.IsConnected) {
    if(isEnable)
      xTaskCreatePinnedToCore(this->xTaskClientSubscribeAll, "xTaskClientSubscribeAll", 4096, (void*)this, 10, \
                  &this->xTaskClientSubscribeUnsubscribeHandle, xTaskCoreID0);
    else
      xTaskCreatePinnedToCore(this->xTaskClientUnSubscribeAll, "xTaskClientUnSubscribeAll", 4096, (void*)this, 10, \
                  &this->xTaskClientSubscribeUnsubscribeHandle, xTaskCoreID0);
  }
};

void ClientSide::startScanning(void) {
  // ---> NimBLEDevice is initialized already at the Server-side <---
  // Retrieve a Scanner and set the callback we want to use to be informed when we
  // have detected a new device. Set scan intervals and specify that we want active scanning 
  pNimBLEScan = NimBLEDevice::getScan();
  pNimBLEScan->setScanCallbacks(new clientScanCallbacks(this));
  pNimBLEScan->setInterval(ScanInterval);
  pNimBLEScan->setWindow(ScanWindow);
  pNimBLEScan->setActiveScan(true); // Get scan responses
  #ifdef ENABLE_FTMS
  LOG("Client Starts Scanning for Peripheral (Trainer) with CPS and FTMS!");
  #else
  LOG("Client Starts Scanning for Peripheral (Trainer) with CPS!");
  #endif
  // Create dedicated xTask and start scanning
  xTaskCreatePinnedToCore(this->xTaskClientStartScanning, "Start Scanning", 4096, (void *)this, 10, \
                          &this->xTaskClientStartScanningHandle, xTaskCoreID0);  
}




