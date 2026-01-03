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

constexpr size_t kPacketSize = 13;

// Generic ANT+ packet
typedef struct {  // — no padding
    uint8_t HEADER[4];  // 4 bytes
    uint8_t BODY[8];    // 8 bytes
    uint8_t CHCKSM;     // 1 byte
} packet_data_t;

typedef union {
    packet_data_t values;
    uint8_t bytes[kPacketSize];
} packet_data_ut;

// Data Page 51 (0x33) – Track Resistance (inside BODY[0..7])
typedef struct {   // — no padding
    uint8_t pageNumber;      // BODY[0] Data Page Number
    uint8_t reserved1;       // BODY[1] 0xFF (reserved for future use)
    uint8_t reserved2;       // BODY[2] 0xFF (reserved for future use)
    uint8_t reserved3;       // BODY[3] 0xFF (reserved for future use)
    uint8_t reserved4;       // BODY[4] 0xFF (reserved for future use)
    uint8_t gradeLSB;        // BODY[5] Grade (Slope) LSB
    uint8_t gradeMSB;        // BODY[6] Grade (Slope) MSB
    uint8_t crr;  			     // BODY[7] Coefficient of Rolling Resistance
} page_0x33_payload_t;

enum FECPageType : uint8_t {
    TRACK_RESISTANCE = 0x33  // Data Page 51 (0x33) – Track Resistance
};

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
  // Generic Packet size is 13 bytes (header 4, body 8 and checksum 1)
  if (FEC_Txd_Data.length() != kPacketSize) {
    LOG("Invalid packet size: expected %d, got %d", kPacketSize, FEC_Txd_Data.length());
    return;
  }
  
  // Transfer to the client side first
  if(pRemote_FEC_Txd_Chr)
    pRemote_FEC_Txd_Chr->writeValue(FEC_Txd_Data, false); // NO Response

  // Decode Rec'd FEC_Txd_Data packet and interpret
  packet_data_ut server_Data;
  memcpy(server_Data.bytes, FEC_Txd_Data.data(), kPacketSize);
  const auto& body = server_Data.values.BODY;	// Create an alias (reference)
  uint8_t pageValue = body[0]; 		  			    // Extract data page number
  
#ifdef DEBUG
  // Parse Generic ANT+ packet for documentation only
  std::string FEC_Txd_Header = FEC_Txd_Data.substr(0, 4); // 4 bytes for header
  std::string FEC_Txd_Body = FEC_Txd_Data.substr(4, 8);   // 8 bytes for body
  // Convert header and body to HEX presentation
  std::string hexHeader = UTILS::getInstance()->toHexString(FEC_Txd_Header);
  std::string hexBody = UTILS::getInstance()->toHexString(FEC_Txd_Body);
  LOG(" -> Server Rec'd FEC Txd Data Page: %d(0x%02X) - [%s][%s][0x%02X]", \
		    pageValue, pageValue, hexHeader.c_str(), hexBody.c_str(), server_Data.values.CHCKSM);
#endif

  switch(pageValue) {
    case TRACK_RESISTANCE : 
	  // Interpret data page 0x33 content -> Overlay first the specific page 0x33 struct
	  const page_0x33_payload_t* page33 = reinterpret_cast<const page_0x33_payload_t*>(&body); 
	  uint16_t rawGrade = (static_cast<uint16_t>(page33->gradeMSB) << 8) | page33->gradeLSB;
    float gradePercentValue = (static_cast<float>(rawGrade - 20000))/100.0;   // Units: 0.01
    operations->setNewGrade(gradePercentValue);
#ifdef DEBUG
	  float crr = (static_cast<float>(page33->crr))*5/100000.0;  // Units: 5x0.000001 - Range: 0.0 – 0.0127
    LOG("    Raw Grade: %05d Grade Percentage: %6.2f%% crr: %6.4f", rawGrade, gradePercentValue, crr);
#endif 
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
      LOG("Fitness Equipment Cycling Service: Not Found!");
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
      LOG("Mandatory Client_FEC_Txd_Chr: Cannot Write!");
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