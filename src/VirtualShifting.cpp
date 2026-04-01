#include "VirtualShifting.h"

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
#define DEBUG_ASYNC
#define DEBUG_ONWRITE
#endif

// Unsigned little endian base 128 (ULEB128) implementation, 
// which stores arbitrarily large unsigned integers in a variable length format. 
#include <uleb128.h>

// ------------------------------------------------------------------------------------------------
#include "Utilities.h"
// Include operations control settings class
#include "Operations.h"

// ------------------------------------------------------------------------------------
// Server Virtual Shifting ------------------------------------------------------------
// ------------------------------------------------------------------------------------
#ifdef TRAINER_WITH_LEGACY_ZVS_SERVICE
#define CLIENT_ZWIFT_VIRTUAL_SHIFTING_SERVICE NimBLEUUID("00000001-19ca-4651-86e5-fa29dcdd09d1")
#else
#define CLIENT_ZWIFT_VIRTUAL_SHIFTING_SERVICE NimBLEUUID((uint16_t)0xFC82)
#endif

#define ZWIFT_VIRTUAL_SHIFTING_SERVICE NimBLEUUID((uint16_t)0xFC82)
static NimBLEUUID ZWIFT_ASYNC_CHARACTERISTIC_UUID("00000002-19CA-4651-86E5-FA29DCDD09D1");   // Measurement
static NimBLEUUID ZWIFT_SYNCRX_CHARACTERISTIC_UUID("00000003-19CA-4651-86E5-FA29DCDD09D1");  // Control Point (commands)
static NimBLEUUID ZWIFT_SYNCTX_CHARACTERISTIC_UUID("00000004-19CA-4651-86E5-FA29DCDD09D1");  // Response (to commands)

// Initialize the static member
ZVS* ZVS::instance = nullptr;

class server_VS_ASYNC_Chr_callbacks final : public NimBLECharacteristicCallbacks {
private:
    ZVS* zvsInstance;  // LOCAL Pointer to the ZVS class instance
public:
    // Constructor to accept a pointer to the ZVS class instance
    server_VS_ASYNC_Chr_callbacks(ZVS* instance) : zvsInstance(instance) {}
protected:
    inline void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
        zvsInstance->serverVSASYNCOnSubscribe(pCharacteristic, connInfo, subValue);
    } // onSubscribe
}; 

class server_VS_SYNCTX_Chr_callbacks final : public NimBLECharacteristicCallbacks {
private:
    ZVS* zvsInstance;  // LOCAL Pointer to the ZVS class instance
public:
    // Constructor to accept a pointer to the ZVS class instance
    server_VS_SYNCTX_Chr_callbacks(ZVS* instance) : zvsInstance(instance) {}
protected:
    inline void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
        zvsInstance->serverVSSYNCTXOnSubscribe(pCharacteristic, connInfo, subValue);
    } // onSubscribe
}; 

class server_VS_SYNCRX_Chr_callbacks final : public NimBLECharacteristicCallbacks {
private:
    ZVS* zvsInstance;  // LOCAL Pointer to the ZVS class instance
public:
    // Constructor to accept a pointer to the ZVS class instance
    server_VS_SYNCRX_Chr_callbacks(ZVS* instance) : zvsInstance(instance) {}
protected:
    inline void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        zvsInstance->serverVSSYNCRXOnWrite(pCharacteristic, connInfo);
    }
};

void ZVS::serverVSSYNCRXOnWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) {
    std::string syncRXData = pCharacteristic->getValue();
    size_t syncRXDataLen = syncRXData.length();
    if(syncRXDataLen == 0) return;

    // Pass on Data to the client-side
    if(!write_pRemote_VS_SYNCRX_Chr(syncRXData))
      LOG("ERROR >> write_pRemote_VS_SYNCRX_Chr Failed!");

    // Convert std::string to std::vector<uint8_t>
    std::vector<uint8_t> requestData(syncRXData.begin(), syncRXData.end());
    LOG("Zwift request: %s", utils->getHexString(requestData).c_str());
    
#ifdef TRAINER_WITH_LEGACY_ZVS_SERVICE
    decodeLegacyZVSRequest(requestData);
#else
    decodeNewZVSRequest(requestData);
#endif
};

void ZVS::decodeLegacyZVSRequest(const std::vector<uint8_t>& data) {
    // Basic sanity check
    if (data.size() < 5) {
        return; // Not a Zwift VS Legacy request to decode
    }
    // Accept either Wahoo Kickr or Zwift Hub legacy request
    if (data[0] != 0x3F && data[0] != 0x04) {
        return;
    }
    
    // Zwift Hub: Set Simulation Parameter
    if (data[0] == 0x04 && data[1] == 0x22 && data[3] == 0x10) {
        float zwiftGrade = 0.0f;
        // 8-bit grade [04 22 02 10 69]
        if (data[2] == 0x02 && data.size() >= 5) {
            uint8_t value = data[4];
            zwiftGrade = (float)value * 0.025f;
        }
        // 16-bit grade [04 22 03 10 97 01]
        if (data[2] == 0x03 && data.size() >= 6) {
            uint16_t value = data[4] | (data[5] << 8);
            zwiftGrade = (float)value * 0.0025f;
        }
        LOG(" -> Zwift Road Grade: %.2f", zwiftGrade);
        operations->setNewGrade(zwiftGrade); // grade in percentage resolution 0.01
        return;
    }
    // Zwift Hub done!
    
    // Restrict further decoding to ID (0x3F)
    if (data[0] != 0x3F) return;
    // Wahoo Kickr a.o.: Set Simulation Parameter and Set Gear Ratio
    size_t offset = 1; // Skip request ID (0x3F)
    while (offset < data.size()) {
        uint8_t tag = data[offset++];
        // Defensive bounds check
        if (offset >= data.size()) {
            break;
        }
        uint64_t uval = 0;
        size_t consumed = bfs::DecodeUleb128(data.data() + offset, data.size() - offset, &uval);
        // Invalid or truncated ULEB128 → abort safely
        if (consumed == 0 || offset + consumed > data.size()) {
            break;
        }
        offset += consumed;
        // ---- Road Grade --------------------------------------------------
        if (tag == 0x28) {
            int64_t zwiftGrade = static_cast<int64_t>(uval);
            // ZigZag decode
            zwiftGrade = (zwiftGrade >> 1) ^ -(zwiftGrade & 1);
            float grade = (float)zwiftGrade / 100.0f;
            LOG(" -> Zwift Road Grade: %6.2f", grade);
            operations->setNewGrade(grade); // grade in percentage resolution 0.01
        }
        // ---- Gear Ratio --------------------------------------------------
        else if (tag == 0x38) {
#ifdef DEBUG_ONWRITE
            uint64_t zwiftGearRatio = uval;
            double gearRatio = (double)zwiftGearRatio / 10000.0f;
            int currentGear = utils->getGearNumberFromRatio(gearRatio);
            LOG(" -> Zwift Gear Ratio: %4.2f -> Gear: %d", gearRatio, currentGear);
#endif
        }
        // ---- Unknown / ignored tags -------------------------------------
        else {
            // Intentionally ignored
            // Future-proof: safely skip unknown fields
        }
    } // while
};


void ZVS::decodeNewZVSRequest(const std::vector<uint8_t>& data) {
    // Basic sanity check
    if (data.empty() || data[0] != 0x04) {
        return; // Not a Zwift VS Legacy request
    }
    size_t offset = 1; // Skip request ID (0x3F)

    while (offset < data.size()) {
        uint8_t tag = data[offset++];
        // Defensive bounds check
        if (offset >= data.size()) {
            break;
        }
        uint64_t uval = 0;
        size_t consumed = bfs::DecodeUleb128(data.data() + offset, data.size() - offset, &uval);
        // Invalid or truncated ULEB128 → abort safely
        if (consumed == 0 || offset + consumed > data.size()) {
            break;
        }
        offset += consumed;
        // ---- Road Grade --------------------------------------------------
        if (data[1] == 0x22 && tag == 0x10) {
            int64_t zwiftGrade = static_cast<int64_t>(uval);
            // ZigZag decode
            zwiftGrade = (zwiftGrade >> 1) ^ -(zwiftGrade & 1);
            float grade = (float)zwiftGrade / 100.0f;
            LOG(" -> Zwift Road Grade: %6.2f", grade);
            operations->setNewGrade(grade); // grade in percentage resolution 0.01
        }
        // ---- Gear Ratio --------------------------------------------------
        else if (data[1] == 0x2A && tag == 0x10) {
#ifdef DEBUG_ONWRITE
            uint64_t zwiftGearRatio = uval;
            double gearRatio = (double)zwiftGearRatio / 10000.0f;
            int currentGear = utils->getGearNumberFromRatio(gearRatio);
            LOG(" -> Zwift Gear Ratio: %4.2f -> Gear: %d", gearRatio, currentGear);
#endif
        }
        // ---- Unknown / ignored tags -------------------------------------
        else {
            // Intentionally ignored
            // Future-proof: safely skip unknown fields
        }
    } // while
};

void ZVS::serverVSASYNCOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) {
    std::string str = std::string(pCharacteristic->getUUID());
    if(subValue == 0) {
      LOG("Central Unsubscribed: [%s] ASYNC Status", str.c_str());
      isServerVSasyncNotifyEnabled = false;
    } else {
      LOG("Central Subscribed: [%s] ASYNC Status", str.c_str());
      isServerVSasyncNotifyEnabled = true;
    }
};

void ZVS::serverVSSYNCTXOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) {
    std::string str = std::string(pCharacteristic->getUUID());
    if(subValue == 0) {
      LOG("Central Unsubscribed: [%s] SYNCTX Data", str.c_str());
      isServerVSsynctxIndicateEnabled = false;
    } else {
      LOG("Central Subscribed: [%s] SYNCTX Data", str.c_str());
      isServerVSsynctxIndicateEnabled = true;
    }
};

// Constructor
ZVS::ZVS() { 
  operations = OPS::getInstance();
  utils = UTILS::getInstance();
}

// Destructor
ZVS::~ZVS() { }

ZVS* ZVS::getInstance() {
    if (ZVS::instance == nullptr) {
        ZVS::instance = new ZVS();
    }
    return ZVS::instance;
}

void ZVS::server_setupZVS(NimBLEServer* pServer) {
    server_VS_Service = pServer->createService(ZWIFT_VIRTUAL_SHIFTING_SERVICE);
    server_VS_ASYNC_Chr = server_VS_Service->createCharacteristic(ZWIFT_ASYNC_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::NOTIFY);
    server_VS_ASYNC_Chr->setCallbacks(new server_VS_ASYNC_Chr_callbacks(this));
    server_VS_SYNCRX_Chr = server_VS_Service->createCharacteristic(ZWIFT_SYNCRX_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::WRITE_NR);
    server_VS_SYNCRX_Chr->setCallbacks(new server_VS_SYNCRX_Chr_callbacks(this));
    server_VS_SYNCTX_Chr = server_VS_Service->createCharacteristic(ZWIFT_SYNCTX_CHARACTERISTIC_UUID, NIMBLE_PROPERTY::INDICATE);
    server_VS_SYNCTX_Chr->setCallbacks(new server_VS_SYNCTX_Chr_callbacks(this));   
    server_VS_Service->start();   
}

bool ZVS::client_VirtualShifting_Connect(NimBLEClient* pClient) {
    // Obtain a reference to the remote ZVS service.
    pRemote_VirtualShifting_Service = pClient->getService(CLIENT_ZWIFT_VIRTUAL_SHIFTING_SERVICE);
    if (pRemote_VirtualShifting_Service == nullptr) {
      LOG("Zwift Virtual Shifting Service: Not Found!");
      return false;
    }
    LOG("Client_VirtualShifting_Service: Found!");
	
    pRemote_VS_ASYNC_Chr = pRemote_VirtualShifting_Service->getCharacteristic(ZWIFT_ASYNC_CHARACTERISTIC_UUID);
    if (pRemote_VS_ASYNC_Chr == nullptr) {
      LOG("Mandatory pRemote_VS_ASYNC_Chr: Not Found!");
      return false;
    }
    LOG("Client_VS_ASYNC_Chr: Found!");  
    if(!pRemote_VS_ASYNC_Chr->canNotify()) {
      LOG("Mandatory pRemote_VS_ASYNC_Chr: Cannot Notify!");
      return false;
    }
	
    pRemote_VS_SYNCRX_Chr = pRemote_VirtualShifting_Service->getCharacteristic(ZWIFT_SYNCRX_CHARACTERISTIC_UUID);
    if (pRemote_VS_SYNCRX_Chr == nullptr) {
      LOG("Mandatory pRemote_VS_SYNCRX_Chr: Not Found!");
      return false;     
    }
    LOG("Client_VS_SYNCRX_Chr: Found!");

    if(!pRemote_VS_SYNCRX_Chr->canWriteNoResponse()) { 
      LOG("Mandatory pRemote_VS_SYNCRX_Chr: Cannot Write-No-Response!");
      return false;
    }

	pRemote_VS_SYNCTX_Chr = pRemote_VirtualShifting_Service->getCharacteristic(ZWIFT_SYNCTX_CHARACTERISTIC_UUID);
    if (pRemote_VS_SYNCTX_Chr == nullptr) {
      LOG("Mandatory pRemote_VS_SYNCTX_Chr: Not Found!");
      return false;
    }
    LOG("Client_VS_SYNCTX_Chr: Found!");  
    if(!pRemote_VS_SYNCTX_Chr->canIndicate()) {
      LOG("Mandatory pRemote_VS_SYNCTX_Chr: Cannot Indicate!");
      return false;
    }

    return true;    
}

bool ZVS::write_pRemote_VS_SYNCRX_Chr(std::string data) {

// was clientIsConnected
  if(!operations->Trainer.IsConnected) return false;
  
  // Copy to this scope as it will be handled asynchronously by another core and NimBLE may only write garbage
  std::string localCopy = data;
  if(pRemote_VS_SYNCRX_Chr->writeValue(localCopy, false)) {
    return true;
  }
  return false;
}

ZVS::TrainerData ZVS::getTrainerDataValues(std::vector<uint8_t>* trainerData) {

    ZVS::TrainerData out;

    if (!trainerData || trainerData->empty()) {
        return out;
    }

    const std::vector<uint8_t>& data = *trainerData;
    size_t offset = 0;

    // Check header byte (expected: 0x03)
    if (data[offset] != 0x03) {
        return out;    // invalid packet
    }
    offset++;

    out.valid = true;

    // Process blocks until we run out of data
    while (offset < data.size()) {

        uint8_t blockId = data[offset++];

        uint64_t uval = 0;
        size_t consumed = bfs::DecodeUleb128(data.data() + offset, data.size() - offset, &uval);

        if (consumed == 0) {
            out.valid = false; // decoding failed
            break;
        }

        offset += consumed;

        int64_t value = static_cast<int64_t>(uval);

        switch (blockId) {
            case 0x08: out.power    = value; break;
            case 0x10: out.cadence  = value; break;
            case 0x18: out.unknown1 = value; break;
            case 0x20: out.unknown2 = value; break;
            case 0x28: out.unknown3 = value; break;
            case 0x30: out.unknown4 = value; break;

            default:
                // Unknown block ID — ignored but not considered an error
                break;
        }
    }

    return out;
}

void ZVS::processTrainerData(std::vector<uint8_t>* trainerData) {

    ZVS::TrainerData td = getTrainerDataValues(trainerData);

    if (!td.valid) {
        // Optional logging
        LOG("Client ASYNC: Invalid Trainer Data packet!");
        return;
    }

    // Clean, readable access:
    LOG("--> Instant Trainer Data: Power: [%04lld] - Cadence: [%03lld]", td.power, td.cadence);
    // Handle remaining fields as needed…
}

void ZVS::client_VS_ASYNC_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                                        uint8_t* pData, size_t length, bool isNotify) {
	// Pass on data to the server-side
	if(isServerVSasyncNotifyEnabled) {
		server_VS_ASYNC_Chr->setValue(pData, length);
		server_VS_ASYNC_Chr->notify();
	}	
	LOG("Client ASYNC Notified: %s", utils->getHexString(pData, length).c_str());
#ifdef DEBUG_ASYNC
	// Decode data -> Encoded Trainerdata (Power and Cadence) start with: 03 08
  std::vector<uint8_t> trainerData(pData, pData + length);
  if (trainerData.empty() || trainerData[0] != 0x03)
    return;
  // Decode and show content
  processTrainerData(&trainerData);
#endif
}
																				  
void ZVS::Static_VS_ASYNC_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                                          uint8_t* pData, size_t length, bool isNotify) {
    if (instance) {
        instance->client_VS_ASYNC_Notify_Callback(pBLERemoteCharacteristic, pData, length, isNotify);
    }
}

void ZVS::client_VS_SYNCTX_Indicate_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, \
																						size_t length, bool isNotify) {
	// Pass on data to the server-side
	if(isServerVSsynctxIndicateEnabled) {
		server_VS_SYNCTX_Chr->setValue(pData, length);
		server_VS_SYNCTX_Chr->indicate();
	}	
	// Just Show Data
	LOG("Client SYNCTX Indicated: %s", utils->getHexString(pData, length).c_str());
}

void ZVS::Static_VS_SYNCTX_Indicate_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                                          uint8_t* pData, size_t length, bool isNotify) {
    if (instance) {
        instance->client_VS_SYNCTX_Indicate_Callback(pBLERemoteCharacteristic, pData, length, isNotify);
    }
}

void ZVS::client_ZVS_Subscribe(void) {
  if( pRemote_VS_ASYNC_Chr != nullptr ) 
    if( !pRemote_VS_ASYNC_Chr->subscribe(notifications, Static_VS_ASYNC_Notify_Callback) ) 
      LOG(">>> ERROR pRemote_VS_ASYNC_Chr Subscribe failed!"); 
  if( pRemote_VS_SYNCTX_Chr != nullptr ) 
    if( !pRemote_VS_SYNCTX_Chr->subscribe(indications, Static_VS_SYNCTX_Indicate_Callback) ) 
      LOG(">>> ERROR pRemote_VS_SYNCTX_Chr Subscribe failed!");    
}

void ZVS::client_ZVS_Unsubscribe(void) {
  if ( pRemote_VS_ASYNC_Chr != nullptr )
    if( !pRemote_VS_ASYNC_Chr->unsubscribe(false) )
      LOG(">>> ERROR pRemote_VS_ASYNC_Chr Unsubscribe failed!"); 
  if ( pRemote_VS_SYNCTX_Chr != nullptr )
    if( !pRemote_VS_SYNCTX_Chr->unsubscribe(false) )
      LOG(">>> ERROR pRemote_VS_SYNCTX_Chr Unsubscribe failed!"); 
}

