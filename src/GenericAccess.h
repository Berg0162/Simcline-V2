#ifndef GENERICACCESS_H
#define GENERICACCESS_H

#include <Arduino.h>
#include <string>

// Include all NimBLE constants
#include "config/configNimBLE.h"
#include <NimBLEDevice.h>

// Generic Access Service
class GAS {
private:
// constructor
GAS();
static GAS* instance; // Singleton class instance
NimBLERemoteService* pRemote_GenericAccess_Service = nullptr;
NimBLERemoteCharacteristic* pRemote_GA_Appearance_Chr = nullptr; // Read
NimBLERemoteCharacteristic* pRemote_GA_DeviceName_Chr = nullptr; // Read, Write
std::string client_GA_DeviceName_Str;

public:
~GAS();
static GAS* getInstance();  // Singleton access method
void server_setupGAS(NimBLEServer* pServer);
bool client_GenericAccess_Connect(NimBLEClient* pClient);
static uint16_t client_GA_Appearance_Value;
}; // Generic Access

#endif