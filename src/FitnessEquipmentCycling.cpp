#include "FitnessEquipmentCycling.h"

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress DEBUG messages that help debugging...
// Uncomment general "#define DEBUG" to activate debug messages for this class
#define DEBUG
// Include these debug utility macros in all cases!
#include "config/configDebug.h"
// ------------------------------------------------------------------------------------------------

#ifdef DEBUG
//  Restrict activating one or more of the following DEBUG directives --> process intensive 
//  The overhead can lead to spurious side effects and a loss of quality of service handling!!
//  Notice: Training Apps hardly ever subscribe to server_FEC_Rxd_Chr, they use CPS and CSC data instead!
//#define DEBUG_FEC_RXD
#endif

// Include operations control settings class
#include "Operations.h"

///////////////////////////////////////////////
/////////// TACX FE-C ANT+ over BLE ///////////
///////////////////////////////////////////////
const NimBLEUUID UUID_TACX_FEC_PRIMARY_SERVICE("6E40FEC1-B5A3-F393-E0A9-E50E24DCCA9E");
const NimBLEUUID UUID_TACX_FEC_RXD_CHARACTERISTIC("6E40FEC2-B5A3-F393-E0A9-E50E24DCCA9E");
const NimBLEUUID UUID_TACX_FEC_TXD_CHARACTERISTIC("6E40FEC3-B5A3-F393-E0A9-E50E24DCCA9E");

class server_FEC_Rxd_Chr_callbacks final : public NimBLECharacteristicCallbacks {
private:
    FEC* fecInstance;  // LOCAL Pointer to the FEC class instance
public:
    // Constructor to accept a pointer to the FEC class instance
    server_FEC_Rxd_Chr_callbacks(FEC* instance) : fecInstance(instance) {}
protected:
    inline void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
        fecInstance->serverFECRxdOnSubscribe(pCharacteristic, connInfo, subValue);
    } // onSubscribe
}; 

class server_FEC_Txd_Chr_callback final : public NimBLECharacteristicCallbacks {
private:
    FEC* fecInstance;  // LOCAL Pointer to the FEC class instance
public:
    // Constructor to accept a pointer to the FEC class instance
    server_FEC_Txd_Chr_callback(FEC* instance) : fecInstance(instance) {}
protected:
    inline void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        fecInstance->serverFECTxdOnWrite(pCharacteristic, connInfo);
    }
};

// Initialize the static member
FEC* FEC::instance = nullptr;

/*  We only define onSubscribe !!!
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo);
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo); 
    void onStatus(NimBLECharacteristic* pCharacteristic, int code)); 

    Notice: Training Apps hardly ever subscribe to server_FEC_Rxd_Chr, they use CPS and CSC data instead!   
*/ 
void FEC::serverFECRxdOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) {
    std::string str = std::string(pCharacteristic->getUUID());
    if(subValue == 0) {
      LOG("Central Unsubscribed: [%s] FEC Rxd", str.c_str());
      isServerFECRxdNotifyEnabled = false;
    } else {
      LOG("Central Subscribed: [%s] FEC Rxd", str.c_str());
      isServerFECRxdNotifyEnabled = true;
    }
}

void FEC::serverFECTxdOnWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo& connInfo) {
  std::string FEC_Txd_Data = server_FEC_Txd_Chr->getValue();
  //size_t TxdDatalen = FEC_Txd_Data.length(); // Packet length is 13 bytes: header 4, body 8 and checksum 1
  // Transfer to the client side first
  if(pRemote_FEC_Txd_Chr)
    pRemote_FEC_Txd_Chr->writeValue(FEC_Txd_Data, false);
  // Decode Rec'd FEC_Txd_Data and interpret
  uint8_t pageValue = static_cast<uint8_t>(FEC_Txd_Data[4]); // Extract page number
#ifdef DEBUG
  std::string FEC_Txd_Body = FEC_Txd_Data.substr(4, 8); // skip header -> 8 bytes for body
  // Convert body to HEX presentation
  std::string hexBody = UTILS::getInstance()->toHexString(FEC_Txd_Body);
  LOG(" -> Server Rec'd FEC Txd Data Page %d (0x%02X) Body [%s]", pageValue, pageValue, hexBody.c_str());
#endif
  switch(pageValue) {
    case 0x33 : // Data Page 51 (0x33) – Track Resistance
      uint8_t lsb_gradeValue = static_cast<uint8_t>(FEC_Txd_Data[9]);   // Extract
      uint8_t msb_gradeValue = static_cast<uint8_t>(FEC_Txd_Data[10]);  // Extract
      long RawgradeValue = lsb_gradeValue + msb_gradeValue*256;         // Combine
      float gradePercentValue = float((RawgradeValue - 20000))/100.0;   // Resolution 0.01
      operations->setNewGrade(gradePercentValue);
      LOG("    RawgradeValue: %05d Grade percentage: %6.2f%%", RawgradeValue, gradePercentValue);
  }
};

// Constructor
FEC::FEC() { 
  operations = OPS::getInstance();
}

// Destructor
FEC::~FEC() { }

FEC* FEC::getInstance() {
    if (instance == nullptr) {
        instance = new FEC();
    }
    return instance;
}

void FEC::server_setupFEC(NimBLEServer* pServer) {
  server_FitnessEquipmentCycling_Service = pServer->createService(UUID_TACX_FEC_PRIMARY_SERVICE);
  server_FEC_Rxd_Chr = server_FitnessEquipmentCycling_Service->createCharacteristic(UUID_TACX_FEC_RXD_CHARACTERISTIC, \
                                                                                                   NIMBLE_PROPERTY::NOTIFY);
  server_FEC_Rxd_Chr->setCallbacks(new server_FEC_Rxd_Chr_callbacks(this));
  server_FEC_Txd_Chr = server_FitnessEquipmentCycling_Service->createCharacteristic(UUID_TACX_FEC_TXD_CHARACTERISTIC, \
                                                                        NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::WRITE_NR);
  server_FEC_Txd_Chr->setCallbacks(new server_FEC_Txd_Chr_callback(this));
  server_FitnessEquipmentCycling_Service->start();    
}

void FEC::server_FEC_Rxd_Chr_Notify(uint8_t* pData, size_t length) {
  if(isServerFECRxdNotifyEnabled) {
    server_FEC_Rxd_Chr->setValue(pData, length);
    server_FEC_Rxd_Chr->notify();
  }
}

void FEC::client_FEC_Rxd_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, \
                                                                                            size_t length, bool isNotify) {
  // Notice: Training Apps hardly ever subscribe to server_FEC_Rxd_Chr, they use CPS and CSC data instead!
  // Client FEC Rxd data is tranferred to the Server (if enabled!)
  if(operations->Laptop.IsConnected) {
    server_FEC_Rxd_Chr_Notify(pData, length);
  }

// Tacx FEC Trainer sends at regular intervals the following pages to document the user's bike effort!
// Data Page 16 (0x10) – General FE Data
// Data Page 25 (0x19) - Specific Trainer/Stationary Bike Data

#ifdef DEBUG_FEC_RXD
  uint8_t buffer[length]= {}; 
  // Transfer first the contents of data to buffer (array of chars)
  memcpy(buffer, pData, length);
  uint8_t pageValue = static_cast<uint8_t>(buffer[4]);
  std::string hexString = UTILS::getInstance()->toHexString(buffer, static_cast<uint8_t>(length));
  LOG(" -> Client Rec'd FEC Rxd Data Page %d (0x%02X) Raw: [%s]", pageValue, pageValue, hexString.c_str());
#endif
}

void FEC::Static_FEC_Rxd_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                                          uint8_t* pData, size_t length, bool isNotify) {
    if (instance) {
        instance->client_FEC_Rxd_Notify_Callback(pBLERemoteCharacteristic, pData, length, isNotify);
    }
}

bool FEC::client_FitnessEquipmentCycling_Connect(NimBLEClient* pClient) {
    // Obtain a reference to the remote FEC service.
    pRemote_FitnessEquipmentCycling_Service = pClient->getService(UUID_TACX_FEC_PRIMARY_SERVICE);
    if (pRemote_FitnessEquipmentCycling_Service == nullptr) {
      LOG("Cycling Speed Cadence Service: Not Found!");
      return false;
    }
    LOG("Client_FitnessEquipmentCycling_Service: Found!");
    pRemote_FEC_Rxd_Chr = pRemote_FitnessEquipmentCycling_Service->getCharacteristic(UUID_TACX_FEC_RXD_CHARACTERISTIC);
    if (pRemote_FEC_Rxd_Chr == nullptr) {
      LOG("Mandatory client_FEC_Rxd_Chr: Not Found!");
      return false;
    }
    LOG("Client_FEC_Rxd_Chr: Found!");  
    if(!pRemote_FEC_Rxd_Chr->canNotify()) {
      LOG("Mandatory Client_FEC_Rxd_Chr: Cannot Notify!");
      return false;
    }
    pRemote_FEC_Txd_Chr = pRemote_FitnessEquipmentCycling_Service->getCharacteristic(UUID_TACX_FEC_TXD_CHARACTERISTIC);
    if (pRemote_FEC_Txd_Chr == nullptr) {
      LOG("Mandatory Client_FEC_Txd_Chr: Not Found!");
      return false;     
    }
    LOG("Client_FEC_Txd_Chr: Found!");
    if(!pRemote_FEC_Txd_Chr->canWrite()) {
      LOG("Mandatory Client_FEC_Rxd_Chr: Cannot Write!");
      return false;
    }
    return true;    
}

void FEC::client_FEC_Subscribe(void) {
  if( pRemote_FEC_Rxd_Chr != nullptr ) 
    if( !pRemote_FEC_Rxd_Chr->subscribe(notifications, Static_FEC_Rxd_Notify_Callback) ) 
      LOG(">>> ERROR remote FEC Rxd Subscribe failed!");                        
}

void FEC::client_FEC_Unsubscribe(void) {
  if ( pRemote_FEC_Rxd_Chr != nullptr )
    if( !pRemote_FEC_Rxd_Chr->unsubscribe(false) )
      LOG(">>> ERROR remote FEC Rxd Subscribe failed!"); 
}