#ifndef FITNESSMACHINE_H
#define FITNESSMACHINE_H

#include <Arduino.h>
#include <string>

// Include all NimBLE constants
#include "config/configNimBLE.h"
#include <NimBLEDevice.h>

// Trainingstatus Characteristic is not mandatory with all cycling APP's -> Zwift simply ignores it!
// To minimize the size (of FTMS) and optimize MITM performance: switch it OFF by default ! 
// uncomment only to switch it ON when cycling APP demands it!!
//#define ENABLE_TRAININGSTATUS

// Control point consists of 1 opcode (byte) and maximum 18 bytes as parameters
// This ftmcp_data_t structure represents the control point data. The first octet represents the opcode of the request
// followed by a parameter array of maximum 18 octects
const uint8_t FTM_CONTROL_POINT_DATALEN = 19; // Control point consists of 1 opcode (byte) and maximum 18 bytes as parameters
// The Fitness Machine Control Point data type structure 
typedef struct __attribute__( ( packed ) )
{
  uint8_t OPCODE;
  uint8_t OCTETS[(FTM_CONTROL_POINT_DATALEN-1)];
} ftmcp_data_t;

typedef union // The union type automatically maps the bytes member array to the ftmcp_data_t structure member values
{
  ftmcp_data_t values;
  uint8_t bytes[FTM_CONTROL_POINT_DATALEN];
} ftmcp_data_ut;

/**
 * Fitness Machine Control Point opcodes 
 * 
 * LSO: uint8 Op Code
 * MSO: 0..18 octets Parameters
 */
const uint8_t ftmcpRequestControl = 0x00;
const uint8_t ftmcpReset = 0x01;
const uint8_t ftmcpSetTargetSpeed = 0x02;
const uint8_t ftmcpSetTargetInclination = 0x03;
const uint8_t ftmcpSetTargetResistanceLevel = 0x04;
const uint8_t ftmcpSetTargetPower = 0x05;
const uint8_t ftmcpSetTargetHeartRate = 0x06;
const uint8_t ftmcpStartOrResume = 0x07;
const uint8_t ftmcpStopOrPause = 0x08;
const uint8_t ftmcpSetTargetedExpendedEngery = 0x09;
const uint8_t ftmcpSetTargetedNumberOfSteps = 0x0A;
const uint8_t ftmcpSetTargetedNumberOfStrided = 0x0B;
const uint8_t ftmcpSetTargetedDistance = 0x0C;
const uint8_t ftmcpSetTargetedTrainingTime = 0x0D;
const uint8_t ftmcpSetTargetedTimeInTwoHeartRateZones = 0x0E;
const uint8_t ftmcpSetTargetedTimeInThreeHeartRateZones = 0x0F;
const uint8_t ftmcpSetTargetedTimeInFiveHeartRateZones = 0x10;
const uint8_t ftmcpSetIndoorBikeSimulationParameters = 0x11;
const uint8_t ftmcpSetWheelCircumference = 0x12;
const uint8_t ftmcpSetSpinDownControl = 0x13;
const uint8_t ftmcpSetTargetedCadence = 0x14;

class OPS;

class FTMS {
//NOT private: allow FTMSXS to access variables protected:
private:
FTMS(); 
static FTMS* instance;                                          // Singleton class instance
OPS* operations;
ftmcp_data_ut server_FTM_Control_Point_Data;                    // Fitness Machine Control Point Data member
// Server Fitness Machine Service -----------------------------------------------------------------
NimBLEService* server_FitnessMachine_Service = nullptr;         // FTM Service
NimBLECharacteristic* server_FTM_Feature_Chr = nullptr;         //  Fitness Machine Feature, mandatory, read
NimBLECharacteristic* server_FTM_Status_Chr = nullptr;          //  Fitness Machine Status, mandatory, notify
NimBLECharacteristic* server_FTM_IndoorBikeData_Chr = nullptr;  //  Indoor Bike Data, optional, notify
NimBLECharacteristic* server_FTM_ControlPoint_Chr = nullptr;    //  Fitness Machine Control Point, optional, write & indicate
#ifdef ENABLE_TRAININGSTATUS
NimBLECharacteristic* server_FTM_TrainingStatus_Chr = nullptr;  //  Training Status, optional, read & notify
#endif
NimBLECharacteristic* server_FTM_SupportedResistanceLevelRange_Chr = nullptr; // Supported Resistance Level, read, optional
NimBLECharacteristic* server_FTM_SupportedPowerRange_Chr = nullptr;   // Supported Power Levels, read, optional
// -----------------------------------------------------------------------------------------------

// Client Fitness Machine Service -----------------------------------------------------------------
NimBLERemoteService* pRemote_FitnessMachine_Service = nullptr;      // FTM Service
// Service characteristics exposed by FTM Service
NimBLERemoteCharacteristic* pRemote_FTM_SupportedResistanceLevelRange_Chr = nullptr; // Supported Resistance Level, read, optional
//MITM
std::string client_FTM_SupportedResistanceLevelRange_Str; 
NimBLERemoteCharacteristic* pRemote_FTM_SupportedPowerRange_Chr = nullptr;  // Supported Power Levels, read, optional
//MITM
std::string client_FTM_SupportedPowerRange_Str; 
NimBLERemoteCharacteristic* pRemote_FTM_Feature_Chr = nullptr;        //  Fitness Machine Feature, mandatory, read
//MITM
std::string client_FTM_Feature_Str; 
#ifdef ENABLE_TRAININGSTATUS
NimBLERemoteCharacteristic* pRemote_FTM_TrainingStatus_Chr = nullptr; //  Training Status, optional, read & notify
#endif
NimBLERemoteCharacteristic* pRemote_FTM_Status_Chr = nullptr;         //  Fitness Machine Status, mandatory, notify
NimBLERemoteCharacteristic* pRemote_FTM_IndoorBikeData_Chr = nullptr; //  Indoor Bike Data, optional, notify
NimBLERemoteCharacteristic* pRemote_FTM_ControlPoint_Chr = nullptr;   //  Fitness Machine Control Point, optional, write & indicate

// FTM_ControlPoint Write With Response xTask for transferring training App data to the trainer
void static xTaskClientWriteWithResponse(void* parameter);
TaskHandle_t xTaskClientWriteWithResponseHandle = NULL;
boolean xTaskClientWriteWithResponseDataReady = false;
std::string xTaskClientWriteWithResponseData; 

void client_FTM_IndoorBikeData_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
inline void static Static_FTM_IndoorBikeData_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) __attribute__((always_inline));
void client_FTM_ControlPoint_Indicate_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);

void static xTaskServerIndicateResponse(void* parameter);
TaskHandle_t xTaskServerIndicateResponseHandle = NULL;
uint8_t xTaskServerIndicateResponseData[20];  // Buffer for Indicate response data
size_t xTaskServerIndicateResponseDataLen = 0;

inline void static Static_FTM_ControlPoint_Indicate_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) __attribute__((always_inline));
void client_FTM_Status_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
inline void static Static_FTM_Status_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) __attribute__((always_inline));
#ifdef ENABLE_TRAININGSTATUS
void client_FTM_TrainingStatus_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);
inline void static Static_FTM_TrainingStatus_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify) __attribute__((always_inline));
#endif

public:
~FTMS();
static FTMS* getInstance();  // Singleton access method
void serverFTMStatusOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue);
boolean isServerFTMStatusNotifyEnabled = false;
void serverFTMIndoorBikeDataOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue);
boolean isServerFTMIndoorBikeDataNotifyEnabled = false;
#ifdef ENABLE_TRAININGSTATUS
void serverFTMTrainingStatusOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue);
boolean isServerFTMTrainingStatusNotifyEnabled = false;
void server_FTM_TrainingStatus_Chr_Notify(uint8_t* pData, size_t length);
#endif
void serverFTMControlPointOnWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo);
long cpWriteTime;
void serverFTMControlPointOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue);
boolean isServerFTMControlPointIndicateEnabled = false;
void server_setupFTMS(NimBLEServer* pServer);
void server_FTM_Status_Chr_Notify(uint8_t* pData, size_t length);
bool isClientCPWriteAcknowledged = true;  //  Default state at start
long icInterval;
void server_FTM_IndoorBikeData_Chr_Notify(uint8_t* pData, size_t length);
void server_Idle_State_Notify(void);
bool client_FitnessMachine_Connect(NimBLEClient* pClient);
void client_FTMS_Subscribe(void);
void client_FTMS_Unsubscribe(void);
}; // Class FTMS

#endif // FITNESSMACHINE_H