#ifndef FITNESSEQUIPMENTCYCLING_H
#define FITNESSEQUIPMENTCYCLING_H

#include <Arduino.h>
#include <string>

// Include all NimBLE constants
#include "config/configNimBLE.h"
#include <NimBLEDevice.h>

class OPS;

class FEC {
private:
FEC();
static FEC* instance;                                       // Singleton class instance
OPS* operations;
// Server Fitness Equipment Cycling Service -----------------------------------------------------
NimBLEService* server_FitnessEquipmentCycling_Service = nullptr;  
NimBLECharacteristic* server_FEC_Rxd_Chr = nullptr;         // Notify, Read
NimBLECharacteristic* server_FEC_Txd_Chr = nullptr;         // Write No Response

// Client Fitness Equipment Cycling Service -----------------------------------------------------
NimBLERemoteService* pRemote_FitnessEquipmentCycling_Service = nullptr;
NimBLERemoteCharacteristic* pRemote_FEC_Rxd_Chr = nullptr;  // Notify, Read
NimBLERemoteCharacteristic* pRemote_FEC_Txd_Chr = nullptr;  // Write No Response

void client_FEC_Rxd_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                                  uint8_t* pData, size_t length, bool isNotify);
inline void static Static_FEC_Rxd_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                    uint8_t* pData, size_t length, bool isNotify) __attribute__((always_inline));
public:
~FEC();
static FEC* getInstance();  // Singleton access method
void serverFECRxdOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue);
void serverFECTxdOnWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo& connInfo);
boolean isServerFECRxdNotifyEnabled = false;
void server_FEC_Rxd_Chr_Notify(uint8_t* pData, size_t length);

void server_setupFEC(NimBLEServer* pServer);
bool client_FitnessEquipmentCycling_Connect(NimBLEClient* pClient);
void client_FEC_Subscribe(void);
void client_FEC_Unsubscribe(void);
}; // class FEC

#endif // FITNESSEQUIPMENTCYCLING_H