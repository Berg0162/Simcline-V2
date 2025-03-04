#include "HeartRateMonitor.h"

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress DEBUG messages that help debugging...
// Uncomment general "#define DEBUG" to activate debug messages for this class
#define DEBUG
// Include these debug utility macros in all cases!
#include "config/configDebug.h"
// ------------------------------------------------------------------------------------------------

#ifdef DEBUG
//  Restrict activating one or more of the following << EXTRA >> debug directives --> process intensive 
//  The overhead can lead to spurious side effects and a loss of quality of service handling!!
//#define DEBUG_HRM               // If defined allows for parsing and decoding the Heart Rate Data
#endif

// Include operations control settings class
#include "Operations.h"

/* HRM Service Definitions -------------------------------------------------------------
 * Heart Rate Monitor Service:  0x180D
 * Heart Rate Measurement Char: 0x2A37 (Mandatory)
 * Body Sensor Location Char:   0x2A38 (Optional)
 */
#define UUID16_SVC_HEART_RATE             NimBLEUUID((uint16_t)0x180D)
#define UUID16_CHR_HEART_RATE_MEASUREMENT NimBLEUUID((uint16_t)0x2A37)
#define UUID16_CHR_BODY_SENSOR_LOCATION   NimBLEUUID((uint16_t)0x2A38)
/*  HR Body Sensor Location
    0x00 Other
    0x01 Chest
    0x02 Wrist
    0x03 Finger
    0x04 Hand
    0x05 Ear Lobe
    0x06 Foot
    0x07–0xFF Reserved for Future Use
*/
/*
    Field #1 - Flags (byte)
        Bit 0   - Heart Rate Value Format
                    0 = uint8
                    1 = uint16
        Bit 1-2 - Sensor Contact Status
                    0 - Sensor Contact feature is not supported in the current connection
                    1 - Sensor Contact feature is not supported in the current connection
                    2 - Sensor Contact feature is supported, but contact is not detected
                    3 - Sensor Contact feature is supported and contact is detected
        Bit 3   - Energy Expended Status
                    0 = Energy Expended field is not present
                    1 = Energy Expended field is present. Units: kilo Joules
        Bit 3   - RR-Interval bit
                    0 = RR-Interval values are not present.
                    1 = One or more RR-Interval values are present.
        Bit 5-7 - Reserved
    Field #2 - Heart Rate Measurement Value (uint8)
    Field #3 - Heart Rate Measurement Value (uint16)
    Field #4 - Energy Expended (uint16)
    Field #5 - RR-Interval (uint16)
    // Flags = Format uint16 and contact supported and detected
    //byte HR_MeasurementFlags = 0b00000101;
*/

class server_HRM_Measurement_Chr_callbacks final : public NimBLECharacteristicCallbacks {
private:
    HRM* hrmInstance;  // LOCAL Pointer to the HRM class instance
public:
    // Constructor to accept a pointer to the HRM class instance
    server_HRM_Measurement_Chr_callbacks(HRM* instance) : hrmInstance(instance) {}
protected:
  inline void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
    hrmInstance->serverHRMMeasurementOnSubscribe(pCharacteristic, connInfo, subValue);
  }
};

// Initialize the static member
HRM* HRM::instance = nullptr;

// ----------------------------------------------------------------------------------
void HRM::client_HR_Measurement_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                                    uint8_t* pData, size_t length, bool isNotify) {
  // Client HR Measurement data is tranferred to the Server (Zwift)
  if(operations->Laptop.IsConnected)
    server_HR_Measurement_Chr_Notify(pData, length);
  // Measurement contains of Flags byte, measurement (8 or 16 bit) and optional fields
#ifdef DEBUG_HRM
  uint8_t HRDataLen = (uint8_t)length;
  uint8_t HRDataBuf[HRDataLen] = {};
  // Transfer first the contents of data to buffer (array of chars)
  memcpy(HRDataBuf, pData, HRDataLen);
  std::string hexString = UTILS::getInstance()->toHexString(HRDataBuf, HRDataLen);
  LOG(" -> Client Rec'd Raw Heart Rate Measurement Data: [%d] [%s]", HRDataLen, hexString.c_str());
  uint8_t offset = 0;
  uint8_t flags = 0;
  memcpy(&flags, &HRDataBuf[offset], 1); // Transfer buffer fields to flags variable
  offset += 1;  // UINT8
  if(flags & 1) { // 16 bit data value is present flag
    uint16_t HRMvalue = 0;
    memcpy(&HRMvalue, &HRDataBuf[offset], 2); // Transfer buffer fields to variable
    LOG("Heart Beats: %d HBM (16)", HRMvalue);
    offset += 2;  // UINT16 
  } else { // 8 bit value
    uint8_t HRMvalue = 0;
    memcpy(&HRMvalue, &HRDataBuf[offset], 1); // Transfer buffer fields to variable
    LOG("Heart Beats: %d HBM (8)", HRMvalue);
    offset += 1;  // UINT8     
  }
  
  if(flags & 2) { // sensor
    std::string str = flags & 4 ? "ON" : "OFF";
    LOG("Contact is detected: %s", str.c_str());
  /*
    if (flags & 4) LOG("Contact is detected: ON"); 
    else LOG("Contact is detected: OFF");
  */
  } else LOG(" Contact is NOT detected!");
  if(flags & 8) LOG(" Expended Energy");
  if(flags & 16) LOG(" RR interval");
#endif
}

void HRM::Static_client_HR_Measurement_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    if (HRM::instance) {
        HRM::instance->client_HR_Measurement_Notify_Callback(pBLERemoteCharacteristic, pData, length, isNotify);
    }
}

/*  We only define onSubscribe !!!
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo);
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo); 
    void onNotify(NimBLECharacteristic* pCharacteristic);    
*/ 
void HRM::serverHRMMeasurementOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) {
    std::string str = std::string(pCharacteristic->getUUID());
    if(subValue == 0) {
      LOG("Central Unsubscribed: [%s] HRM Measurement", str.c_str());
      isServerHRMMeasurementNotifyEnabled = false;
    } else {
      LOG("Central Subscribed: [%s] HRM Measurement", str.c_str());
      isServerHRMMeasurementNotifyEnabled = true;
    }
}

// Constructor
HRM::HRM() { 
    operations = OPS::getInstance();
}

// Destructor
HRM::~HRM() { }

HRM* HRM::getInstance() {
    if (HRM::instance == nullptr) {
        HRM::instance = new HRM();
    }
    return HRM::instance;
}

bool HRM::client_HeartRate_Connect(NimBLEClient* pClient) {
    // Obtain a reference to the remote HRM service.
    pRemote_HeartRate_Service = pClient->getService(UUID16_SVC_HEART_RATE);
    if (pRemote_HeartRate_Service == nullptr) {
      LOG("Heart Rate Service: Not Found!");
      return false; // ENABLE_HBM
    }
    LOG("Client_HeartRate_Service: Found!");
    pRemote_HR_Measurement_Chr = pRemote_HeartRate_Service->getCharacteristic(UUID16_CHR_HEART_RATE_MEASUREMENT);
    if (pRemote_HR_Measurement_Chr == nullptr) {
      LOG("Mandatory client_HR_Measurement_Chr: Not Found!");
      return false;
    }
    LOG("Client_HR_Measurement_Chr: Found!");  
    if(!pRemote_HR_Measurement_Chr->canNotify()) {
      LOG("Mandatory Client_HR_Measurement_Chr: Cannot Notify!");
      return false;
    }
    pRemote_HR_Location_Chr = pRemote_HeartRate_Service->getCharacteristic(UUID16_CHR_BODY_SENSOR_LOCATION);
    if (pRemote_HR_Location_Chr == nullptr) {
      LOG("Client_HR_Location_Chr: Not Found!");
    } else {
      LOG("Client_HR_Location_Chr: Found!");
      // Read the value of the characteristic.
      if(pRemote_HR_Location_Chr->canRead()) {
        client_HR_Location_Value = pRemote_HR_Location_Chr->readValue<uint8_t>(); 
        if(server_HR_Location_Chr) { 
          server_HR_Location_Chr->setValue(&client_HR_Location_Value, 1); // Transfer/Update the value to the server side
        }
        // Body sensor location value is 8 bit
#ifdef DEBUG
        const uint8_t body_str_len = 7;
        const char* body_str[body_str_len] = { "Other", "Chest", "Wrist", "Finger", "Hand", "Ear Lobe", "Foot" };
        if(client_HR_Location_Value <= body_str_len)
          LOG(" -> Client Reads HR Location Sensor: Loc#: %d %s", client_HR_Location_Value, \
                                                                                      body_str[client_HR_Location_Value]);
        else 
          LOG(" -> Client Reads HR Location Sensor: Loc#: %d", client_HR_Location_Value);
#endif
      }
    }
    return true;    
}

void HRM::server_setupHRM(NimBLEServer* pServer) {
    server_HeartRate_Service = pServer->createService(UUID16_SVC_HEART_RATE);
    server_HR_Measurement_Chr = server_HeartRate_Service->createCharacteristic(UUID16_CHR_HEART_RATE_MEASUREMENT, 
                                                                            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    server_HR_Measurement_Chr->setCallbacks(new server_HRM_Measurement_Chr_callbacks(this)); //NIMBLE
    server_HR_Location_Chr = server_HeartRate_Service->createCharacteristic(UUID16_CHR_BODY_SENSOR_LOCATION, 
                                                                            NIMBLE_PROPERTY::READ);
    // Set server_HR_Location for sensor
    server_HR_Location_Chr->setValue(&client_HR_Location_Value, 1); // Set default
    server_HeartRate_Service->start();  
}

void HRM::client_HR_Subscribe(void) {
  if ( pRemote_HR_Measurement_Chr != nullptr )
    if( !pRemote_HR_Measurement_Chr->subscribe(notifications, Static_client_HR_Measurement_Notify_Callback) ) 
      LOG(">>> ERROR remote HRM Measure Subscribe failed!");                
}

void HRM::client_HR_Unsubscribe(void) {
  if ( pRemote_HR_Measurement_Chr != nullptr )
    if( !pRemote_HR_Measurement_Chr->unsubscribe(false) )
      LOG(">>> ERROR remote HRM Measure Unsubscribe failed!");
}

void HRM::server_HR_Measurement_Chr_Notify(uint8_t* pData, size_t length) {
  if(isServerHRMMeasurementNotifyEnabled) {
    server_HR_Measurement_Chr->setValue(pData, length);
    server_HR_Measurement_Chr->notify();
  }
}
