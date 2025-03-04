#ifndef CLIENTSIDE_H
#define CLIENTSIDE_H

#include <Arduino.h>
#include <string>

// Include all NimBLE constants
#include "config/configNimBLE.h"
#include <NimBLEDevice.h>

class OPS;

class ClientSide {
private:
ClientSide();
static ClientSide* instance;
OPS* operations; // Private pointer to store the singleton instance
const NimBLEAdvertisedDevice* trainerDevice = nullptr;
bool hasConnectPassed = false;
NimBLEScan* pNimBLEScan = nullptr;
// xTask to connect Client with peripheral and check all services + characteristics
static void xTaskClientConnectServer(void *parameter);
TaskHandle_t xTaskClientConnectServerHandle = NULL;
// NimBLE Scanning has NO setting for auto restart scanning after Client Disconnects
static void xTaskClientStartScanning(void *parameter);
TaskHandle_t xTaskClientStartScanningHandle = NULL;
// xTask to subscribe/unsubscribe to all relevant client Characteritics after connect/disconnect
static void xTaskClientSubscribeAll(void *parameter);
static void xTaskClientUnSubscribeAll(void *parameter);
TaskHandle_t xTaskClientSubscribeUnsubscribeHandle = NULL;
public:
~ClientSide();
static ClientSide* getInstance();
static NimBLEClient* pClient;                    // Pointer to NimBLEClient instance
void clientConnectionCallbacksOnConnect(NimBLEClient* pClient);
void clientConnectionCallbacksOnDisconnect(NimBLEClient* pClient, int reason);
bool clientConnectServer(void);
void clientScanCallbacksOnResult(const NimBLEAdvertisedDevice* advertisedDevice);
void startScanning(void);
void clientSubscribeToAll(bool isEnable);
bool hasSubscribedToAll = false;
}; // ClientSide class

#endif // CLIENTSIDE_H