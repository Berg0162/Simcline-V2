#include "ServerSide.h"

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress DEBUG messages that help debugging...
// Uncomment general "#define DEBUG" to activate debug messages for this class
#define DEBUG
// Include these debug utility macros in all cases!
#include "config/configDebug.h"
// ------------------------------------------------------------------------------------------------

#define UUID16_SVC_CYCLING_POWER              NimBLEUUID((uint16_t)0x1818)
#define UUID16_SVC_FITNESS_MACHINE            NimBLEUUID((uint16_t)0x1826)
#define UUID128_SVC_TACX_FEC                  NimBLEUUID("6E40FEC1-B5A3-F393-E0A9-E50E24DCCA9E")

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
#endif
#include "NordicUart.h"
#include "ClientSide.h"
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
NimBLEServer* ServerSide::pServer = nullptr;
ServerSide* ServerSide::instance = nullptr;

// ------------------------------------------------------------------------------------------------

// Server Connect and Disconnect callbacks defined
class server_Connection_Callbacks : public NimBLEServerCallbacks {
private:
    ServerSide* serverInstance;  // LOCAL Pointer to store the ServerSide class instance
public:
    // Constructor to accept a pointer to the ServerSide class instance
    server_Connection_Callbacks(ServerSide* instance) : serverInstance(instance) {}
protected: 
  void onConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) override {
    serverInstance->serverConnectionCallbacksOnConnect(pServer, connInfo);
  }
  void onDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) override {
    serverInstance->serverConnectionCallbacksOnDisconnect(pServer, connInfo, reason);
  }
  void onMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) override {
    serverInstance->serverConnectionCallbacksOnMTUChange(MTU, connInfo);
  }
  void onAuthenticationComplete(NimBLEConnInfo& connInfo) override {
      serverInstance->serverSecurityCallbacksOnAuthenticationComplete(connInfo);
  }
};

void ServerSide::serverConnectionCallbacksOnConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo) {
    // Get some connection parameters of the peer device.
    const uint16_t serverConnectionHandle = connInfo.getConnHandle();
    NimBLEAddress remoteAddress = connInfo.getIdAddress();
    pAdvertising->stop();
    const uint16_t peer_MTU = pServer->getPeerMTU(serverConnectionHandle);
    pServer->updateConnParams(serverConnectionHandle, ConnectionMinInterval, ConnectionMaxInterval, \
                                  ConnectionLatency, ConnectionTimeout);
    delay(10); // Allow time to settle
    LOG("Server connects to Central (Laptop/Phone) MAC Address: [%s] MTU: [%d] Conn Handle: [%d]", \
                                      UTILS::getInstance()->toString(remoteAddress).c_str(), peer_MTU, serverConnectionHandle);    
    LOG(" -> Updated Connection Parameters: Min Interval: [%d] Max Interval: [%d] Latency: [%d] Supervision Timeout: [%d]",\
                                  ConnectionMinInterval, ConnectionMaxInterval, ConnectionLatency, ConnectionTimeout);
#ifdef EXTENDEDDATALEN
    pServer->setDataLen(serverConnectionHandle, ExtendedDataLen); // Send Extended Data Len Request to the peer (Zwift/Laptop)
    LOG("Server sends an Extended Data Len Request!");
    delay(10); // Allow some time to settle....
#endif
    // ------------------------ Which Client has been exactly connected? ---------------------------------
    // [1] Smartphone is connecting ----------------------------------------------------------------------
    if(remoteAddress.isRpa() ) { // Check for typical Phone RPA
      NimBLEDevice::startSecurity(serverConnectionHandle);
      LOG("Handle Smartphone with Resolvable Private Address!");
      operations->Smartphone.PeerAddress = remoteAddress;
      operations->Smartphone.conn_handle = serverConnectionHandle;
      operations->Smartphone.IsConnected = true;
      LOG("Central (%s/Simcline App) has to Subscribe to Characteristics and start..", \
                     operations->Smartphone.PeerName.c_str());
      Presentation::getInstance()->ShowMessageWindow("Server", "Phone", "Connected!", 0);
      return; // We are done here!
    }
    // [2] Laptop is connecting ---------------------------------------------------------------------------
    if(remoteAddress != operations->Laptop.PeerAddress) { // Does NOT equal known Laptop MAC address sofar...
      if(operations->enableUnknownDev) { // Only when allowed handle this one time exception
        operations->Laptop.PeerAddress = remoteAddress; // Set Unknown Mac Address to Laptop.PeerAddress
        LOG("Unknown Mac Address registered for Central (Laptop)");
    	  UTILS::getInstance()->setMacAddressNVS("Laptop", remoteAddress); 	// Store Laptop Mac Address in NVS
      } else { // [3] In MITM mode Unknown device is connecting ------------------------------------------
        LOG(">>> ERROR Forced Server Disconnect: Unknown Laptop Mac Address!");
  	    pServer->advertiseOnDisconnect(false); // Set server auto-restart advertise OFF
        forcedDisconnect = true;
        pServer->disconnect(serverConnectionHandle); // Disconnect the UNKNOWN Device       
        return; // We are done, this is a fatal error state! 
      }
    }
    // [4] Known Laptop MAC address!
    operations->Laptop.conn_handle = serverConnectionHandle;       
    operations->Laptop.IsConnected = true;
    LOG("Central (%s/Zwift) has to Subscribe to Characteristics and start..", \
                    operations->Laptop.PeerName.c_str());
    Presentation::getInstance()->ShowMessageWindow("Server", "Laptop", "Connected!", 0);
    if(operations->Trainer.IsConnected)
      ClientSide::getInstance()->clientSubscribeToAll(true); // Tell the client to send data now! 
};

void ServerSide::serverConnectionCallbacksOnMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo) {
    // Get all connection parameters of the peer device
    uint16_t serverConnectionHandle = connInfo.getConnHandle();
#ifdef DEBUG
    LOG("Central (%s/Zwift) updated MTU [%d] for connection ID: %d", operations->Laptop.PeerName.c_str(), MTU, serverConnectionHandle); 
#endif      
};

void ServerSide::serverConnectionCallbacksOnDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason) {
    // Get some Disconnection parameters of the peer device.
    uint16_t serverConnectionHandle = connInfo.getConnHandle();
    NimBLEAddress remoteAddress = connInfo.getIdAddress();
    if(operations->Laptop.conn_handle == serverConnectionHandle) { // Laptop/Desktop is disconnected
      operations->Laptop.conn_handle = BLE_HS_CONN_HANDLE_NONE;
      operations->Laptop.IsConnected = false;
      LOG("Server disconnected from Central (%s) Conn handle: [%d] Mac Address: [%s]", operations->Laptop.PeerName.c_str(), \
			              serverConnectionHandle, UTILS::getInstance()->toString(remoteAddress).c_str());
#ifdef CONFIG_NIMBLE_CPP_ENABLE_RETURN_CODE_TEXT
      LOG(" -> Failed Reason [%d][%s]", reason, NimBLEUtils::returnCodeToString(reason));
#endif
      Presentation::getInstance()->ShowMessageWindow("Laptop", "Lost!", "Advertise!", 0);
      if(operations->Trainer.IsConnected) 
        ClientSide::getInstance()->clientSubscribeToAll(false); // Tell the client not to send data!
    }
    if (operations->Smartphone.conn_handle == serverConnectionHandle ) { // Smartphone is disconnected
      operations->Smartphone.conn_handle = BLE_HS_CONN_HANDLE_NONE;
      operations->Smartphone.IsConnected = false;
      LOG("Server disconnected from Central (%s) Conn handle: [%d] Mac Address: [%s]", operations->Smartphone.PeerName.c_str(), \
			              serverConnectionHandle, UTILS::getInstance()->toString(remoteAddress).c_str());
      Presentation::getInstance()->ShowMessageWindow("Phone", "Lost!", "Advertise!", 0);
    }
    // NimBLe does auto advertise after disconnect, unless switched off
    if(forcedDisconnect) LOG(">>> Server stopped re-advertising!");
    else LOG(" --> Server is advertising again!");
};

void ServerSide::serverSecurityCallbacksOnAuthenticationComplete(NimBLEConnInfo& connInfo) {
#ifdef DEBUG
  std::string str;
  if (connInfo.isAuthenticated()) str += " --> Authenticated!";
  else str += " --> Not Authenticated!"; 
  if (connInfo.isEncrypted()) str += " --> Encrypted!";
  else str += " --> Not Encrypted!";
  if (connInfo.isBonded()) str += " --> Bonded!";
  LOG(" >> Security Completed! %s", str.c_str());
#endif
};

// Constructor
  ServerSide::ServerSide() {  
    //pServerCallbacks = new server_Connection_Callbacks(this);
    operations = OPS::getInstance(); 
}

// Destructor
ServerSide::~ServerSide() {
	  //delete pServerCallbacks;
}

ServerSide* ServerSide::getInstance() {
    if (instance == nullptr) {
        instance = new ServerSide();
    }
    return instance;
}

void ServerSide::init(void) {
  LOG("Initializing the ServerSide");
  // Initialize NimBLE with maximum connections as Peripheral = 1, Central = 1
  NimBLEDevice::init(THISDEVICENAME);  // Give the device a Shortname
  // Set MTU to be negotiated during request from client peer
  if( NimBLEDevice::setMTU(ConnectionMTU) ) { 
    LOG("Preferred connection MTU succesfully set to: %d", ConnectionMTU);
  }
  // Setup Security
  NimBLEDevice::setSecurityAuth(false, true, true);  // NO BONDING at ESP32 side
  NimBLEDevice::setSecurityPasskey(123456);
  NimBLEDevice::setSecurityIOCap(BLE_HS_IO_DISPLAY_ONLY);
  // Set the ESP-board hardware static address to PUBLIC
  NimBLEDevice::setOwnAddrType(BLE_OWN_ADDR_PUBLIC);

  pServer = NimBLEDevice::createServer();
  // Setup callbacks for onConnect, onDisconnect a.o.
  pServer->setCallbacks(new server_Connection_Callbacks(this));
  // Set server auto-restart advertise on
  pServer->advertiseOnDisconnect(true); 
  forcedDisconnect = false;

  // Server Characteristics setup
  LOG("Configuring the default Generic Access Service");
  GAS::getInstance()->server_setupGAS(pServer);
  LOG("Configuring the Server Device Information Service");
  DIS::getInstance()->server_setupDIS(pServer);
  LOG("Configuring the Server Cycle Power Service");
  CPS::getInstance()->server_setupCPS(pServer);
#ifdef ENABLE_FTMS
  LOG("Configuring the Server Fitness Machine Service");
  FTMS::getInstance()->server_setupFTMS(pServer);
#endif
  LOG("Configuring the Server Nordic Uart Service"); 
  NUS::getInstance()->server_setupNUS(pServer);
#ifdef ENABLE_CSC
  LOG("Configuring the Server Cadence and Speed Service");  
  CSC::getInstance()->server_setupCSC(pServer);
#endif
#ifdef ENABLE_HRM
  LOG("Configuring the Server Heart Rate Service");  
  HRM::getInstance()->server_setupHRM(pServer);
#endif
#ifdef ENABLE_TACXFEC
  LOG("Configuring the Server Fitness Equipment Cycling Service");  
  FEC::getInstance()->server_setupFEC(pServer);
#endif
  // Start the GATT server. Required to be called after setup of all services and 
  // characteristics / descriptors for the NimBLE host to register them.
  LOG("Server is setup and started!"); 
  pServer->start();
  LOG("Setting up advertising payload(s)");
  setupAdvertising();
}

void ServerSide::startAdvertising(void) {
    LOG("Server is advertising!"); 
    pAdvertising->start();
};

void ServerSide::setupAdvertising(void) {
    // Prepare for advertising
    // Optional: set the transmit power, default is 3db
    NimBLEDevice::setPower(9); // +9db 
    pAdvertising = NimBLEDevice::getAdvertising(); 
    pAdvertising->addServiceUUID(UUID16_SVC_CYCLING_POWER);
    LOG("Setting Service in Advertised data to    [CPS]");
#ifdef ENABLE_FTMS
    if( pAdvertising->addServiceUUID(UUID16_SVC_FITNESS_MACHINE) )
      LOG("Setting Service in Advertised data to    [FTMS]");
#endif
#ifdef ENABLE_TACXFEC
    // Put 128-bit UUID in Scan Response
    NimBLEAdvertisementData scanResp;
    scanResp.addServiceUUID(UUID128_SVC_TACX_FEC);
    pAdvertising->setScanResponseData(scanResp);
    LOG("Setting Service in Scan Response to      [FEC]");
#endif
    if( pAdvertising->setAppearance(GAS::getInstance()->client_GA_Appearance_Value) )
      LOG("Setting Appearance in Advertised data to [%d]", GAS::getInstance()->client_GA_Appearance_Value);
    if( pAdvertising->setName(THISDEVICENAME) )
      LOG("Setting DeviceName in Advertised data to [%s]", THISDEVICENAME);
    // Add the transmission power level to the advertisement packet.
    if( pAdvertising->addTxPower() ) 
      LOG("Setting TxPower in Advertised data");
    pAdvertising->enableScanResponse(true); // Respond to active scans
    pAdvertising->setMinInterval(AdvertiseMinInterval); 
    pAdvertising->setMaxInterval(AdvertiseMaxInterval); 
}

