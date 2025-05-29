#ifndef NORDICUART_H
#define NORDICUART_H

#include <Arduino.h>
#include <string>

// Include all NimBLE constants
#include "config/configNimBLE.h"
#include <NimBLEDevice.h>

class OPS;

class NUS {
private:
NUS();
static NUS* instance;                          // Singleton class instance
OPS* operations;
// Server NORDIC UART SERVICE a.k.a. NUS -----------------------------------------------
NimBLEService* server_NordicUart_Service = nullptr; 
NimBLECharacteristic* server_NUS_Rxd_Chr = nullptr;  // Write No Response (Receiving Data)
NimBLECharacteristic* server_NUS_Txd_Chr = nullptr;  // Read Notify (Sending Data)

public:
~NUS();
static NUS* getInstance();  // Singleton access method
void serverNUSRxdChrCallbacksOnWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo);
void serverNUSTxdCallbacksOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue);
void server_NUS_Txd_Persistent_Settings(void);
void server_setupNUS(NimBLEServer* pServer);
};

#endif // NORDICUART_H