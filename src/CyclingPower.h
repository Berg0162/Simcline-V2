#ifndef CYCLINGPOWER_H
#define CYCLINGPOWER_H

#include <Arduino.h>
#include <string>

// Include all NimBLE constants
#include "config/configNimBLE.h"
#include <NimBLEDevice.h>

/**
 * Wahoo Trainer Proprietary Control Point opcodes in decimal 
 * 
 * LSO: uint8 Op Code
 * MSO: 0..18 octets Parameters
 */
const uint8_t unlock                     = 32;
const uint8_t setResistanceMode          = 64;
const uint8_t setStandardMode            = 65;
const uint8_t setErgMode                 = 66;
const uint8_t setSimMode                 = 67;
const uint8_t setSimCRR                  = 68;
const uint8_t setSimWindResistance       = 69;
const uint8_t setSimGrade                = 70;
const uint8_t setSimWindSpeed            = 71;
const uint8_t setWheelCircumference      = 72;
const uint8_t unlockCommand[3] = {0x20, 0xEE, 0xFC}; // Wahoo Unlock command

/** 
 * The Wahoo Proprietary Control Point data type structure 
 * 
 */
const uint8_t WAHOO_CONTROL_POINT_DATALEN = 19; // Control point consists of 1 opcode (byte) and maximum 18 bytes as parameters
// This wahoocp_data_t structure represents the control point data. The first octet represents the opcode of the request
// followed by a parameter array of maximum 18 octects
typedef struct __attribute__( ( packed ) )
{
  uint8_t OPCODE;
  uint8_t OCTETS[(WAHOO_CONTROL_POINT_DATALEN-1)];
} wahoocp_data_t;

typedef union // The union type automatically maps the bytes member array to the wahoocp_data_t structure member values
{
  wahoocp_data_t values;
  uint8_t bytes[WAHOO_CONTROL_POINT_DATALEN];
} wahoocp_data_ut;

class OPS;

// Cycling Power Service
class CPS {
//NOT private: allow derived class to access variables --> protected:
private:
  CPS();
  static CPS* instance;  // Singleton class instance
  OPS* operations;
  wahoocp_data_ut server_Wahoo_Control_Point_Data;  // Wahoo Control Point Data variable

  // Client Cycling Power Service --------------------------------------------------------
  NimBLERemoteService* pRemote_CyclingPower_Service = nullptr;
  NimBLERemoteCharacteristic* pRemote_CP_Measurement_Chr = nullptr;           // Notify, Read
  NimBLERemoteCharacteristic* pRemote_CP_Feature_Chr = nullptr;               // Read
  uint32_t client_CP_Feature_Flags = { 0b00000000000000010000011010001011 };  // Relevant Cycling Power features
  NimBLERemoteCharacteristic* pRemote_CP_Location_Chr = nullptr;              // Read
  uint8_t client_CP_Location_Value = { 0x0C };                                // rear wheel !

  // Server Cycling Power Service ---------------------------------------------------------------
  NimBLEService* server_CyclingPower_Service = nullptr; 
  NimBLECharacteristic* server_CP_Measurement_Chr = nullptr;   // Notify, Read
  NimBLECharacteristic* server_CP_Feature_Chr = nullptr;       // Read
  NimBLECharacteristic* server_CP_Location_Chr = nullptr;      // Read
  NimBLECharacteristic* server_CP_ControlPoint_Chr = nullptr;  // Indicate, Write -> Optional

  void client_CP_Measurement_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                                        uint8_t* pData, size_t length, bool isNotify);
  inline void static Static_CP_Measurement_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                            uint8_t* pData, size_t length, bool isNotify) __attribute__((always_inline));

  void client_CP_ControlPoint_Indicate_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                                            uint8_t* pData, size_t length, bool isNotify);
  inline void static Static_CP_ControlPoint_Indicate_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                              uint8_t* pData, size_t length, bool isNotify) __attribute__((always_inline));

  // ControlPoint Write With Response xTask for transferring training App data to the trainer
  void static xTaskClientWriteWithResponse(void *parameter);
  std::string xTaskClientWriteWithResponseData; 
  TaskHandle_t xTaskClientWriteWithResponseHandle = NULL;
  boolean xTaskClientWriteWithResponseDataReady = false;

  void static xTaskServerIndicateResponse(void* parameter);
  TaskHandle_t xTaskServerIndicateResponseHandle = NULL;
  uint8_t xTaskServerIndicateResponseData[20];  // Buffer for Indicate response data
  size_t xTaskServerIndicateResponseDataLen = 0;

public:
  ~CPS();
  static CPS* getInstance();  // Singleton access method
  NimBLERemoteCharacteristic* pRemote_CP_ControlPoint_Chr = nullptr;  // Indicate, Write -> Optional
  void serverCPControlPointOnWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo); 
  void serverCPControlPointOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue);
  boolean isServerCPControlPointIndicateEnabled = false;
  
  void serverCPMeasurementOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue);
  boolean isServerCPMeasurementNotifyEnabled = false;
  void server_CP_Measurement_Chr_Notify(uint8_t* pData, size_t length);

  void server_setupCPS(NimBLEServer* pServer);
  bool client_CyclingPower_Connect(NimBLEClient* pClient); 
  void client_CP_Subscribe(void); 
  void client_CP_Unsubscribe(void);
  long cpWriteTime;
  long icInterval;
};  // CPS -> Cycling Power Service

#endif  // CYCLINGPOWER_H