#include "CyclingPower.h"

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress DEBUG messages that help debugging...
// Uncomment general "#define DEBUG" to activate debug messages for this class
#define DEBUG
// Include these debug utility macros in all cases!
#include "config/configDebug.h"

#ifdef DEBUG
//  Restrict activating one or more of the following DEBUG directives --> process intensive 
//  The overhead can lead to spurious side effects and a loss of quality of service handling!!
//#define DEBUG_CP_MEASUREMENT            // If defined allows for parsing and decoding the Cycling Power Measurement Data
#define DEBUG_CPS_CONTROLPOINT          // If defined allows for parsing and decoding the Cycling Power Control Point Data
//#define DEBUG_CPS_CONTROLPOINT_DECODE  // If defined allows for decoding Raw Data
#define DEBUG_CPS_CONTROLPOINT_RESPONSE // If defined allows for parsing the Data
#endif

// ------------------------------------------------------------------------------------------------
// Include operations control settings class
#include "Operations.h"

#include "ClientSide.h"

/* Cycling Power Service ---------------------------------------------------------------
 * CP Service: 0x1818  
 * CP Characteristic: 0x2A63 (Measurement)    Mandatory
 * CP Characteristic: 0x2A65 (Feature)        Mandatory
 * CP Characteristic: 0x2A5D (Location)       Optional
 * CP Characteristic: 0x2A66 (Control Point)  Optional
 */
#define UUID16_SVC_CYCLING_POWER                              NimBLEUUID((uint16_t)0x1818)
#define UUID16_CHR_CYCLING_POWER_MEASUREMENT                  NimBLEUUID((uint16_t)0x2A63)
//#define UUID16_CHR_CYCLING_POWER_VECTOR                       NimBLEUUID((uint16_t)0x2A64)
#define UUID16_CHR_CYCLING_POWER_FEATURE                      NimBLEUUID((uint16_t)0x2A65)
#ifdef ENABLE_WAHOOCPS
#define UUID16_CHR_CYCLING_POWER_CONTROL_POINT                NimBLEUUID("A026E005-0A7D-4AB3-97FA-F1500F9FEB8B")
#else
#define UUID16_CHR_CYCLING_POWER_CONTROL_POINT                NimBLEUUID((uint16_t)0x2A66)
#endif
#define UUID16_CHR_SENSOR_LOCATION                            NimBLEUUID((uint16_t)0x2A5D) // shared with CSC

#ifdef DEBUG
const uint8_t client_CP_Feature_Len = 20; // Num. of Feature elements
const char* client_CP_Feature_Str[client_CP_Feature_Len] = { 
      "Pedal power balance supported",
      "Accumulated torque supported",
      "Wheel revolution data supported",
      "Crank revolution data supported",
      "Extreme magnitudes supported",
      "Extreme angles supported",
      "Top/bottom dead angle supported",
      "Accumulated energy supported",
      "Offset compensation indicator supported",
      "Offset compensation supported",
      "Cycling power measurement characteristic content masking supported",
      "Multiple sensor locations supported",
      "Crank length adj. supported",
      "Chain length adj. supported",
      "Chain weight adj. supported",
      "Span length adj. supported",
      "Sensor measurement context",
      "Instantaineous measurement direction supported",
      "Factory calibrated date supported",
      "Enhanced offset compensation supported" };

const uint8_t client_CP_Sensor_Location_Str_Len = 17;       
const char* client_CP_Sensor_Location_Str[client_CP_Sensor_Location_Str_Len] = { "Other", "Top of shoe", "In shoe", "Hip", 
    "Front wheel", "Left crank", "Right crank", "Left pedal", "Right pedal", "Front hub", 
    "Rear dropout", "Chainstay", "Rear wheel", "Rear hub", "Chest", "Spider", "Chain ring"};
#endif

class server_CP_ControlPoint_Chr_callback final : public NimBLECharacteristicCallbacks {
private:
    CPS* cpsInstance;  // LOCAL Pointer to the CPS class instance
public:
    // Constructor to accept a pointer to the CPS class instance
    server_CP_ControlPoint_Chr_callback(CPS* instance) : cpsInstance(instance) {}
protected:
    inline void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        cpsInstance->serverCPControlPointOnWrite(pCharacteristic, connInfo);
    }
    inline void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
        cpsInstance->serverCPControlPointOnSubscribe(pCharacteristic, connInfo, subValue);
    }
    void onStatus(NimBLECharacteristic* pCharacteristic, int code) {
#ifdef DEBUG_CPS_CONTROLPOINT_RESPONSE
      if(code == BLE_HS_EDONE) {
        LOG("[%5d] Indication confirmed!", (millis()-cpsInstance->icInterval) );
      } else if(code == 0) {
        LOG("Notification Success!");
      } else {
        LOG("Status: [%d]", code);
      }
#endif 
    }
};

/*  We only define onSubscribe !!!
    void onRead(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo);
    void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo);  
    void onNotify(NimBLECharacteristic* pCharacteristic);    
*/
class server_CP_Measurement_Chr_callback final : public NimBLECharacteristicCallbacks {
private:
    CPS* cpsInstance;  // LOCAL Pointer to the CPS class instance
public:
    // Constructor to accept a pointer to the CPS class instance
    server_CP_Measurement_Chr_callback(CPS* instance) : cpsInstance(instance) {}
protected:
    inline void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
        cpsInstance->serverCPMeasurementOnSubscribe(pCharacteristic, connInfo, subValue);
    }
};

// Initialize the static member
CPS* CPS::instance = nullptr;

// -------------------------------------------------------------------------------------
/* Return Code = 0x06 --> BLE_HS_ENOMEM --> Operation failed due to resource exhaustion.
* This issue occurs when we try to send too many packets before controller can process them. In this case, the packets get queued. 
* As we queue more number of packets, we reach a point where we run out of memory. In this case, BLE_HS_ENOMEM is returned.
*/
void CPS::xTaskClientWriteWithResponse(void* parameter) {
  CPS* cpsInstance = (CPS*)parameter;
  while(true) { 
      vTaskSuspend(cpsInstance->xTaskClientWriteWithResponseHandle);  // Suspend now until resumed by callback
      if(cpsInstance->xTaskClientWriteWithResponseDataReady) {
          // Copy to this scope as it will be handled asynchronously by another core and NimBLE may only write garbage
          std::string localCopy = cpsInstance->xTaskClientWriteWithResponseData; 
          if(!cpsInstance->pRemote_CP_ControlPoint_Chr->writeValue(localCopy, true))
              LOG(">>> CPS CP Error: Failed to write characteristic -> Return Code = 0x06");
          cpsInstance->xTaskClientWriteWithResponseDataReady = false; // Ready for next data transfer
      } 
  } // while
};

void CPS::serverCPControlPointOnWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo& connInfo) {
#ifdef DEBUG_CPS_CONTROLPOINT 
  static long cpInterval;
  static long cpPrevTime = millis();
  cpInterval = millis() - cpPrevTime; // Time between consecutive server_CP_ControlPoint calls
  cpPrevTime = millis();
  cpWriteTime = cpPrevTime; // Set start time to measure response time later
#endif
  xTaskClientWriteWithResponseData = server_CP_ControlPoint_Chr->getValue();
  size_t cpDatalen = xTaskClientWriteWithResponseData.length();
  // Check for Empty xTaskClientWriteWithResponseData field
  if(!cpDatalen) return; // We are Done!
  static uint8_t eventCnt = 0;
  // Server Control Point data is tranferred to the Client
  if(operations->Trainer.IsConnected && ClientSide::getInstance()->hasSubscribedToAll ) {
    if(xTaskClientWriteWithResponseDataReady == false) {
        xTaskClientWriteWithResponseDataReady = true;      // Server Control Point data are ready!
        vTaskResume(xTaskClientWriteWithResponseHandle);   // Resume xTask and write the data a.s.a.p.
        eventCnt = 0;
    }  
  } else {  // Zwift keeps sending CP-Data -> Trainer is unresponsive!
    if(eventCnt++ < 2) LOG(">>> CPS Trainer is unresponsive! Skipped CP Data!"); // Limit to 3
    return; // Done!
  }

#ifndef ENABLE_WAHOOCPS
  // Default CP Controlpoint handling
  #ifdef DEBUG_CPS_CONTROLPOINT
  // Convert first to HEX presentation
  std::string hexString = UTILS::getInstance()->toHexString(xTaskClientWriteWithResponseData);
  LOG("[%5d] --> Raw CPS Control Point Data [%d] [%s]", cpInterval, cpDatalen, hexString.c_str());
  #endif
#else 
  // Handling when ENABLE_WAHOOCPS is defined
  // Transfer the contents of data to server_Wahoo_Control_Point_Data.bytes
  memcpy(server_Wahoo_Control_Point_Data.bytes, xTaskClientWriteWithResponseData.data(), cpDatalen);

  #ifdef DEBUG_CPS_CONTROLPOINT
  // Display the raw request packet
  std::string hexString = UTILS::getInstance()->toHexString(server_Wahoo_Control_Point_Data.values.OCTETS, cpDatalen-1);
  LOG("[%5d] -> Raw Wahoo Control Point Data [len: %d] [OpCode: %02X] [Values: %s]", cpInterval, cpDatalen, \
                                              server_Wahoo_Control_Point_Data.values.OPCODE, hexString.c_str());
  #endif
  // The documentation I found states that all write actions to this Wahoo CP characteristic are "Write with Response"
  // So we have formally to acknowledge the receipt of the trainer setting
  // Zwift does NOT care at all if one sets a response, it is still working !!!
  // Decodes an incoming Wahoo Control Point request
  switch(server_Wahoo_Control_Point_Data.values.OPCODE) {
  #ifdef DEBUG_CPS_CONTROLPOINT_DECODE    
    case unlock: {
      LOG("    Request to Unlock Machine!");
      break;
    }
    case setResistanceMode: {
      LOG("    Set Resistance Mode!");
      break;
    }
    case setStandardMode: {
      LOG("    Set Standard Mode!");
      break;
    }
    case setSimMode : {
      uint16_t tmp = (server_Wahoo_Control_Point_Data.values.OCTETS[0]) + (server_Wahoo_Control_Point_Data.values.OCTETS[1] << 8);       
      float weight = (float(tmp) / 100); // Rider weight in Kg
      tmp = ( (server_Wahoo_Control_Point_Data.values.OCTETS[2]) + (server_Wahoo_Control_Point_Data.values.OCTETS[3] << 8) );
      float rrc = (float(tmp) / 1000);    // Rolling Resistance Coefficient
      tmp = ( (server_Wahoo_Control_Point_Data.values.OCTETS[4]) + (server_Wahoo_Control_Point_Data.values.OCTETS[5] << 8) );      
      float wrc = (float(tmp) / 1000);    // Wind Resistance Coefficient
      LOG("    Set Simulation Mode! --> Weight: %0.2f RRC: %f WRC: %f", weight, rrc, wrc);
      break;
    }
  #endif
    case setSimGrade: {
      uint16_t gr = ( server_Wahoo_Control_Point_Data.values.OCTETS[0] + (server_Wahoo_Control_Point_Data.values.OCTETS[1] << 8) );
      float grade = 100 * float( ((gr * 2.0 / 65535) - 1.0) ); // Percentage of road grade --> range: between +1 and -1 (!)
      operations->setNewGrade(grade); // grade percentage resolution 0.01
  #ifdef DEBUG_CPS_CONTROLPOINT_DECODE
      LOG("    Set Simulation Grade! --> Grade: %4.1f%%", grade); 
  #endif      
      break;       
    }
  #ifdef DEBUG_CPS_CONTROLPOINT_DECODE
    case setErgMode:
    case setSimCRR:
    case setSimWindResistance:
    case setSimWindSpeed:
    case setWheelCircumference:
    {
      LOG("    Unresolved OpCode!");
      break;
    }
  #endif    
    } // switch
#endif // ENABLE_WAHOOCPS
}; // serverCPControlPointOnWrite

void CPS::serverCPControlPointOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) {
        std::string str = std::string(pCharacteristic->getUUID());
        if(subValue == 0) {
          LOG("Central Unsubscribed: [%s] CPS Control Point", str.c_str());
          isServerCPControlPointIndicateEnabled = false;
        } else {
          LOG("Central Subscribed: [%s] CPS Control Point", str.c_str());
          isServerCPControlPointIndicateEnabled = true;
        }
}

void CPS::serverCPMeasurementOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) {
        std::string str = std::string(pCharacteristic->getUUID());
        if(subValue == 0) {
          LOG("Central Unsubscribed: [%s] CPS Measurement", str.c_str());
          isServerCPMeasurementNotifyEnabled = false;
        } else { 
          LOG("Central Subscribed: [%s] CPS Measurement", str.c_str());
          isServerCPMeasurementNotifyEnabled = true;
        }
}

void CPS::server_CP_Measurement_Chr_Notify(uint8_t* pData, size_t length) {
  if(isServerCPMeasurementNotifyEnabled) {
    server_CP_Measurement_Chr->setValue(pData, length);
    server_CP_Measurement_Chr->notify();
  }
}

void CPS::client_CP_Measurement_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                              uint8_t* pData, size_t length, bool isNotify) {
  // Client CP Measurement data is tranferred to the Server
  if(operations->Laptop.IsConnected) 
    server_CP_Measurement_Chr_Notify(pData, length);

#ifdef DEBUG_CP_MEASUREMENT
  uint8_t buffer[length]= {}; 
  // Transfer first the contents of data to buffer (array of chars)
  memcpy(buffer, pData, length);
  std::string hexString = UTILS::getInstance()->toHexString(buffer, static_cast<uint8_t>(length));
  LOG(" -> Client Rec'd Raw CP Measurement Data: [%d] [%s]", length, hexString.c_str());
  uint8_t offset = 0;
  // Get flags field
  uint16_t flags = 0;
  memcpy(&flags, &buffer[offset], 2); // Transfer buffer fields to variable
  offset += 2;  // UINT16
  // Get Instantaneous Power values UINT16
  uint16_t PowerValue = 0;
  memcpy(&PowerValue, &buffer[offset], 2); // Transfer buffer fields to variable
  offset += 2;  // UINT16
  LOG("Instantaneous Power: %4d", PowerValue);
  // Get the other CP measurement values
  if ((flags & 1) != 0) {
    //  Power Balance Present
    LOG("Pedal Power Balance!");
  }
  if ((flags & 2) != 0) {
    // Accumulated Torque
    LOG("Accumulated Torque!");
  }
  // etcetera...
#endif
} // End cpmc_notify_callback

void CPS::Static_CP_Measurement_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                            uint8_t* pData, size_t length, bool isNotify) {
    if (CPS::instance) {
        CPS::instance->client_CP_Measurement_Notify_Callback(pBLERemoteCharacteristic, pData, length, isNotify);
    }
}

void CPS::xTaskServerIndicateResponse(void* parameter) {
    CPS* cpsInstance = (CPS*)parameter; // Local pointer
    while(true) {
        vTaskSuspend(cpsInstance->xTaskServerIndicateResponseHandle);  // Suspend now until resumed by callback
        if(cpsInstance->xTaskServerIndicateResponseDataLen > 0) {
            cpsInstance->server_CP_ControlPoint_Chr->setValue(cpsInstance->xTaskServerIndicateResponseData, \
                                                              cpsInstance->xTaskServerIndicateResponseDataLen); 
            cpsInstance->server_CP_ControlPoint_Chr->indicate();
        }
        cpsInstance->xTaskServerIndicateResponseDataLen = 0;  // Reset data length after processing
    }
};

void CPS::client_CP_ControlPoint_Indicate_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                            uint8_t* pData, size_t length, bool isNotify) {
  // The receipt of Control Point settings is acknowledged by the trainer: handle it
  // Send Client's Received Response message to the Server
  // ONLY in case of WAHOO unlock command response -> isServerCPControlPointIndicateEnabled NOT yet enabled!!
  // That is exactly what we want, afterall unlock was generated by the client-side of MITM and NOT by Zwift!
  if (length > sizeof(xTaskServerIndicateResponseData)) return;  // Prevent buffer overflow
  if(operations->Laptop.IsConnected && isServerCPControlPointIndicateEnabled && length > 0)  {
     memcpy(xTaskServerIndicateResponseData, pData, length);      // Store data for task
    xTaskServerIndicateResponseDataLen = length;
#ifdef DEBUG_CPS_CONTROLPOINT_RESPONSE
    icInterval = millis();
#endif
    vTaskResume(xTaskServerIndicateResponseHandle); // Resume the Server Indicate Response task
  }
#ifdef DEBUG_CPS_CONTROLPOINT_RESPONSE
  // Convert first the contents of data (array of chars) to HEX presentation
  std::string hexString = UTILS::getInstance()->toHexString(pData, static_cast<uint8_t>(length));
  LOG("[%5d] CPS Control Point Rsp: [%s]", (millis()-cpWriteTime), hexString.c_str());
#endif  
}

void CPS::Static_CP_ControlPoint_Indicate_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                              uint8_t* pData, size_t length, bool isNotify) {
    if (CPS::instance) {
        CPS::instance->client_CP_ControlPoint_Indicate_Callback(pBLERemoteCharacteristic, pData, length, isNotify);
    }
}

// Constructor
CPS::CPS() { 
  operations = OPS::getInstance();
}

// Destructor
CPS::~CPS() { }

CPS* CPS::getInstance() {
    if (CPS::instance == nullptr) {
        CPS::instance = new CPS();
    }
    return CPS::instance;
}

void CPS::server_setupCPS(NimBLEServer* pServer) {
    server_CyclingPower_Service = pServer->createService(UUID16_SVC_CYCLING_POWER);
    server_CP_ControlPoint_Chr = server_CyclingPower_Service->createCharacteristic(UUID16_CHR_CYCLING_POWER_CONTROL_POINT, 
                                                                            NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE);
    server_CP_ControlPoint_Chr->setCallbacks(new server_CP_ControlPoint_Chr_callback(this));
    // Start a CRITICAL task for off loaded handling Server CP Indicate Response message -> Core 0 and high priority
    // Check first if xTaskServerIndicateResponseHandle does not exist
    if(xTaskServerIndicateResponseHandle == NULL) {
      xTaskCreatePinnedToCore(this->xTaskServerIndicateResponse, "Indicate RSP", 4096, (void *)this, 24, \
                                                    &this->xTaskServerIndicateResponseHandle, xTaskCoreID0);
      LOG("Server CPS Control Point Indicate-Response Task created!");
    }
    server_CP_Measurement_Chr = server_CyclingPower_Service->createCharacteristic(UUID16_CHR_CYCLING_POWER_MEASUREMENT, 
                                                                            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    server_CP_Measurement_Chr->setCallbacks(new server_CP_Measurement_Chr_callback(this));

    server_CP_Feature_Chr = server_CyclingPower_Service->createCharacteristic(UUID16_CHR_CYCLING_POWER_FEATURE, 
                                                                            NIMBLE_PROPERTY::READ);
    // Set server CP Feature Flags field                                                                       
    server_CP_Feature_Chr->setValue(client_CP_Feature_Flags);  // Set default                                                                       
    server_CP_Location_Chr = server_CyclingPower_Service->createCharacteristic(UUID16_CHR_SENSOR_LOCATION, 
                                                                            NIMBLE_PROPERTY::READ);
    // Set server_CP_Location for sensor
    server_CP_Location_Chr->setValue(&client_CP_Location_Value, 1); // Set default 
    server_CyclingPower_Service->start();   
}

bool CPS::client_CyclingPower_Connect(NimBLEClient* pClient) {
    // Obtain a reference to the remote CP service.
    pRemote_CyclingPower_Service = pClient->getService(UUID16_SVC_CYCLING_POWER);
    if (pRemote_CyclingPower_Service == nullptr) {
      LOG("Mandatory Cycling Power Service: Not Found!");
      return false;
    }
    LOG("Client_CyclingPower_Service: Found!");
    pRemote_CP_ControlPoint_Chr = pRemote_CyclingPower_Service->getCharacteristic(UUID16_CHR_CYCLING_POWER_CONTROL_POINT);
    if (pRemote_CP_ControlPoint_Chr == nullptr)
      LOG("Client_CP_ControlPoint_Chr: Not Found!");
    else {
      LOG("Client_CP_ControlPoint_Chr: Found!"); 
      // -----------------------------------------------------------------------------------------------------------------------------------------
      // Start a CRITICAL task pRemote_CP_ControlPoint_Chr write-with-response -> Pin task to core 0 and high priority for best results!
      // Stack size after extensive testing -> 4096 (very stable!!)
      // Check first if xTaskClientWriteWithResponse does not exist
      if(xTaskClientWriteWithResponseHandle == NULL) { 
        xTaskCreatePinnedToCore(this->xTaskClientWriteWithResponse, "Write w Response", 4096, (void *)this, 24, \
                                &this->xTaskClientWriteWithResponseHandle, xTaskCoreID0);
        LOG("Client CPS ControlPoint xTask Write-With-Response created!");
      }
      // ----------------------------------------------------------------------------------------------------------------------------------------- 
      if(!pRemote_CP_ControlPoint_Chr->canIndicate()) {
          LOG("Mandatory Client_CP_ControlPoint_Chr: Cannot Indicate!");
          return false;
      }
    }
    pRemote_CP_Measurement_Chr = pRemote_CyclingPower_Service->getCharacteristic(UUID16_CHR_CYCLING_POWER_MEASUREMENT);
    if (pRemote_CP_Measurement_Chr == nullptr) {
      LOG("Mandatory client_CP_Measurement_Chr: Not Found!");
      return false;
    } 
    LOG("Client_CP_Measurement_Chr: Found!");  
    if(!pRemote_CP_Measurement_Chr->canNotify()) {
      LOG("Mandatory Client_CP_Measurement_Chr: Cannot Notify!");
      return false;
    } 
    pRemote_CP_Feature_Chr = pRemote_CyclingPower_Service->getCharacteristic(UUID16_CHR_CYCLING_POWER_FEATURE);
    if (pRemote_CP_Feature_Chr == nullptr) {
      LOG("Mandatory Client_CP_Feature_Chr: Not Found!");
      return false;     
    }
    LOG("Client_CP_Feature_Chr: Found!");
    // Read the value of the characteristic.
    if(pRemote_CP_Feature_Chr->canRead()) {
       // Read 32-bit client_CP_Feature_Chr value
      client_CP_Feature_Flags = pRemote_CP_Feature_Chr->readValue<uint32_t>();
      if(server_CP_Feature_Chr) {
        server_CP_Feature_Chr->setValue(client_CP_Feature_Flags); // Transfer/Update the value to the server side
      }
#ifdef DEBUG
      const uint8_t CPFC_FIXED_DATALEN = 4;
      uint8_t cpfcData[CPFC_FIXED_DATALEN] = {static_cast<uint8_t>(client_CP_Feature_Flags & 0xff), \
                                              static_cast<uint8_t>(client_CP_Feature_Flags >> 8), \
                                              static_cast<uint8_t>(client_CP_Feature_Flags >> 16), \
                                              static_cast<uint8_t>(client_CP_Feature_Flags >> 24)};
      std::string hexString = UTILS::getInstance()->toHexString(cpfcData, CPFC_FIXED_DATALEN);                                    
      LOG(" -> Client Reads Raw CP Feature bytes: [4] [%s]", hexString.c_str());
      for (int i = 0; i < client_CP_Feature_Len; i++) {
        if ( client_CP_Feature_Flags & (1 << i) )
          LOG("  %s", client_CP_Feature_Str[i]);
      }
#endif
      } // canRead Feature
    pRemote_CP_Location_Chr = pRemote_CyclingPower_Service->getCharacteristic(UUID16_CHR_SENSOR_LOCATION);
    if (pRemote_CP_Location_Chr == nullptr) {
      LOG("Client_CP_Location_Chr: Not Found!");
    } else {
      LOG("Client_CP_Location_Chr: Found!");
      // Read the value of the characteristic.
      if(pRemote_CP_Location_Chr->canRead()) {
        client_CP_Location_Value = pRemote_CP_Location_Chr->readValue<uint8_t>(); 
        if(server_CP_Location_Chr)
          server_CP_Location_Chr->setValue(&client_CP_Location_Value, 1); // Transfer/Update to the server side
        // CP sensor location value is 8 bit
#ifdef DEBUG
        if(client_CP_Location_Value <= client_CP_Sensor_Location_Str_Len) 
          LOG(" -> Client Reads CP Location Sensor: Loc#: %d %s", client_CP_Location_Value, \
                                                        client_CP_Sensor_Location_Str[client_CP_Location_Value]);
        else 
          LOG(" -> Client Reads CP Location Sensor: Loc#: %d", client_CP_Location_Value); 
#endif
      }
    }   
    return true;    
}

void CPS::client_CP_Subscribe(void) { 
  if ( pRemote_CP_ControlPoint_Chr != nullptr ) {
    if( !pRemote_CP_ControlPoint_Chr->subscribe(indications, Static_CP_ControlPoint_Indicate_Callback) )
      LOG(">>> ERROR remote CP Control Point Subscribe failed!");
    else {
#ifdef ENABLE_WAHOOCPS 
      // --------------------------Unlock the Wahoo trainer ------------------------------------
      delay(5); // Time to settle after subscribe
    #ifdef DEBUG_CPS_CONTROLPOINT
      cpWriteTime = millis(); // Set current Time to measure acknowledged time later
    #endif
      xTaskClientWriteWithResponseData = std::string(reinterpret_cast<char const*>(unlockCommand));
      xTaskClientWriteWithResponseDataReady = true;      // Server Control Point data are ready!
      vTaskResume(xTaskClientWriteWithResponseHandle);   // Resume xTask and write the data a.s.a.p.
      LOG("Client sends CP Control Point: [20 EE FC] -> Wahoo Unlock Command Key");
      delay(50); // Give the Wahoo trainer some time to wake up
      // -------------------------------------------------------------------------------------- 
#endif
    }
  }
  if ( pRemote_CP_Measurement_Chr != nullptr )
    if( !pRemote_CP_Measurement_Chr->subscribe(notifications, Static_CP_Measurement_Notify_Callback) )
      LOG(">>> ERROR remote CP Measure Subscribe failed!");               
};

void CPS::client_CP_Unsubscribe(void) {
  if ( pRemote_CP_ControlPoint_Chr != nullptr )
    if( !pRemote_CP_ControlPoint_Chr->unsubscribe(false) )
      LOG(">>> ERROR remote CP Control Point UNsubscribe failed!");

  if ( pRemote_CP_Measurement_Chr != nullptr )
    if( !pRemote_CP_Measurement_Chr->unsubscribe(false) )
      LOG(">>> ERROR remote CP Measure UNsubscribe failed!");
};

