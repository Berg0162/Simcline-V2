#include "CyclingSpeedCadence.h"

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
//#define DEBUG_CSC_MEASUREMENT // If defined allows for parsing and decoding the Cycling Power Measurement Data
#endif

// Include operations control settings class
#include "Operations.h"

/*---------------------------------------------------------------------------------------
 * Cycling Speed and Cadence Service
 * CSC Service:   0x1816 
 * CSC Measurement Characteristic:  0x2A5B
 * CSC Feature Characteristic:      0x2A5C  
 */
#define UUID16_SVC_CYCLING_SPEED_AND_CADENCE                  NimBLEUUID((uint16_t)0x1816)
//#define UUID16_CHR_CSC_CONTROL_POINT                          NimBLEUUID((uint16_t)0x2A55)
#define UUID16_CHR_CSC_MEASUREMENT                            NimBLEUUID((uint16_t)0x2A5B)
#define UUID16_CHR_CSC_FEATURE                                NimBLEUUID((uint16_t)0x2A5C)
#define UUID16_CHR_SENSOR_LOCATION                            NimBLEUUID((uint16_t)0x2A5D) // shared with CP

#ifdef DEBUG
const uint8_t CSC_FEATURE_FIXED_DATALEN = 2; // UINT16
const uint8_t client_CSC_Feature_Len = 3;
const char* client_CSC_Feature_Str[client_CSC_Feature_Len] = {"Wheel rev supported", "Crank rev supported", "Multiple locations supported"};
const uint8_t client_CSC_Sensor_Location_Str_Len = 17;       
const char* client_CSC_Sensor_Location_Str[client_CSC_Sensor_Location_Str_Len] = { "Other", "Top of shoe", "In shoe", "Hip", 
    "Front wheel", "Left crank", "Right crank", "Left pedal", "Right pedal", "Front hub", 
    "Rear dropout", "Chainstay", "Rear wheel", "Rear hub", "Chest", "Spider", "Chain ring"};
#endif

class server_CSC_Measurement_Chr_callbacks final : public NimBLECharacteristicCallbacks {
private:
    CSC* cscInstance;  // LOCAL Pointer to the CSC class instance
public:
    // Constructor to accept a pointer to the CSC class instance
    server_CSC_Measurement_Chr_callbacks(CSC* instance) : cscInstance(instance) {}
protected:
    inline void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
        cscInstance->serverCSCMeasurementOnSubscribe(pCharacteristic, connInfo, subValue);
    } // onSubscribe
}; 

// Initialize the static member
CSC* CSC::instance = nullptr;

/*  We only define onSubscribe !!!
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo);
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo); 
    void onNotify(NimBLECharacteristic* pCharacteristic);    
*/ 
void CSC::serverCSCMeasurementOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) {
    std::string str = std::string(pCharacteristic->getUUID());
    if(subValue == 0) {
      LOG("Central Unsubscribed: [%s] CSC Measurement", str.c_str());
      isServerCSCMeasurementNotifyEnabled = false;
    } else {
      LOG("Central Subscribed: [%s] CSC Measurement", str.c_str());
      isServerCSCMeasurementNotifyEnabled = true;
    }
}

// Constructor
CSC::CSC() { 
  operations = OPS::getInstance();
}

// Destructor
CSC::~CSC() { }

CSC* CSC::getInstance() {
    if (instance == nullptr) {
        instance = new CSC();
    }
    return instance;
}

void CSC::server_setupCSC(NimBLEServer* pServer) {
   server_CyclingSpeedCadence_Service = pServer->createService(UUID16_SVC_CYCLING_SPEED_AND_CADENCE);
    server_CSC_Measurement_Chr = server_CyclingSpeedCadence_Service->createCharacteristic(UUID16_CHR_CSC_MEASUREMENT, 
                                                                            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    server_CSC_Measurement_Chr->setCallbacks(new server_CSC_Measurement_Chr_callbacks(this)); //NIMBLE 
    server_CSC_Feature_Chr = server_CyclingSpeedCadence_Service->createCharacteristic(UUID16_CHR_CSC_FEATURE, 
                                                                            NIMBLE_PROPERTY::READ);
    // Set server CSC Feature Flags field                                                                            
    server_CSC_Feature_Chr->setValue(client_CSC_Feature_Flags);  // Set default                                                                      
    server_CSC_Location_Chr = server_CyclingSpeedCadence_Service->createCharacteristic(UUID16_CHR_SENSOR_LOCATION, 
                                                                            NIMBLE_PROPERTY::READ);
    // Set server_CSC_Location for sensor
    server_CSC_Location_Chr->setValue(&client_CSC_Location_Value, 1); // Set Default
    server_CyclingSpeedCadence_Service->start();    
}

void CSC::server_CSC_Measurement_Chr_Notify(uint8_t* pData, size_t length) {
  if(isServerCSCMeasurementNotifyEnabled) {
    server_CSC_Measurement_Chr->setValue(pData, length);
    server_CSC_Measurement_Chr->notify();
  }
}

void CSC::client_CSC_Measurement_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  // Client CSC Measurement data is tranferred to the Server
  if(operations->Laptop.IsConnected) 
    server_CSC_Measurement_Chr_Notify(pData, length);

#ifdef DEBUG_CSC_MEASUREMENT
  uint8_t buffer[length]= {}; 
  // Transfer first the contents of data to buffer (array of chars)
  memcpy(buffer, pData, length);
  std::string hexString = UTILS::getInstance()->toHexString(buffer, static_cast<uint8_t>(length));
  LOG(" -> Client Rec'd Raw CSC Data: [%d] [%s]", length, hexString.c_str());
  uint8_t offset = 0; 
  // we define the offset that is to be used when reading the next field
  // Size of variables (e.g. 2 (16) or 4 bytes (32)) are constants in BluetoothGattCharacteristic
  // these represent the values you can find in the "Value Fields" table in the "Format" column
  // Read the Flags field at buffer[0]
  uint8_t flags = buffer[offset]; 
  offset += 1; // UINT8 
  // we have to check the flags' nth bit to see if C1 field exists 
  if ((flags & 1) != 0) {
    uint32_t cum_wheel_rev = 0;
    memcpy(&cum_wheel_rev, &buffer[offset], 4);
    offset += 4; // UINT32
    uint16_t last_wheel_event = 0;
    memcpy(&last_wheel_event, &buffer[offset], 2);
    offset += 2; // UINT16
    LOG(" Cum. wheel rev.: %d Last wheel event: %d ", cum_wheel_rev, last_wheel_event);
/* Calculation of speed at the Collector can be derived from the wheel circumference and
* data in two successive measurements. The Collector calculation can be performed as
* shown below:
* Speed = (Difference in two successive Cumulative Wheel Revolution values * Wheel Circumference) 
*         / (Difference in two successive Last Wheel Event Time values)
*/
  }
  // we have to check the flags' nth bit to see if C2 field exists 
  if ((flags & 2) != 0) {
    uint16_t cum_cranks = 0;
    memcpy(&cum_cranks, &buffer[offset], 2);
    offset += 2; // UINT16
    uint16_t last_crank_event = 0;
    memcpy(&last_crank_event, &buffer[offset], 2);
    offset += 2; // UINT16
    LOG(" Cum cranks: %d Last crank event: %d", cum_cranks, last_crank_event);
/* Calculation of cadence at the Collector can be derived from data in two successive
* measurements. The Collector calculation can be performed as shown below:
* Cadence = (Difference in two successive Cumulative Crank Revolution values)
*           / (Difference in two successive Last Crank Event Time values)        
*/         
  }
  // etcetera...
#endif
}

void CSC::Static_CSC_Measurement_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                                    uint8_t* pData, size_t length, bool isNotify) {
    if (instance) {
        instance->client_CSC_Measurement_Notify_Callback(pBLERemoteCharacteristic, pData, length, isNotify);
    }
}

bool CSC::client_CyclingSpeedCadence_Connect(NimBLEClient* pClient) {
    // Obtain a reference to the remote CSC service.
    pRemote_CyclingSpeedCadence_Service = pClient->getService(UUID16_SVC_CYCLING_SPEED_AND_CADENCE);
    if (pRemote_CyclingSpeedCadence_Service == nullptr) {
      LOG("Cycling Speed Cadence Service: Not Found!");
      return false; // ENABLE_CSC
    }
    LOG("Client_CyclingSpeedCadence_Service: Found!");
    pRemote_CSC_Measurement_Chr = pRemote_CyclingSpeedCadence_Service->getCharacteristic(UUID16_CHR_CSC_MEASUREMENT);
    if (pRemote_CSC_Measurement_Chr == nullptr) {
      LOG("Mandatory client_CSC_Measurement_Chr: Not Found!");
      return false;
    }
    LOG("Client_CSC_Measurement_Chr: Found!");  
    if(!pRemote_CSC_Measurement_Chr->canNotify()) {
      LOG("Mandatory Client_CSC_Measurement_Chr: Cannot Notify!");
      return false;
    }
    pRemote_CSC_Feature_Chr = pRemote_CyclingSpeedCadence_Service->getCharacteristic(UUID16_CHR_CSC_FEATURE);
    if (pRemote_CSC_Feature_Chr == nullptr) {
      LOG("Mandatory Client_CSC_Feature_Chr: Not Found!");
      return false;     
    }
    LOG("Client_CSC_Feature_Chr: Found!");
    // Read the value of the characteristic.
    if(pRemote_CSC_Feature_Chr->canRead()) 
      {
        // Read 16-bit client_CSC_Feature_Chr value
        client_CSC_Feature_Flags = pRemote_CSC_Feature_Chr->readValue<uint16_t>(); 
        if(server_CSC_Feature_Chr) {
          server_CSC_Feature_Chr->setValue(client_CSC_Feature_Flags); // Transfer/Update to the server side
        }
#ifdef DEBUG
        //  Little Endian Representation
        uint8_t cscfcData[CSC_FEATURE_FIXED_DATALEN] = { static_cast<uint8_t>(client_CSC_Feature_Flags & 0xff), \
                                                         static_cast<uint8_t>(client_CSC_Feature_Flags >> 8) }; 
        std::string hexString = UTILS::getInstance()->toHexString(cscfcData, CSC_FEATURE_FIXED_DATALEN);                  
        LOG(" -> Client Reads Raw CSC Feature bytes: [2] [%s]", hexString.c_str());
        for (int i = 0; i < client_CSC_Feature_Len; i++) {
          if ( (client_CSC_Feature_Flags & (1 << i)) != 0 ) {
            LOG("  %s", client_CSC_Feature_Str[i]);
            }
        }
#endif
      } // canRead Feature
     
    pRemote_CSC_Location_Chr = pRemote_CyclingSpeedCadence_Service->getCharacteristic(UUID16_CHR_SENSOR_LOCATION);
    if (pRemote_CSC_Location_Chr == nullptr) {
      LOG("Client_CSC_Location_Chr: Not Found!");
    } else {
      LOG("Client_CSC_Location_Chr: Found!");
      // Read the value of the characteristic.
      if(pRemote_CSC_Location_Chr->canRead()) {
        client_CSC_Location_Value = pRemote_CSC_Location_Chr->readValue<uint8_t>(); 
        //MITM
        if(server_CSC_Location_Chr) { 
          server_CSC_Location_Chr->setValue(&client_CSC_Location_Value, 1); // Transfer/Update the server side
        }
        // CSC sensor location value is 8 bit
#ifdef DEBUG
        if(client_CSC_Location_Value <= client_CSC_Sensor_Location_Str_Len) 
          LOG(" -> Client Reads CSC Location Sensor: Loc#: %d %s", \
                                client_CSC_Location_Value, client_CSC_Sensor_Location_Str[client_CSC_Location_Value]);
        else 
          LOG(" -> Client Reads CSC Location Sensor: Loc#: %d", client_CSC_Location_Value);
#endif
      }
    }
    return true;    
}

void CSC::client_CSC_Subscribe(void) {
  if( pRemote_CSC_Measurement_Chr != nullptr ) 
    if( !pRemote_CSC_Measurement_Chr->subscribe(notifications, Static_CSC_Measurement_Notify_Callback) ) 
      LOG(">>> ERROR remote CSC Measure Subscribe failed!");                        
}

void CSC::client_CSC_Unsubscribe(void) {
  if ( pRemote_CSC_Measurement_Chr != nullptr )
    if( !pRemote_CSC_Measurement_Chr->unsubscribe(false) )
      LOG(">>> ERROR remote CSC Measure Subscribe failed!"); 
}


