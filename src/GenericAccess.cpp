#include "GenericAccess.h"

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress DEBUG messages that help debugging...
// Uncomment general "#define DEBUG" to activate debug messages for this class
#define DEBUG
// Include these debug utility macros in all cases!
#include "config/configDebug.h"

#ifdef DEBUG
//#define DEBUGSETUP
#endif

// Client Generic Access --------------------------------------------------------------------------
#define UUID16_SVC_GENERIC_ACCESS                             NimBLEUUID((uint16_t)0x1800)
#define UUID16_CHR_DEVICE_NAME                                NimBLEUUID((uint16_t)0x2A00)
#define UUID16_CHR_APPEARANCE                                 NimBLEUUID((uint16_t)0x2A01)
#define UUID16_CHR_PERIPHERAL_PREFERRED_CONNECTION_PARAMETERS NimBLEUUID((uint16_t)0x2A04)
#define UUID16_CHR_CENTRAL_ADDRESS_RESOLUTION                 NimBLEUUID((uint16_t)0x2AA6)

// We need this for setting the Server-side Generic Access Char's --> Appearance and DeviceName
#include <nimble/nimble/host/services/gap/include/services/gap/ble_svc_gap.h>

// Initialize the static members
GAS* GAS::instance = nullptr;
uint16_t GAS::client_GA_Appearance_Value = BLE_APPEARANCE_GENERIC_CYCLING; // Default decimal: 1152 -> Generic Cycling

// ------------------------------------------------------------------------------------------------
// Constructor
GAS::GAS() { 
    client_GA_DeviceName_Str = THISDEVICENAME;
}

// Destructor
GAS::~GAS() {
    client_GA_DeviceName_Str = "";
 }

GAS* GAS::getInstance() {
    if (instance == nullptr) {
        instance = new GAS();
    }
    return instance;
}

void GAS::server_setupGAS(NimBLEServer* pServer) {
  // Set the Generic Access Appearance value from default: [0] --> Unknown to [1152] --> Generic Cycling
    int RespErr = ble_svc_gap_device_appearance_set(client_GA_Appearance_Value);
#ifdef DEBUGSETUP
    if(RespErr == 0) {
      LOG("Successfully Set Generic Access Appearance Chr value to:  [%d] Generic Cycling", client_GA_Appearance_Value); 
    } else {
      LOG("Unable to Set Generic Access Appearance Chr value!");      
    } 
#endif
  // Set Generic Access Device Name Chr to a value
    RespErr = ble_svc_gap_device_name_set((const char*)client_GA_DeviceName_Str.c_str());
#ifdef DEBUGSETUP
    if(RespErr == 0) {
      LOG("Successfully Set Generic Access Device Name Chr value to: [%s]", client_GA_DeviceName_Str.c_str()); 
    } else {
      LOG("Unable to Set Generic Access Device Name Chr value!");      
    } 
#endif
};

bool GAS::client_GenericAccess_Connect(NimBLEClient* pClient) {
    // If Generic Access is not found then go on.... NOT FATAL !
    pRemote_GenericAccess_Service = pClient->getService(UUID16_SVC_GENERIC_ACCESS);    
    if ( pRemote_GenericAccess_Service == nullptr ) {
      LOG("Client Generic Access: NOT Found!");
      return true;
    }
    LOG("Client Generic Access: Found!");
    pRemote_GA_DeviceName_Chr = pRemote_GenericAccess_Service->getCharacteristic(UUID16_CHR_DEVICE_NAME);  
      if ( pRemote_GA_DeviceName_Chr != nullptr ) {
          if(pRemote_GA_DeviceName_Chr->canRead()) {
            client_GA_DeviceName_Str = pRemote_GA_DeviceName_Chr->readValue();
            int RespErr = ble_svc_gap_device_name_set((const char*)client_GA_DeviceName_Str.c_str()); // Transfer/Update the value to the server side    
            LOG(" -> Client Reads Device Name:   [%s]", client_GA_DeviceName_Str.c_str());
          }            
      }     
      pRemote_GA_Appearance_Chr = pRemote_GenericAccess_Service->getCharacteristic(UUID16_CHR_APPEARANCE);       
      if ( pRemote_GA_Appearance_Chr != nullptr ) {
          if(pRemote_GA_Appearance_Chr->canRead()) {
            client_GA_Appearance_Value = pRemote_GA_Appearance_Chr->readValue<uint16_t>();
            int RespErr = ble_svc_gap_device_appearance_set(client_GA_Appearance_Value); // Transfer/Update the value to the server side
            LOG(" -> Client Reads Appearance:    [%d]", client_GA_Appearance_Value);
          }
      }     
    return true;     
};

