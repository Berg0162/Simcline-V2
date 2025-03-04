#include "DeviceInformation.h"

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress DEBUG messages that help debugging...
// Uncomment general "#define DEBUG" to activate debug messages for this class
#define DEBUG
// Include these debug utility macros in all cases!
#include "config/configDebug.h"

// Include operation control settings class
#include "Operations.h"

// Device Information Service ----------------------------------------------------------
#define UUID16_SVC_DEVICE_INFORMATION                         NimBLEUUID((uint16_t)0x180A)
#define UUID16_CHR_MODEL_NUMBER_STRING                        NimBLEUUID((uint16_t)0x2A24)
#define UUID16_CHR_SERIAL_NUMBER_STRING                       NimBLEUUID((uint16_t)0x2A25)
#define UUID16_CHR_MANUFACTURER_NAME_STRING                   NimBLEUUID((uint16_t)0x2A29)
//#define UUID16_CHR_FIRMWARE_REVISION_STRING                   NimBLEUUID((uint16_t)0x2A26)
//#define UUID16_CHR_HARDWARE_REVISION_STRING                   NimBLEUUID((uint16_t)0x2A27)
//#define UUID16_CHR_SOFTWARE_REVISION_STRING                   NimBLEUUID((uint16_t)0x2A28)

// Initialize the static member
DIS* DIS::instance = nullptr;

// ------------------------------------------------------------------------------------------------
// Constructor
DIS::DIS() { 
  operations = OPS::getInstance();
} 

// Destructor
DIS::~DIS() { }

DIS* DIS::getInstance() {
    if (instance == nullptr) {
        instance = new DIS();
    }
    return instance;
}

void DIS::server_setupDIS(NimBLEServer* pServer) {
    server_DeviceInformation_Service = pServer->createService(UUID16_SVC_DEVICE_INFORMATION);
    server_DIS_ModelNumber_Chr = server_DeviceInformation_Service->createCharacteristic(UUID16_CHR_MODEL_NUMBER_STRING, 
                                                                            NIMBLE_PROPERTY::READ);
    server_DIS_ModelNumber_Chr->setValue(client_DIS_ModelNumber_Str); // Set default
    server_DIS_SerialNumber_Chr = server_DeviceInformation_Service->createCharacteristic(UUID16_CHR_SERIAL_NUMBER_STRING, 
                                                                            NIMBLE_PROPERTY::READ);
    server_DIS_SerialNumber_Chr->setValue(client_DIS_SerialNumber_Str); // Set default
    server_DIS_ManufacturerName_Chr = server_DeviceInformation_Service->createCharacteristic(UUID16_CHR_MANUFACTURER_NAME_STRING, 
                                                                            NIMBLE_PROPERTY::READ);                                                                                        
    server_DIS_ManufacturerName_Chr->setValue(client_DIS_Manufacturer_Str); // Set default
/*
    server_DIS_Firmware_Chr = server_DeviceInformation_Service->createCharacteristic(UUID16_CHR_FIRMWARE_REVISION_STRING, 
                                                                            NIMBLE_PROPERTY::READ);
    server_DIS_Hardware_Chr = server_DeviceInformation_Service->createCharacteristic(UUID16_CHR_HARDWARE_REVISION_STRING, 
                                                                            NIMBLE_PROPERTY::READ);
    server_DIS_Software_Chr = server_DeviceInformation_Service->createCharacteristic(UUID16_CHR_SOFTWARE_REVISION_STRING, 
                                                                            NIMBLE_PROPERTY::READ);
    server_DIS_Hardware_Chr->setValue(client_DIS_Hardware_Str);
    server_DIS_Firmware_Chr->setValue(client_DIS_Firmware_Str);
    server_DIS_Software_Chr->setValue(client_DIS_Software_Str);
*/
    server_DeviceInformation_Service->start();
};

bool DIS::client_DeviceInformation_Connect(NimBLEClient* pClient) {
    // If Device Information is not found then go on.... NOT FATAL !
    pRemote_DeviceInformation_Service = pClient->getService(UUID16_SVC_DEVICE_INFORMATION);    
    if ( pRemote_DeviceInformation_Service == nullptr ) {
      LOG("Device Information Service: NOT Found!");
      return true;
    }
      LOG("Client Device Information Service: Found!");
      pRemote_DIS_ManufacturerName_Chr = pRemote_DeviceInformation_Service->getCharacteristic(UUID16_CHR_MANUFACTURER_NAME_STRING);  
      if ( pRemote_DIS_ManufacturerName_Chr != nullptr ) {
          if(pRemote_DIS_ManufacturerName_Chr->canRead()) {
            client_DIS_Manufacturer_Str = pRemote_DIS_ManufacturerName_Chr->readValue();
            if(server_DIS_ManufacturerName_Chr)
              server_DIS_ManufacturerName_Chr->setValue(client_DIS_Manufacturer_Str); // Transfer/Update the value to the server side
            LOG(" -> Client Reads Manufacturer Name: [%s]", client_DIS_Manufacturer_Str.c_str());
          }            
      }     
      pRemote_DIS_ModelNumber_Chr = pRemote_DeviceInformation_Service->getCharacteristic(UUID16_CHR_MODEL_NUMBER_STRING);       
      if ( pRemote_DIS_ModelNumber_Chr != nullptr ) {
          if(pRemote_DIS_ModelNumber_Chr->canRead()) {
            client_DIS_ModelNumber_Str = pRemote_DIS_ModelNumber_Chr->readValue();
            if(server_DIS_ModelNumber_Chr)
              server_DIS_ModelNumber_Chr->setValue(client_DIS_ModelNumber_Str); // Transfer/Update the value to the server side
            LOG(" -> Client Reads Model Number:      [%s]", client_DIS_ModelNumber_Str.c_str());
          }
      }  
      pRemote_DIS_SerialNumber_Chr = pRemote_DeviceInformation_Service->getCharacteristic(UUID16_CHR_SERIAL_NUMBER_STRING);       
      if ( pRemote_DIS_SerialNumber_Chr != nullptr ) {
          if(pRemote_DIS_SerialNumber_Chr->canRead()) {
            client_DIS_SerialNumber_Str = pRemote_DIS_SerialNumber_Chr->readValue();
            if(server_DIS_SerialNumber_Chr)
              server_DIS_SerialNumber_Chr->setValue(client_DIS_SerialNumber_Str); // Transfer/Update the value to the server side
            LOG(" -> Client Reads Serial Number:     [%s]", client_DIS_SerialNumber_Str.c_str());
          }
      }       
  return true;    
};

