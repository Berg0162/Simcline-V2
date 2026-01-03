#include "FitnessMachine.h"

// Include operations control settings class
#include "Operations.h"
// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress DEBUG messages that help debugging...
// Uncomment general "#define DEBUG" to activate debug messages for this class
#define DEBUG
// Include these debug utility macros in all cases!
#include "config/configDebug.h"
// ------------------------------------------------------------------------------------------------

#include "ClientSide.h"
#include "Utilities.h"

#ifdef DEBUG
//  Restrict activating one or more of the following << EXTRA >> debug directives --> process intensive 
//  The overhead can lead to spurious side effects and a loss of quality of service handling!!
//#define DEBUG_FTM_INDOORBIKEDATA// If defined allows for parsing the Indoor Bike Data
#ifdef DEBUG_FTM_INDOORBIKEDATA
//#define DEBUG_DECODE_IBD        // If defined allows for decoding the Indoor Bike Data
#endif
//#define DEBUG_FTM_TRAININGSTATUS// If defined allows for parsing the Training Status Data
//#define DEBUG_FTM_STATUS        // If defined allows for parsing the Machine Status Data
//#define DEBUG_FTM_CONTROLPOINT_RESPONSE           // If defined allows for parsing the Data
#define DEBUG_FTM_CONTROLPOINT_OPCODE_DATA_RAW    // If defined allows for parsing Raw Data
#define DEBUG_FTM_CONTROLPOINT_OPCODE_DATA_DECODE // If defined allows for decoding Raw Data
#endif

/* --------------------------------------------------------------------------------------
 * Fitness Machine Service, uuid 0x1826 or 00001826-0000-1000-8000-00805F9B34FB
*/
#define UUID16_SVC_FITNESS_MACHINE                            NimBLEUUID((uint16_t)0x1826)
#define UUID16_CHR_FITNESS_MACHINE_FEATURE                    NimBLEUUID((uint16_t)0x2ACC)
#define UUID16_CHR_INDOOR_BIKE_DATA                           NimBLEUUID((uint16_t)0x2AD2)
#ifdef ENABLE_TRAININGSTATUS
#define UUID16_CHR_TRAINING_STATUS                            NimBLEUUID((uint16_t)0x2AD3)
#endif
//#define UUID16_CHR_SUPPORTED_SPEED_RANGE                      NimBLEUUID((uint16_t)0x2AD4)
//#define UUID16_CHR_SUPPORTED_INCLINATION_RANGE                NimBLEUUID((uint16_t)0x2AD5)
#define UUID16_CHR_SUPPORTED_RESISTANCE_LEVEL_RANGE           NimBLEUUID((uint16_t)0x2AD6)
//#define UUID16_CHR_SUPPORTED_HEART_RATE_RANGE                 NimBLEUUID((uint16_t)0x2AD7)
#define UUID16_CHR_SUPPORTED_POWER_RANGE                      NimBLEUUID((uint16_t)0x2AD8)
#define UUID16_CHR_FITNESS_MACHINE_CONTROL_POINT              NimBLEUUID((uint16_t)0x2AD9)
#define UUID16_CHR_FITNESS_MACHINE_STATUS                     NimBLEUUID((uint16_t)0x2ADA)
//const uint8_t FTM_SRLR_FIXED_DATALEN = 6;
//const uint8_t FTM_SPR_FIXED_DATALEN = 6;
//const uint8_t FTM_FEATURE_FIXED_DATALEN = 8;

#ifdef DEBUG
// ---------------------Fitness Machine Features (bytes 1-4):
const uint8_t client_FTM_Feature_Str_One_Len = 32;
const char* client_FTM_Feature_Str_One[client_FTM_Feature_Str_One_Len] = {
"Average Speed Supported", "Cadence Supported", "Total Distance Supported", "Inclination Supported",
"Elevation Gain Supported", "Pace Supported", "Step Count Supported", "Resistance Level Supported",
"Stride Count Supported", "Expended Energy Supported", "Heart Rate Measurement Supported", "Metabolic Equivalent Supported",
"Elapsed Time Supported", "Remaining Time Supported", "Power Measurement Supported", "Force on Belt and Power Output Supported",
"User Data Retention Supported", "Reserved for Future Use","","","","","","","","","","","","","",""};
// ---------------------Fitness Machine Target Setting Features (bytes 5-8) :
const uint8_t client_FTM_Feature_Str_Two_Len = 32;
const char* client_FTM_Feature_Str_Two[client_FTM_Feature_Str_Two_Len] = {
"Speed Target Setting Supported", "Inclination Target Setting Supported", "Resistance Target Setting Supported", "Power Target Setting Supported",
"Heart Rate Target Setting Supported", "Targeted Expended Energy Configuration Supported", "Targeted Step Number Configuration Supported",
"Targeted Stride Number Configuration Supported", "Targeted Distance Configuration Supported", "Targeted Training Time Configuration Supported",
"Targeted Time in Two Heart Rate Zones Configuration Supported", "Targeted Time in Three Heart Rate Zones Configuration Supported",
"Targeted Time in Five Heart Rate Zones Configuration Supported", "Indoor Bike Simulation Parameters Supported", "Wheel Circumference Configuration Supported",
"Spin Down Control Supported", "Targeted Cadence Configuration Supported", "Reserved for Future Use","","","","","","","","","","","","","",""};
#endif

// Default values to start server-side with
const unsigned char defaultResistance[6] = {0x00, 0x00, 0xE8, 0x03, 0x01, 0x00};
const unsigned char defaultPower[6] = {0x00, 0x00, 0xA0, 0x0F, 0x01, 0x00};
const unsigned char defaultFeature[8] = {0x83, 0x44, 0x00, 0x00, 0x00, 0x20, 0x00, 0x00};

// Initialize the static member
FTMS* FTMS::instance = nullptr;

// ---------------------------------------------------------------------------------------
class server_FTM_Status_Chr_callback final : public NimBLECharacteristicCallbacks { 
private:
    FTMS* ftmsInstance;  // LOCAL Pointer to the FTMS class instance
public:
    // Constructor to accept a pointer to the FTMS class instance
    server_FTM_Status_Chr_callback(FTMS* instance) : ftmsInstance(instance) {}
protected:
    inline void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
      ftmsInstance->serverFTMStatusOnSubscribe(pCharacteristic, connInfo, subValue);
    }
}; 

void FTMS::serverFTMStatusOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) {
    std::string str = std::string(pCharacteristic->getUUID());
    if(subValue == 0) {
      LOG("Central Unsubscribed: [%s] FTM Machine Status", str.c_str());
      isServerFTMStatusNotifyEnabled = false;
    } else {
      LOG("Central Subscribed: [%s] FTM Machine Status", str.c_str());
      isServerFTMStatusNotifyEnabled = true;
    }
}

class server_FTM_IndoorBikeData_Chr_callback final : public NimBLECharacteristicCallbacks { 
private:
    FTMS* ftmsInstance;  // LOCAL Pointer to the FTMS class instance
public:
    // Constructor to accept a pointer to the FTMS class instance
    server_FTM_IndoorBikeData_Chr_callback(FTMS* instance) : ftmsInstance(instance) {}
protected:
    inline void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
      ftmsInstance->serverFTMIndoorBikeDataOnSubscribe(pCharacteristic, connInfo, subValue);
    }
}; 

void FTMS::serverFTMIndoorBikeDataOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) {
    std::string str = std::string(pCharacteristic->getUUID());
    if(subValue == 0) {
      LOG("Central Unsubscribed: [%s] FTM Indoor Bike Data", str.c_str());
      isServerFTMIndoorBikeDataNotifyEnabled = false;
    } else {
      LOG("Central Subscribed: [%s] FTM Indoor Bike Data", str.c_str());
      isServerFTMIndoorBikeDataNotifyEnabled = true;
    }
}

#ifdef ENABLE_TRAININGSTATUS
class server_FTM_TrainingStatus_Chr_callback final : public NimBLECharacteristicCallbacks { 
private:
    FTMS* ftmsInstance;  // LOCAL Pointer to the FTMS class instance
public:
    // Constructor to accept a pointer to the FTMS class instance
    server_FTM_TrainingStatus_Chr_callback(FTMS* instance) : ftmsInstance(instance) {}
protected:
    inline void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
      ftmsInstance->serverFTMTrainingStatusOnSubscribe(pCharacteristic, connInfo, subValue);
    }
}; 

void FTMS::serverFTMTrainingStatusOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) {
    std::string str = std::string(pCharacteristic->getUUID());
    if(subValue == 0) {
      LOG("Central Unsubscribed: [%s] FTM Training Status", str.c_str());
      isServerFTMTrainingStatusNotifyEnabled = false;
    } else {
      LOG("Central Subscribed: [%s] FTM Training Status", str.c_str());
      isServerFTMTrainingStatusNotifyEnabled = true;
    }
}
#endif

class server_FTM_ControlPoint_Chr_callback final : public NimBLECharacteristicCallbacks {
private:
    FTMS* ftmsInstance;  // LOCAL Pointer to the CPS class instance
public:
    // Constructor to accept a pointer to the CPS class instance
    server_FTM_ControlPoint_Chr_callback(FTMS* instance) : ftmsInstance(instance) { }
protected:
    inline void onWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo) override {
        ftmsInstance->serverFTMControlPointOnWrite(pCharacteristic, connInfo);
    }
    inline void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
        ftmsInstance->serverFTMControlPointOnSubscribe(pCharacteristic, connInfo, subValue);
    }
    void onStatus(NimBLECharacteristic* pCharacteristic, int code) {
      ftmsInstance->isClientCPWriteAcknowledged = true;
#ifdef DEBUG_FTM_CONTROLPOINT_RESPONSE
      if(code == BLE_HS_EDONE) {
        LOG("[%5d] Indication confirmed!", (millis()-ftmsInstance->icInterval) );
      } else if(code == 0) {
        LOG("Notification Success!");
      } else {
        LOG("Status: [%d]", code);
      }
#endif 
    }
};

// -------------------------------------------------------------------------------------
/* Return Code = 0x06 --> BLE_HS_ENOMEM --> Operation failed due to resource exhaustion.
* This issue occurs when we try to send too many packets before controller can process them. In this case, the packets get queued. 
* As we queue more number of packets, we reach a point where we run out of memory. In this case, BLE_HS_ENOMEM is returned.
*/
void FTMS::xTaskClientWriteWithResponse(void *parameter) {
  FTMS* ftmsInstance = (FTMS *)parameter;
  while(true) {
      vTaskSuspend(ftmsInstance->xTaskClientWriteWithResponseHandle); // At the beginning ensures: task only runs when resumed.
      if(ftmsInstance->xTaskClientWriteWithResponseDataReady) {
          // Copy to this scope as it will be handled asynchronously by another core and NimBLE may only write garbage
          std::string localCopy = ftmsInstance->xTaskClientWriteWithResponseData; 
          if (!ftmsInstance->pRemote_FTM_ControlPoint_Chr->writeValue(localCopy, true))
              LOG(">>> Error: Client FTMS_CP Failed to Write-with-Response!");
          ftmsInstance->xTaskClientWriteWithResponseDataReady = false; // Indicate: ready for next data transfer
      }
  } // while
};

void FTMS::serverFTMControlPointOnWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo& connInfo) {
#ifdef DEBUG_FTM_CONTROLPOINT_OPCODE_DATA_RAW
  static long cpInterval;
  static long cpPrevTime = millis();
  cpInterval = millis() - cpPrevTime; // Time between consecutive server_FTM_ControlPoint calls
  cpPrevTime = millis();
  cpWriteTime = cpPrevTime; // extra for response timing 
#endif
  static uint8_t unresponsiveCnt = 0;
  static uint8_t missedCPWriteAcknowledgedCnt = 0;
  // Fill xTaskClientWriteWithResponseData for processing/decoding
  xTaskClientWriteWithResponseData = server_FTM_ControlPoint_Chr->getValue();
  size_t ftmcpDataLen = xTaskClientWriteWithResponseData.length();
  // Check ftmcp data is not empty!
  if(!ftmcpDataLen) return; // We are done!
  // Just pass on to Client and process later!  
  // Verify valid connection, has subscribed and previous CP write has been acknowledged
  // Threshold for missing Client-CP-Write-Acknowledge message is 2 in a row !
  if(isClientCPWriteAcknowledged) missedCPWriteAcknowledgedCnt = 0;
  else missedCPWriteAcknowledgedCnt++;
  if(operations->Trainer.IsConnected && ClientSide::getInstance()->hasSubscribedToAll && (missedCPWriteAcknowledgedCnt < 3) ) { 
    if(xTaskClientWriteWithResponseDataReady == false) { // Previous xTask Write With Response is finished
      xTaskClientWriteWithResponseDataReady = true;      // Server FTM Control Point data are ready!
      isClientCPWriteAcknowledged = false;
      unresponsiveCnt = 0;
      vTaskResume(xTaskClientWriteWithResponseHandle);   // Resume xTask and write the data a.s.a.p.
    }
  } else { // Training App keeps sending FTM_CP-Data when Client/Trainer is disconnected or unresponsive!!
    if(unresponsiveCnt++ < 2) { // Limit active response to this state
      LOG(">>> FTMS Trainer is unresponsive! Skipped CP Data!");
      server_Idle_State_Notify();
    } else ClientSide::getInstance()->pClient->disconnect();
    return; // Ignore App data until the trainer reconnects again
  }

  // Process xTaskClientWriteWithResponseData for presentation and/or Simcline operation settings!!
  // Transfer the contents of data to server_FTM_Control_Point_Data.bytes
  memcpy(server_FTM_Control_Point_Data.bytes, xTaskClientWriteWithResponseData.data(), ftmcpDataLen);
#ifdef DEBUG_FTM_CONTROLPOINT_OPCODE_DATA_RAW  
  // Convert first the contents of data (array of chars) to HEX presentation
  std::string hexString = UTILS::getInstance()->toHexString(server_FTM_Control_Point_Data.values.OCTETS, ftmcpDataLen-1);
  LOG("[%5d] FTM Control Point Req: [%02X] [%s]", cpInterval, server_FTM_Control_Point_Data.values.OPCODE, hexString.c_str());
#endif 

  // Decodes an incoming Fitness Machine Control Point request
  switch(server_FTM_Control_Point_Data.values.OPCODE) {
#ifdef DEBUG_FTM_CONTROLPOINT_OPCODE_DATA_DECODE
    case ftmcpRequestControl: {
      LOG("Request Control of Machine!");
      break;
    }
    case ftmcpStartOrResume: {
      LOG("Start or Resume Machine!");
      break;
    }
    case ftmcpStopOrPause: {
      LOG("Stop or Pause Machine, Parameter: Stop!");
      break;
    }
    case ftmcpReset: {
      LOG("Reset Machine!");
      break;
    }
#endif
    case ftmcpSetIndoorBikeSimulationParameters: {
#ifdef DEBUG_FTM_CONTROLPOINT_OPCODE_DATA_DECODE
      // Short is 16 bit signed, so the windspeed is converted from two bytes to signed value. Highest bit is sign bit
      short ws = (server_FTM_Control_Point_Data.values.OCTETS[0] << 8) + server_FTM_Control_Point_Data.values.OCTETS[1]; 
      float wind_speed = ws / 1000.0; // meters per second, resolution 0.001
#endif
      // Short is 16 bit signed, so a negative grade is correctly converted from two bytes to signed value. Highest bit is sign bit
      short gr = (server_FTM_Control_Point_Data.values.OCTETS[3] << 8) + server_FTM_Control_Point_Data.values.OCTETS[2];
      float grade = (float)(gr/100.0); // percentage, resolution 0.01
      operations->setNewGrade(grade);
#ifdef DEBUG_FTM_CONTROLPOINT_OPCODE_DATA_DECODE
      float crr = server_FTM_Control_Point_Data.values.OCTETS[4] / 10000.0; // Coefficient of rolling resistance, resolution 0.0001
      float cw = server_FTM_Control_Point_Data.values.OCTETS[5] / 100.0; // Wind resistance Kg/m, resolution 0.01;
      LOG("Set Indoor Bike Simulation Parameters Ws: %5.3f Gr: %6.2f Crr: %6.4f Cw: %4.2f", wind_speed, grade, crr, cw);
#endif
      break;             
    }
    case ftmcpSetTargetResistanceLevel:
    case ftmcpSetTargetSpeed:
    case ftmcpSetTargetInclination:
    case ftmcpSetTargetPower:
    case ftmcpSetTargetHeartRate:
    case ftmcpSetTargetedExpendedEngery:
    case ftmcpSetTargetedNumberOfSteps:
    case ftmcpSetTargetedNumberOfStrided:
    case ftmcpSetTargetedDistance:
    case ftmcpSetTargetedTrainingTime:
    case ftmcpSetTargetedTimeInTwoHeartRateZones:
    case ftmcpSetTargetedTimeInThreeHeartRateZones:
    case ftmcpSetTargetedTimeInFiveHeartRateZones:
    case ftmcpSetWheelCircumference:
    case ftmcpSetSpinDownControl:
    case ftmcpSetTargetedCadence: {
      LOG(" >>> Info: FTM_CONTROLPOINT call with Unresolved OpCode!");
      break;
      }
    } // switch
}; // onWrite

void FTMS::serverFTMControlPointOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) {
        std::string str = std::string(pCharacteristic->getUUID());
        if(subValue == 0) {
          LOG("Central Unsubscribed: [%s] FTM Control Point", str.c_str());
          isServerFTMControlPointIndicateEnabled = false;
        } else {
          LOG("Central Subscribed: [%s] FTM Control Point", str.c_str());
          isServerFTMControlPointIndicateEnabled = true;
        }
} // onSubscribe

// Constructor
FTMS::FTMS() { 
    operations = OPS::getInstance();
}

// Destructor
FTMS::~FTMS() { }

FTMS* FTMS::getInstance() {
    if (FTMS::instance == nullptr) {
        FTMS::instance = new FTMS();
    }
    return FTMS::instance;
}

void FTMS::server_setupFTMS(NimBLEServer* pServer) {   
    server_FitnessMachine_Service = pServer->createService(UUID16_SVC_FITNESS_MACHINE);
    server_FTM_Feature_Chr = server_FitnessMachine_Service->createCharacteristic(UUID16_CHR_FITNESS_MACHINE_FEATURE, 
                                                                            NIMBLE_PROPERTY::READ);
    server_FTM_Feature_Chr->setValue(defaultFeature, 8); // Set default

    server_FTM_Status_Chr = server_FitnessMachine_Service->createCharacteristic(UUID16_CHR_FITNESS_MACHINE_STATUS, 
                                                                            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    server_FTM_Status_Chr->setCallbacks(new server_FTM_Status_Chr_callback(this)); //NIMBLE    
    server_FTM_IndoorBikeData_Chr = server_FitnessMachine_Service->createCharacteristic(UUID16_CHR_INDOOR_BIKE_DATA, 
                                                                            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    server_FTM_IndoorBikeData_Chr->setCallbacks(new server_FTM_IndoorBikeData_Chr_callback(this)); //NIMBLE                                                                                                                                                           
    server_FTM_SupportedResistanceLevelRange_Chr = server_FitnessMachine_Service->createCharacteristic(UUID16_CHR_SUPPORTED_RESISTANCE_LEVEL_RANGE, 
                                                                            NIMBLE_PROPERTY::READ);
    server_FTM_SupportedResistanceLevelRange_Chr->setValue(defaultResistance, 6); // Set default
                                                                                 
    server_FTM_SupportedPowerRange_Chr = server_FitnessMachine_Service->createCharacteristic(UUID16_CHR_SUPPORTED_POWER_RANGE, 
                                                                            NIMBLE_PROPERTY::READ);
    server_FTM_SupportedPowerRange_Chr->setValue(defaultPower, 6); // Set default
#ifdef ENABLE_TRAININGSTATUS 
    server_FTM_TrainingStatus_Chr = server_FitnessMachine_Service->createCharacteristic(UUID16_CHR_TRAINING_STATUS, 
                                                                            NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::NOTIFY);
    server_FTM_TrainingStatus_Chr->setCallbacks(new server_FTM_TrainingStatus_Chr_callback(this)); //NIMBLE 
#endif
    server_FTM_ControlPoint_Chr = server_FitnessMachine_Service->createCharacteristic(UUID16_CHR_FITNESS_MACHINE_CONTROL_POINT, 
                                                                            NIMBLE_PROPERTY::WRITE | NIMBLE_PROPERTY::INDICATE);
    server_FTM_ControlPoint_Chr->setCallbacks(new server_FTM_ControlPoint_Chr_callback(this)); 
    // Start a CRITICAL task for off loaded handling Server CP Indicate Response message -> Core 0 and high priority
    // Check first if xTaskServerIndicateResponseHandle does not exist
    if(xTaskServerIndicateResponseHandle == NULL) {
      xTaskCreatePinnedToCore(this->xTaskServerIndicateResponse, "Indicate RSP", 4096, (void *)this, 24, \
                              &this->xTaskServerIndicateResponseHandle, xTaskCoreID0);
      LOG("Server FTM Control Point Indicate-Response Task created!");
    }
    server_FitnessMachine_Service->start();      
}

#ifdef ENABLE_TRAININGSTATUS
void FTMS::client_FTM_TrainingStatus_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  // Client FTM Training Status data is tranferred to the Server
  if(operations->Laptop.IsConnected)
      server_FTM_TrainingStatus_Chr_Notify(pData, length);
#ifdef DEBUG_FTM_TRAININGSTATUS
  std::string hexString = UTILS::getInstance()->toHexString(pData, static_cast<uint8_t>(length));
  LOG(" -> Client Rec'd Raw FTM Training Status Data: [%d] [%s]", length, hexString,c_str());
#endif
}

void FTMS::Static_FTM_TrainingStatus_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    if (FTMS::instance) {
        FTMS::instance->client_FTM_TrainingStatus_Notify_Callback(pBLERemoteCharacteristic, pData, length, isNotify);
    }
}
#endif

void FTMS::client_FTM_Status_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
  // Client FTM Status data is tranferred to the Server
  if(operations->Laptop.IsConnected)
      server_FTM_Status_Chr_Notify(pData, length);
#ifdef DEBUG_FTM_STATUS
  std::string hexString = UTILS::getInstance()->toHexString(pData, static_cast<uint8_t>(length));
  LOG(" -> Client Rec'd Raw FTM Machine Status Data: [%d] [%s]", length, hexString.c_str());
#endif
}

void FTMS::Static_FTM_Status_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) {
    if (FTMS::instance) {
        FTMS::instance->client_FTM_Status_Notify_Callback(pBLERemoteCharacteristic, pData, length, isNotify);
    }
}

void FTMS::client_FTM_IndoorBikeData_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify)
{
  // Client FTM Indoor Bike Data is tranferred to the Server
  if(operations->Laptop.IsConnected)
      server_FTM_IndoorBikeData_Chr_Notify(pData, length); 
#ifdef DEBUG_FTM_INDOORBIKEDATA
  std::string hexString = UTILS::getInstance()->toHexString(pData, static_cast<uint8_t>(length));
  LOG(" -> Client Rec'd Raw FTM Indoor Bike Data: [%d] [%s]", length, hexString.c_str());
#endif
// Decoding Indoor Bike Data and presenting
// ---> Buffer Data Length depends on data flagged to be present !!!!
#ifdef DEBUG_DECODE_IBD
  uint8_t offset = 0;
  uint16_t flags = 0;
  uint8_t IBDDataBuf[length] = {};
  // Transfer the contents of pData to IBDDataBuf
  memcpy(IBDDataBuf, pData, length);
  memcpy(&flags, &IBDDataBuf[offset], 2); // Transfer buffer fields to variable
  offset += 2;  // UINT16
  if ((flags & 1) == 0) { // Inst. Speed is present (!) if flag (bit 0) is set to 0 (null)
    //  More Data --> Instantaneous Speed 0.01 
    uint16_t inst_Speed = 0;
    memcpy(&inst_Speed, &IBDDataBuf[offset], 2); // Transfer buffer fields to variable
    LOG(" Instant. Speed: %d KPH", (inst_Speed/100));
    offset += 2;  // UINT16 
    }
  if ((flags & 2) != 0) { 
    //  Average Speed 0.01 
    uint16_t av_Speed = 0;
    memcpy(&av_Speed, &IBDDataBuf[offset], 2); // Transfer buffer fields to variable
    LOG(" Average Speed: %d KPH", (av_Speed/100));
    offset += 2;  // UINT16 
    }
  if ((flags & 4) != 0) {
    //  Instantaneous Cadence 0.5
    uint16_t inst_Cadence = 0;
    memcpy(&inst_Cadence, &IBDDataBuf[offset], 2); // Transfer buffer fields to variable
    LOG(" Instantaneous Cadence: %d RPM", (inst_Cadence/2));
    offset += 2;  // UINT16 
    }
  if ((flags & 8) != 0) {
    //  Average Cadence 0.5
    uint16_t av_Cadence = 0;
    memcpy(&av_Cadence, &IBDDataBuf[offset], 2); // Transfer buffer fields to variable
    LOG(" Average Cadence: %d RPM", (av_Cadence/2));
    offset += 2;  // UINT16 
    }
  if ((flags & 16) != 0) {
    //  Total Distance  1
    // Little endian format, transfer 24 bit to 32 bit variable
    uint32_t tot_Distance = 0;
    memcpy(&tot_Distance, &IBDDataBuf[offset], 3); // Transfer buffer fields to variable
    LOG(" Total Distance: %d m", tot_Distance);
    offset += 3;  // UINT24 16 + 8
    }
  if ((flags & 32) != 0) {
    //  Resistance Level  1
    uint16_t res_Level = 0;
    memcpy(&res_Level, &IBDDataBuf[offset], 2); // Transfer buffer fields to variable
    LOG(" Resistance Level: %d ", res_Level);
    offset += 2;  // UINT16 
    }
  if ((flags & 64) != 0) {
    //  Instantaneous Power 1
    uint16_t inst_Power = 0;
    memcpy(&inst_Power, &IBDDataBuf[offset], 2); // Transfer buffer fields to variable
    LOG(" Instantaneous Power: %d Watt", inst_Power);
    offset += 2;  // UINT16 
    }
  if ((flags & 128) != 0) {
    //  Average Power 1
    uint16_t av_Power = 0;
    memcpy(&av_Power, &IBDDataBuf[offset], 2); // Transfer buffer fields to variable
    LOG(" Average Power: %d Watt", av_Power);
    offset += 2;  // UINT16 
    }
  if ((flags & 256) != 0) {
    //  Expended Energy -> UINT16 UINT16 UINT8
    // Total Energy UINT16  1
    uint16_t tot_Energy = 0;
    memcpy(&tot_Energy, &IBDDataBuf[offset], 2); // Transfer buffer fields to variable
    LOG(" Tot. Energy: %d kCal", tot_Energy);
    offset += 2;  // UINT16 
    // Energy per hour UINT16 1
    uint16_t Energy_hr = 0;
    memcpy(&Energy_hr, &IBDDataBuf[offset], 2); // Transfer buffer fields to variable
    LOG(" Energy/hr: %d kCal/hr", Energy_hr);
    offset += 2;  // UINT16 
    // Energy per minute UINT8  1
    uint8_t Energy_pm = 0;
    memcpy(&Energy_pm, &IBDDataBuf[offset], 1); // Transfer buffer fields to variable
    LOG(" Energy/m: %d kCal/m", Energy_pm);
    offset += 1;  // UINT8 
    }
  if ((flags & 512) != 0) {
    //  Heart Rate  1
    uint8_t Heart_Rate = 0;
    memcpy(&Heart_Rate, &IBDDataBuf[offset], 1); // Transfer buffer fields to variable
    LOG(" Heart Rate: %d HBM", Heart_Rate);
    offset += 1;  // UINT8 
    }
  if ((flags & 1024) != 0) {
    //  Metabolic Equivalent 0.1
    uint8_t Mets = 0;
    memcpy(&Mets, &IBDDataBuf[offset], 1); // Transfer buffer fields to variable
    LOG(" Metabolic Equivalent: %d ", Mets/10);
    offset += 1;  // UINT8 
    }
  if ((flags & 2048) != 0) {
    //  Elapsed Time  1
    uint16_t elap_time = 0;
    memcpy(&elap_time, &IBDDataBuf[offset], 2); // Transfer buffer fields to variable
    LOG(" Elapsed time: %d s", elap_time);
    offset += 2;  // UINT16 
    }
  if ((flags & 4096) != 0) {
    //  Remaining Time  1
    uint16_t rem_time = 0;
    memcpy(&rem_time, &IBDDataBuf[offset], 2); // Transfer buffer fields to variable
    LOG(" Remaining time: %d s", rem_time);
    offset += 2;  // UINT16 
    }
#endif
}

void FTMS::Static_FTM_IndoorBikeData_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                                    uint8_t* pData, size_t length, bool isNotify) {
    if (FTMS::instance) {
        FTMS::instance->client_FTM_IndoorBikeData_Notify_Callback(pBLERemoteCharacteristic, pData, length, isNotify);
    }
}

void FTMS::xTaskServerIndicateResponse(void* parameter) {
    FTMS* ftmsInstance = (FTMS*)parameter; // Local pointer
    while(true) {
        vTaskSuspend(ftmsInstance->xTaskServerIndicateResponseHandle); // At the beginning ensures: task only runs when resumed.
        if(ftmsInstance->xTaskServerIndicateResponseDataLen > 0) {
            ftmsInstance->server_FTM_ControlPoint_Chr->setValue(ftmsInstance->xTaskServerIndicateResponseData, \
                                                                ftmsInstance->xTaskServerIndicateResponseDataLen);
            ftmsInstance->server_FTM_ControlPoint_Chr->indicate();
        }
        ftmsInstance->xTaskServerIndicateResponseDataLen = 0;  // Reset data length after processing
    } // while
};

void FTMS::client_FTM_ControlPoint_Indicate_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                                    uint8_t* pData, size_t length, bool isNotify) {
  // The receipt of Control Point settings is acknowledged by the trainer: handle it
  // Send Client's Received Response message to the Server
  if (length > sizeof(xTaskServerIndicateResponseData)) return;  // Prevent buffer overflow
  if(operations->Laptop.IsConnected && isServerFTMControlPointIndicateEnabled && length > 0) {
    memcpy(xTaskServerIndicateResponseData, pData, length);      // Store data for task
    xTaskServerIndicateResponseDataLen = length;
#ifdef DEBUG_FTM_CONTROLPOINT_RESPONSE
    icInterval = millis();
#endif
    vTaskResume(xTaskServerIndicateResponseHandle); // Resume the Server Indicate Response task
  }
#ifdef DEBUG_FTM_CONTROLPOINT_RESPONSE
  // Convert first the contents of data (array of chars) to HEX presentation
  std::string hexString = UTILS::getInstance()->toHexString(pData, static_cast<uint8_t>(length));
  LOG("[%5d] FTM Control Point Rsp: [%s]", (millis()-cpWriteTime), hexString.c_str());
#endif  
};

void FTMS::Static_FTM_ControlPoint_Indicate_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                                      uint8_t* pData, size_t length, bool isNotify) {
    if (FTMS::instance) {
        FTMS::instance->client_FTM_ControlPoint_Indicate_Callback(pBLERemoteCharacteristic, pData, length, isNotify);
    }
}

bool FTMS::client_FitnessMachine_Connect(NimBLEClient* pClient) {
    // Obtain a reference to the remote FTMS service.
    pRemote_FitnessMachine_Service = pClient->getService(UUID16_SVC_FITNESS_MACHINE);
    if (pRemote_FitnessMachine_Service == nullptr) {
      LOG("Mandatory client_FitnessMachine_Service: Not Found!");
      return false;
    }
    LOG("Client_FitnessMachine_Service: Found!");

    pRemote_FTM_Feature_Chr = pRemote_FitnessMachine_Service->getCharacteristic(UUID16_CHR_FITNESS_MACHINE_FEATURE);
    if (pRemote_FTM_Feature_Chr == nullptr) {
      LOG("Mandatory Client_FTM_Feature_Chr: Not Found!");
      return false;
    } else {
      LOG("Client_FTM_Feature_Chr: Found!");
      // Read the value of the characteristic.
      if(pRemote_FTM_Feature_Chr->canRead()) {
        // Read Feature Data
        client_FTM_Feature_Str = pRemote_FTM_Feature_Chr->readValue();
        // Transfer/Update the value to the server side
        if(server_FTM_Feature_Chr)
          server_FTM_Feature_Chr->setValue(client_FTM_Feature_Str); 
#ifdef DEBUG
        uint8_t* MyPtr = (uint8_t*)client_FTM_Feature_Str.c_str();  // Points to first 4 bytes of string
        std::string hexString = UTILS::getInstance()->toHexString(client_FTM_Feature_Str);
        LOG(" -> Client Reads Raw FTM Feature bytes: [8] [%s]", hexString.c_str());
        LOG("- Fitness Machine Features:");
        // Load 32-bit client_CP_Feature_Chr value
        uint32_t client_FTM_Feature_Flags_One;
        memcpy(&client_FTM_Feature_Flags_One, MyPtr, 4);  
        for (int i = 0; i < client_FTM_Feature_Str_One_Len; i++) {
          if ( client_FTM_Feature_Flags_One & (1 << i) ) {
            LOG(" %s", client_FTM_Feature_Str_One[i]);
          }
        }
        LOG("- Target Setting Features:");
        // Load 32-bit client_CP_Feature_Chr value
        uint32_t client_FTM_Feature_Flags_Two;
        memcpy(&client_FTM_Feature_Flags_Two, MyPtr+4, 4); // Points to second 4 bytes of string
        for (int i = 0; i < client_FTM_Feature_Str_Two_Len; i++) {
          if ( client_FTM_Feature_Flags_Two & (1 << i) ) {
            LOG(" %s", client_FTM_Feature_Str_Two[i]);
          }
        }
#endif
      }
    } 

    pRemote_FTM_IndoorBikeData_Chr = pRemote_FitnessMachine_Service->getCharacteristic(UUID16_CHR_INDOOR_BIKE_DATA);
    if (pRemote_FTM_IndoorBikeData_Chr == nullptr) {
      LOG("Mandatory Client_FTM_IndoorBikeData_Chr: Not Found!");
      return false; // Mandatory when service is present
    }
    LOG("Client_FTM_IndoorBikeData_Chr: Found!");  
    if(!pRemote_FTM_IndoorBikeData_Chr->canNotify()) {
      LOG("Mandatory Client_FTM_IndoorBikeData_Chr: Cannot Notify!");
      return false; // Mandatory when service is present
    }

    pRemote_FTM_Status_Chr = pRemote_FitnessMachine_Service->getCharacteristic( UUID16_CHR_FITNESS_MACHINE_STATUS);
    if (pRemote_FTM_Status_Chr == nullptr) {
      LOG("Mandatory Client_FTM_Status_Chr: Not Found!");
      return false; // Mandatory when service is present
    }
    LOG("Client_FTM_Status_Chr: Found!");  
    if(!pRemote_FTM_Status_Chr->canNotify()) {
      LOG("Mandatory Client_FTM_Status_Chr: Cannot Notify!");
      return false; // Mandatory when service is present
    }
#ifdef ENABLE_TRAININGSTATUS
    pRemote_FTM_TrainingStatus_Chr = pRemote_FitnessMachine_Service->getCharacteristic(UUID16_CHR_TRAINING_STATUS);
    if (pRemote_FTM_TrainingStatus_Chr == nullptr) {
      LOG("Client_FTM_TrainingStatus_Chr: Not Found!");
      // NOT Mandatory
    } else {  
      LOG("Client_FTM_TrainingStatus_Chr: Found!");  
      if(!pRemote_FTM_TrainingStatus_Chr->canNotify()) {
        LOG("Mandatory Client_FTM_TrainingStatus_Chr: Cannot Notify!");
        return false; // Mandatory when service is present
      }
    }
#endif
    pRemote_FTM_SupportedResistanceLevelRange_Chr = pRemote_FitnessMachine_Service->getCharacteristic(UUID16_CHR_SUPPORTED_RESISTANCE_LEVEL_RANGE);
    if (pRemote_FTM_SupportedResistanceLevelRange_Chr == nullptr) {
      LOG("Client_FTM_SupportedResistanceLevelRange_Chr: Not Found!");
    } else {
      LOG("Client_FTM_SupportedResistanceLevelRange_Chr: Found!");
      // Read the value of the characteristic.
      if(pRemote_FTM_SupportedResistanceLevelRange_Chr->canRead()) {
        // Read Supported Resistance Level Range Data
        client_FTM_SupportedResistanceLevelRange_Str = pRemote_FTM_SupportedResistanceLevelRange_Chr->readValue();
        // Transfer/Update the value to the server side
        if(server_FTM_SupportedResistanceLevelRange_Chr)
          server_FTM_SupportedResistanceLevelRange_Chr->setValue(client_FTM_SupportedResistanceLevelRange_Str); 
#ifdef DEBUG
        std::string hexString = UTILS::getInstance()->toHexString(client_FTM_SupportedResistanceLevelRange_Str);
        LOG(" -> Client Reads Raw FTM Supported Resistance Level Range bytes: [6] [%s]", hexString.c_str());
#endif
      }
    } 

    pRemote_FTM_SupportedPowerRange_Chr = pRemote_FitnessMachine_Service->getCharacteristic(UUID16_CHR_SUPPORTED_POWER_RANGE);
    if (pRemote_FTM_SupportedPowerRange_Chr == nullptr) {
      LOG("Client_FTM_SupportedPowerRange_Chr: Not Found!");
    } else {
      LOG("Client_FTM_SupportedPowerRange_Chr: Found!");
      // Read the value of the characteristic.
      if(pRemote_FTM_SupportedPowerRange_Chr->canRead()) {
        // Read Supported Power Range Data
        client_FTM_SupportedPowerRange_Str = pRemote_FTM_SupportedPowerRange_Chr->readValue();
        // Transfer/Update the value to the server side
        if(server_FTM_SupportedPowerRange_Chr)
          server_FTM_SupportedPowerRange_Chr->setValue(client_FTM_SupportedPowerRange_Str); 
#ifdef DEBUG
        std::string hexString = UTILS::getInstance()->toHexString(client_FTM_SupportedPowerRange_Str);
        LOG(" -> Client Reads Raw FTM Supported Power Range bytes: [6] [%s]", hexString.c_str());
#endif
      }
    }  

    pRemote_FTM_ControlPoint_Chr = pRemote_FitnessMachine_Service->getCharacteristic(UUID16_CHR_FITNESS_MACHINE_CONTROL_POINT);
    if (pRemote_FTM_ControlPoint_Chr == nullptr) {
      LOG("Mandatory Client_FTM_ControlPoint_Chr: Not Found!");
      return false; // Mandatory when service is present
    }
    LOG("Client_FTM_ControlPoint_Chr: Found!");
    if(!pRemote_FTM_ControlPoint_Chr->canIndicate()) {
      LOG("Mandatory Client_FTM_ControlPoint_Chr: Cannot Indicate!");
      return false; // Mandatory when service is present
    }

    if(!pRemote_FTM_ControlPoint_Chr->canWrite()) {
      LOG("Mandatory Client_FTM_ControlPoint_Chr: Cannot Write!");
      return false; // Mandatory when service is present
    }

    isClientCPWriteAcknowledged = true; // Default state after (re)connect

    // -----------------------------------------------------------------------------------------------------------------------------------------
    // Start a CRITICAL task pRemote_FTM_ControlPoint_Chr write-with-response -> Pin task to core 1 and high priority for best results!
    // Stack size after extensive testing -> 4096 (very stable!!)
    // Check first if xTaskClientWriteWithResponse does not exist and only if server_FTM_ControlPoint_Chr is defined ?
    if(xTaskClientWriteWithResponseHandle == NULL) { 
        xTaskCreatePinnedToCore(this->xTaskClientWriteWithResponse, "Write WR", 4096, (void *)this, 24, \
                                &this->xTaskClientWriteWithResponseHandle, xTaskCoreID0);
        LOG("Client FTM ControlPoint Write-With-Response Task created!");
    }
    // ----------------------------------------------------------------------------------------------------------------------------------------- 
return true;
}

#ifdef ENABLE_TRAININGSTATUS
void FTMS::server_FTM_TrainingStatus_Chr_Notify(uint8_t* pData, size_t length) {
  if(isServerFTMTrainingStatusNotifyEnabled) {
    server_FTM_TrainingStatus_Chr->setValue(pData, length);      
    server_FTM_TrainingStatus_Chr->notify();
  }
}
#endif

void FTMS::server_FTM_Status_Chr_Notify(uint8_t* pData, size_t length) {
  if(isServerFTMStatusNotifyEnabled) {
    server_FTM_Status_Chr->setValue(pData, length);
    server_FTM_Status_Chr->notify();
  }
}

void FTMS::server_FTM_IndoorBikeData_Chr_Notify(uint8_t* pData, size_t length) {
  if(isServerFTMIndoorBikeDataNotifyEnabled) {
    server_FTM_IndoorBikeData_Chr->setValue(pData, length);       
    server_FTM_IndoorBikeData_Chr->notify();
  }
}

void FTMS::server_Idle_State_Notify(void) {
    uint8_t buf[10];
#ifdef ENABLE_TRAININGSTATUS
    // Signal FTM Training Status --> Idle!
    memset(buf, 0, 10);
    buf[0] = 0x00;
    buf[1] = 0x01; 
    server_FTM_TrainingStatus_Chr_Notify(buf, 2);
    delay(5);
#endif
    // Signal FTM Status --> Stopped!
    memset(buf, 0, 10);
    buf[0] = 0x02;  // Machine status: Stopped or Paused
    buf[1] = 0x01;  // Reason code (e.g., 0x01 = stopped)
    server_FTM_Status_Chr_Notify(buf, 2);
    LOG(">>> Signal Server Idle State!");
}

void FTMS::client_FTMS_Subscribe(void) {
  if( pRemote_FTM_ControlPoint_Chr != nullptr)
    if( !pRemote_FTM_ControlPoint_Chr->subscribe(indications, Static_FTM_ControlPoint_Indicate_Callback) )
      LOG(">>> ERROR remote FTM Control Point Subscribe failed!");
#ifdef ENABLE_TRAININGSTATUS
  if( pRemote_FTM_TrainingStatus_Chr != nullptr)
    if( !pRemote_FTM_TrainingStatus_Chr->subscribe(notifications, Static_FTM_TrainingStatus_Notify_Callback) )
      LOG(">>> ERROR remote FTM Training Status Subscribe failed!");
#endif
  if( pRemote_FTM_IndoorBikeData_Chr != nullptr)
    if( !pRemote_FTM_IndoorBikeData_Chr->subscribe(notifications, Static_FTM_IndoorBikeData_Notify_Callback) )
      LOG(">>> ERROR remote FTM Indoor Bike Data Subscribe failed!");

  if( pRemote_FTM_Status_Chr != nullptr)
    if( !pRemote_FTM_Status_Chr->subscribe(notifications, Static_FTM_Status_Notify_Callback) ) 
      LOG(">>> ERROR remote FTM Machine Status Subscribe failed!");
} // subscribe

void FTMS::client_FTMS_Unsubscribe(void) {
  if( pRemote_FTM_ControlPoint_Chr != nullptr)
    if( !pRemote_FTM_ControlPoint_Chr->unsubscribe(false) )
      LOG(">>> ERROR remote FTM Control Point UNsubscribe failed!");

#ifdef ENABLE_TRAININGSTATUS
  if( pRemote_FTM_TrainingStatus_Chr != nullptr)
    if( !pRemote_FTM_TrainingStatus_Chr->unsubscribe(false) ) 
      LOG(">>> ERROR remote FTM Training Status UNsubscribe failed!");
#endif

  if( pRemote_FTM_IndoorBikeData_Chr != nullptr)
    if( !pRemote_FTM_IndoorBikeData_Chr->unsubscribe(false) )
      LOG(">>> ERROR remote FTM Indoor Bike Data UNubscribe failed!");

  if( pRemote_FTM_Status_Chr != nullptr)
    if( !pRemote_FTM_Status_Chr->unsubscribe(false) )
      LOG(">>> ERROR remote FTM Machine Status UNsubscribe failed!");
} // unsubscribe

