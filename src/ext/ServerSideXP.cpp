#include "ServerSideXP.h"

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress DEBUG messages that help debugging...
// Uncomment general "#define DEBUG" to activate debug messages from Class ServerSideXP
#define DEBUG
// Include these debug utility macros in all cases!
#include <config/configDebug.h>
// ------------------------------------------------------------------------------------------------

#include <Utilities.h>
#include <Operations.h>
#include <Presentation.h>
#include <GenericAccess.h>
#include <DeviceInformation.h>
#include <NordicUart.h>
#include <CyclingPower.h>
#ifdef ENABLE_FTMS
#include <FitnessMachine.h>
#endif
#ifdef ENABLE_HRM
#include <HeartRateMonitor.h>
#endif
#ifdef ENABLE_CSC
#include <CyclingSpeedCadence.h>
#endif

// Initialize the static members
ServerSideXP* ServerSideXP::instance = nullptr;

// Declaration of Public overriding ServerSideXP Members --------------------------------------------------------------

void ServerSideXP::serverConnectionCallbacksOnConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
    // Get some connection parameters of the peer device.
    const uint16_t serverConnectionHandle = connInfo.getConnHandle();
    const uint16_t serverConnectionInterval = connInfo.getConnInterval();   // Connection interval   
    const uint16_t serverConnectionLatency = connInfo.getConnLatency(); // Connection latency
    const uint16_t serverConnectionSupTimeout = connInfo.getConnTimeout();   // Connection supervision timeout
    const NimBLEAddress remoteAddress = connInfo.getIdAddress();
    pAdvertising->stop();

    // Check if a Laptop is connecting with Public Address
    if(remoteAddress.isPublic()) { // BLE_ADDR_TYPE_PUBLIC (0)  -  BLE_ADDR_TYPE_RANDOM (1)
      LOG(">>> Forced Server Disconnect: Laptop Mac Address [%s]", \
                    UTILS::getInstance()->toString(remoteAddress).c_str());
      pServer->advertiseOnDisconnect(false);       // Set server auto-restart advertise OFF
      forcedDisconnect = true;
      pServer->disconnect(serverConnectionHandle); // Disconnect the Laptop      
      return; // Wrong Device, we are done! 
    }
    if(remoteAddress.isRpa()) { 
      NimBLEDevice::startSecurity(serverConnectionHandle);
      LOG("Handle Smartphone with Resolvable Private Address!");
      const uint16_t peer_MTU = pServer->getPeerMTU(serverConnectionHandle);
      LOG("ESP32 ServerXP connects to Central (Phone) with MAC Address: [%s] MTU: [%d] Conn Handle: [%d]", \
                  UTILS::getInstance()->toString(remoteAddress).c_str(), peer_MTU, serverConnectionHandle); 
      LOG(" -> Default Connection Parameters: Interval: [%d] Latency: [%d] Supervision Timeout: [%d]",\
                  serverConnectionInterval, serverConnectionLatency, serverConnectionSupTimeout);
#ifdef EXTENDEDDATALEN
      pServer->setDataLen(serverConnectionHandle, ExtendedDataLen); // Send Extended Data Len Request to the peer (Zwift/Laptop)
      vTaskDelay(50/portTICK_PERIOD_MS); // Allow some time to settle....
#endif
      operations->Smartphone.conn_handle = serverConnectionHandle;
      operations->Smartphone.IsConnected = true;
      operations->Smartphone.PeerAddress = remoteAddress;
      LOG("Central (%s/Simcline App) has to Subscribe to NUS and start..", operations->Smartphone.PeerName.c_str());
      Presentation::getInstance()->ShowMessageWindow("ServerXP", "Smartphone", "Connected!", 0);
      return; // Wrong Device, we are done! 
    }
    LOG(">>> Forced Server Disconnect: Unresolved Mac Address [%s]", \
                    UTILS::getInstance()->toString(remoteAddress).c_str());
    pServer->advertiseOnDisconnect(false);       // Set server auto-restart advertise OFF
    forcedDisconnect = true;
    pServer->disconnect(serverConnectionHandle); // Disconnect Wrong Device, we are done! 
};

void ServerSideXP::serverConnectionCallbacksOnDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
    // Get some Disconnection parameters of the peer device.
    const uint16_t serverConnectionHandle = connInfo.getConnHandle();
    NimBLEAddress remoteAddress = connInfo.getIdAddress();;
    if (operations->Smartphone.conn_handle == serverConnectionHandle ) { // Smartphone is disconnected
      operations->Smartphone.conn_handle = BLE_HS_CONN_HANDLE_NONE;
      operations->Smartphone.IsConnected = false;
      LOG("ESP32 ServerXP disconnected from Central (%s) Conn handle: [%d] Mac Address: [%s]", \
                         operations->Smartphone.PeerName.c_str(), \
			 serverConnectionHandle, UTILS::getInstance()->toString(remoteAddress).c_str());
#ifdef CONFIG_NIMBLE_CPP_ENABLE_RETURN_CODE_TEXT
      LOG(" -> Failed Reason [%d][%s]", reason, NimBLEUtils::returnCodeToString(reason));
#endif
    }
    Presentation::getInstance()->ShowMessageWindow("Smartphone", "Lost!", "Advertise!", 0);
    // NimBLe does auto advertise after disconnect, unless switched off
    if(forcedDisconnect) LOG(">>> ESP32 Server stopped re-advertising!");
    else LOG(" --> ESP32 ServerXP is advertising again!");
};

// Constructor
ServerSideXP::ServerSideXP() : ServerSide() {  }

// Destructor
ServerSideXP::~ServerSideXP() { }

ServerSideXP* ServerSideXP::getInstance() {
    if (instance == nullptr) {
        instance = new ServerSideXP();
    }
    return instance;
}

void ServerSideXP::start(void) {
  // Setup the ServerSideXP!
  ServerSideXP::getInstance()->init();
  // Advertise the ServerSideXP and find a SmartPhone to connect with!
  ServerSideXP::getInstance()->startAdvertising();
}
