#ifndef CYCLINGSPEEDCADENCE_H
#define CYCLINGSPEEDCADENCE_H

#include <Arduino.h>
#include <string>

// Include all NimBLE constants
#include "config/configNimBLE.h"
#include <NimBLEDevice.h>

/*       CSC feature flags */
#define     CSC_FEATURE_WHEEL_REV_DATA              0x01
#define     CSC_FEATURE_CRANK_REV_DATA              0x02
#define     CSC_FEATURE_MULTIPLE_SENSOR_LOC         0x04 

class OPS;

class CSC {
private:
CSC();
static CSC* instance;                         // Singleton class instance
OPS* operations;
// Server Cycling Speed and Cadence Service -----------------------------------------------------
NimBLEService* server_CyclingSpeedCadence_Service;  
NimBLECharacteristic* server_CSC_Measurement_Chr;               //     Notify, Read
NimBLECharacteristic* server_CSC_Feature_Chr = nullptr;         //     Read
NimBLECharacteristic* server_CSC_Location_Chr  = nullptr;       //     Read

// Client Cycling Speed and Cadence Service -----------------------------------------------------
NimBLERemoteService*        pRemote_CyclingSpeedCadence_Service;
const NimBLERemoteCharacteristic* pRemote_CSC_Measurement_Chr;         // Notify, Read
const NimBLERemoteCharacteristic* pRemote_CSC_Feature_Chr;             // Read
uint16_t client_CSC_Feature_Flags = (CSC_FEATURE_WHEEL_REV_DATA | CSC_FEATURE_CRANK_REV_DATA | CSC_FEATURE_MULTIPLE_SENSOR_LOC);
const NimBLERemoteCharacteristic* pRemote_CSC_Location_Chr;            // Read
uint8_t client_CSC_Location_Value = {0x0C};                      // --> rear wheel !

void client_CSC_Measurement_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                                    uint8_t* pData, size_t length, bool isNotify);
inline void static Static_CSC_Measurement_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                        uint8_t* pData, size_t length, bool isNotify) __attribute__((always_inline));

public:
~CSC();
static CSC* getInstance();  // Singleton access method
void serverCSCMeasurementOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue);
boolean isServerCSCMeasurementNotifyEnabled = false;
void server_CSC_Measurement_Chr_Notify(uint8_t* pData, size_t length);

void server_setupCSC(NimBLEServer* pServer);
bool client_CyclingSpeedCadence_Connect(NimBLEClient* pClient);
void client_CSC_Subscribe(void);
void client_CSC_Unsubscribe(void);
}; // class CSC

#endif // CYCLINGSPEEDCADENCE_H