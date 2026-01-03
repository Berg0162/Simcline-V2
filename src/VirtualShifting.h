#ifndef VIRTUALSHIFTING_H
#define VIRTUALSHIFTING_H

#include <Arduino.h>
#include <string>

// Include all NimBLE constants
#include "config/configNimBLE.h"

#include <NimBLEDevice.h>
#include <map>

class OPS;
class UTILS;

class ZVS {
  private:
  ZVS();
  static ZVS* instance;
  OPS* operations;
  UTILS* utils;

  // Server Virtual Shifting Service
  NimBLEService *server_VS_Service;
  BLECharacteristic *server_VS_ASYNC_Chr;   // NOTIFY
  BLECharacteristic *server_VS_SYNCRX_Chr;  // WRITE No-Response
  BLECharacteristic *server_VS_SYNCTX_Chr;  // INDICATE

  // Remote Virtual Shifting Service
  NimBLERemoteService *pRemote_VirtualShifting_Service = nullptr;
  NimBLERemoteCharacteristic *pRemote_VS_ASYNC_Chr = nullptr;   // NOTIFY
  NimBLERemoteCharacteristic *pRemote_VS_SYNCRX_Chr = nullptr;  // WRITE No-Response
  NimBLERemoteCharacteristic *pRemote_VS_SYNCTX_Chr = nullptr;  // INDICATE

  struct TrainerData {
    int64_t power = 0;
    int64_t cadence = 0;
    int64_t unknown1 = 0;
    int64_t unknown2 = 0;
    int64_t unknown3 = 0;
    int64_t unknown4 = 0;
    bool valid = false;   // optional: indicates header was OK and decode succeeded
  };
  TrainerData getTrainerDataValues(std::vector<uint8_t>* trainerData);
  void processTrainerData(std::vector<uint8_t>* trainerData);

  void decodeLegacyZVSRequest(const std::vector<uint8_t>& data);
  void decodeNewZVSRequest(const std::vector<uint8_t>& data);
  
  bool write_pRemote_VS_SYNCRX_Chr(std::string data);
  void client_VS_ASYNC_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, \
																						                                      size_t length, bool isNotify);
  void static Static_VS_ASYNC_Notify_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, \
                                                                  uint8_t* pData, size_t length, bool isNotify);
  void client_VS_SYNCTX_Indicate_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, \
																						                                      size_t length, bool isNotify);
  void static Static_VS_SYNCTX_Indicate_Callback(NimBLERemoteCharacteristic* pBLERemoteCharacteristic, uint8_t* pData, \
																						                                      size_t length, bool isNotify);					

  public:
  ~ZVS();
  static ZVS* getInstance(); // Singleton access method

  void server_setupZVS(NimBLEServer* pServer);
  void serverVSSYNCTXOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue);
  bool isServerVSsynctxIndicateEnabled = false;
  void serverVSSYNCRXOnWrite(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo);
  void serverVSASYNCOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue);
  bool isServerVSasyncNotifyEnabled = false;

  bool client_VirtualShifting_Connect(NimBLEClient* pClient);
  void client_ZVS_Subscribe(void);
  void client_ZVS_Unsubscribe(void); 

};

#endif // VIRTUALSHIFTING_H