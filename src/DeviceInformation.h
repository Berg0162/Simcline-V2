#ifndef DEVICEINFORMATION_H
#define DEVICEINFORMATION_H

#include <Arduino.h>
#include <string>

// Include all NimBLE constants
#include "config/configNimBLE.h"
#include <NimBLEDevice.h>

class OPS;

class DIS {
private:
// constructor
DIS();
static DIS* instance; // Singleton class instance
OPS* operations;
// Server Service Device Information --------------------------------------------------
NimBLEService *server_DeviceInformation_Service;
NimBLECharacteristic *server_DIS_ModelNumber_Chr  = nullptr;      // Read
NimBLECharacteristic *server_DIS_SerialNumber_Chr  = nullptr;     // Read
NimBLECharacteristic *server_DIS_ManufacturerName_Chr = nullptr;  // Read
/*
//NimBLECharacteristic *server_DIS_Firmware_Chr;          // Read
//std::string client_DIS_Firmware_Str;
//NimBLECharacteristic *server_DIS_Hardware_Chr;          // Read
//std::string client_DIS_Hardware_Str;
//NimBLECharacteristic *server_DIS_Software_Chr;          // Read
//std::string client_DIS_Software_Str;
*/
// Client Device Information Service       --------------------------------------------------
NimBLERemoteService*  pRemote_DeviceInformation_Service; 
const NimBLERemoteCharacteristic* pRemote_DIS_ManufacturerName_Chr;   // Read
std::string client_DIS_Manufacturer_Str = "Espressif";
const NimBLERemoteCharacteristic* pRemote_DIS_ModelNumber_Chr;       // Read
std::string client_DIS_ModelNumber_Str = "ESP32";
const NimBLERemoteCharacteristic* pRemote_DIS_SerialNumber_Chr;      // Read
std::string client_DIS_SerialNumber_Str = "123";

public:
~DIS();
static DIS* getInstance();  // Singleton access method
void server_setupDIS(NimBLEServer* pServer);
bool client_DeviceInformation_Connect(NimBLEClient* pClient);
}; // Device Information Service 

#endif //DEVICEINFORMATION_H
