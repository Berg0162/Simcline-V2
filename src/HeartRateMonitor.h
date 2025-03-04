#ifndef HEARTRATEMONITOR_H
#define HEARTRATEMONITOR_H

#include <Arduino.h>
#include <string>

// Include all NimBLE constants
#include "config/configNimBLE.h"
#include <NimBLEDevice.h>

class OPS;

class HRM {
private:
HRM();
static HRM* instance;                          // Singleton class instance
OPS* operations;
// Server HRM Service -----------------------------------------------------------------------------
NimBLEService *server_HeartRate_Service;
NimBLECharacteristic *server_HR_Measurement_Chr;           // Notify Write
NimBLECharacteristic *server_HR_Location_Chr = nullptr;    // Read

// Client HRM Service -----------------------------------------------------------------------------
NimBLERemoteService* pRemote_HeartRate_Service;
const NimBLERemoteCharacteristic* pRemote_HR_Measurement_Chr;
const NimBLERemoteCharacteristic* pRemote_HR_Location_Chr;
uint8_t client_HR_Location_Value  = { 0x01 };     // Chest

void client_HR_Measurement_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                                        uint8_t* pData, size_t length, bool isNotify);
inline static void Static_client_HR_Measurement_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                          uint8_t* pData, size_t length, bool isNotify) __attribute__((always_inline));

public:
~HRM();
static HRM* getInstance();  // Singleton access method
bool client_HeartRate_Connect(NimBLEClient* pClient);
void server_setupHRM(NimBLEServer* pServer);
void serverHRMMeasurementOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue);
boolean isServerHRMMeasurementNotifyEnabled = false;
void client_HR_Subscribe(void);
void client_HR_Unsubscribe(void);
void server_HR_Measurement_Chr_Notify(uint8_t* pData, size_t length);
}; // class HRM

#endif // HEARTRATEMONITOR_H