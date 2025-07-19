#ifndef NIMBLESERVER_H
#define NIMBLESERVER_H

#include <Arduino.h>
#include <string>

// Include all NimBLE constants
#include "config/configNimBLE.h"
#include <NimBLEDevice.h>

class OPS;

class ServerSide {
//NOT private: allow ServerXS to access variables
protected:
ServerSide();
static ServerSide* instance;                     // Singleton class instance
OPS* operations;
NimBLEAdvertising *pAdvertising = nullptr;
bool forcedDisconnect = false;

public:
virtual ~ServerSide();
static ServerSide* getInstance();                // Singleton access method
static NimBLEServer* pServer;                    // Pointer to NimBLEServer instance
virtual void serverConnectionCallbacksOnConnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo);    // Allow ServerXP to override
virtual void serverConnectionCallbacksOnDisconnect(NimBLEServer* pServer, NimBLEConnInfo& connInfo, int reason); // Allow ServerXP to override
void serverConnectionCallbacksOnMTUChange(uint16_t MTU, NimBLEConnInfo& connInfo);
void serverConnectionCallbacksonConnParamsUpdate(NimBLEConnInfo& connInfo);
void serverSecurityCallbacksOnAuthenticationComplete(NimBLEConnInfo& connInfo); 
void init(void);
void startAdvertising(void);
void setupAdvertising(void);
}; // Class ServerSide


#endif // NIMBLESERVER_H