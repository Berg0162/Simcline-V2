#include "NordicUart.h"

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress DEBUG messages that help debugging...
// Uncomment general "#define DEBUG" to activate debug messages for this class
#define DEBUG
// Include these debug utility macros in all cases!
#include "config/configDebug.h"
// ------------------------------------------------------------------------------------------------

#ifdef DEBUG
//  Restrict activating one or more of the following << EXTRA >> debug directives --> process intensive 
//  The overhead can lead to spurious side effects and a loss of quality of service handling!!
//#define DEBUG_NUS_SHOW_DATA_RAW // If defined allows for showing Raw Data transferred
#endif

// Include operations control settings class
#include "Operations.h"

/* NORDIC UART SERVICE a.k.a. NUS
 * NUS Service: 6E400001-B5A3-F393-E0A9-E50E24DCCA9E
 * NUS RXD    : 6E400002-B5A3-F393-E0A9-E50E24DCCA9E
 * NUS TXD    : 6E400003-B5A3-F393-E0A9-E50E24DCCA9E
 */
#define UUID_NUS_SERVICE NimBLEUUID("6E400001-B5A3-F393-E0A9-E50E24DCCA9E")
#define UUID_NUS_CHR_RXD NimBLEUUID("6E400002-B5A3-F393-E0A9-E50E24DCCA9E")
#define UUID_NUS_CHR_TXD NimBLEUUID("6E400003-B5A3-F393-E0A9-E50E24DCCA9E")

class server_NUS_Rxd_Chr_callbacks: public NimBLECharacteristicCallbacks {
private:
    NUS* nusInstance;  // LOCAL Pointer to the NUS class instance
public:
    // Constructor to accept a pointer to the CPS class instance
    server_NUS_Rxd_Chr_callbacks(NUS* instance) : nusInstance(instance) {}
protected:
    void onWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo& connInfo) override {
      nusInstance->serverNUSRxdChrCallbacksOnWrite(pCharacteristic, connInfo);
    }
}; 

// Handler class for Server NUS Txd Characteristic actions limited to onSubscribe
class server_NUS_Txd_Callbacks: public NimBLECharacteristicCallbacks {
private:
    NUS* nusInstance;  // LOCAL Pointer to the NUS class instance
public:
    // Constructor to accept a pointer to the CPS class instance
    server_NUS_Txd_Callbacks(NUS* instance) : nusInstance(instance) {}
protected:
    void onSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) override {
      nusInstance->serverNUSTxdCallbacksOnSubscribe(pCharacteristic, connInfo, subValue);
    }
};

// Initialize the static member
NUS* NUS::instance = nullptr;

// Constructor
NUS::NUS() { 
  operations = OPS::getInstance();
}

// Destructor
NUS::~NUS() { }

NUS* NUS::getInstance() {
    if (instance == nullptr) {
        instance = new NUS();
    }
    return instance;
}

void NUS::serverNUSTxdCallbacksOnSubscribe(NimBLECharacteristic* pCharacteristic, NimBLEConnInfo& connInfo, uint16_t subValue) {
    std::string str = std::string(pCharacteristic->getUUID());
    if(subValue == 0) LOG("Central Unsubscribed: [%s] NUS Txd", str.c_str());
    else LOG("Central Subscribed: [%s] NUS Txd", str.c_str());
    if(subValue == 1) { // Send the Control Settings to the phone
      server_NUS_Txd_Persistent_Settings();
    }
};

void NUS::serverNUSRxdChrCallbacksOnWrite(NimBLECharacteristic *pCharacteristic, NimBLEConnInfo& connInfo) {
    // Read data received over NUS Rxd from Mobile Phone
    std::string NusRxdData = server_NUS_Rxd_Chr->getValue();
    uint8_t NusRxdDataLen = NusRxdData.length();  // Get the actual length of data bytes
#ifdef DEBUG_NUS_SHOW_DATA_RAW
    // Display the raw packet data in actual length
    LOG(" -> Server Rec'd NUS Rxd Data [%d][%s]", NusRxdDataLen, NusRxdData.c_str());
#endif
    // The following routines parse and process the incoming commands
    // Every NusRxdData packet starts with a '!' otherwise corrupt/invalid
    if (NusRxdData[0] != '!') {
      LOG("-> Error: RXD-packet does not start with a '!'");
      return; // invalid NusRxdData packet: do not further parse and process
    }
  // RXpacket buffer has IdCode = "S"
  if (NusRxdData[1] == 'S') { // Settings packet
    uint8_t iMax=0, iMin=0, iPerc=0, iDispl=0;
    sscanf((char*)NusRxdData.c_str(), "!S%d;%d;%d;%d;", &iMax, &iMin, &iPerc, &iDispl);
    PhonePrefs phonePrefs(iMax, iMin, iPerc, iDispl); // Move into PhonePrefs Tuple
    operations->storePhonePrefs(phonePrefs); // Store in NVS
#ifdef DEBUG_NUS_SHOW_DATA_RAW
    LOG(" -> Server Sends NUS TXD Confirm Message: Done!");
#endif
    // Confirm to the PHONE: settings rcvd and set in NVS
    server_NUS_Txd_Chr->setValue("!SDone!;");
    server_NUS_Txd_Chr->notify();
    return; // Settings rcvd and set in NVS
  }
  // Manual Control Buttons Up Down get parsed and processed!
  // ONLY when the Actuator plus sensor are working well!
  // i.e. low level up/down movement functions work !!
  if (NusRxdData[1] == 'U') {
    LOG("-> Set motor UPward moving!");
    operations->stepGrade(true);
    return;
  }
  if (NusRxdData[1] == 'D') {
    LOG("-> Set motor DOWNward moving!");
    operations->stepGrade(false);
    return;
  } 
  server_NUS_Txd_Chr->setValue("!UOut of Order!;");
  server_NUS_Txd_Chr->notify();
#ifdef DEBUG_NUS_SHOW_DATA_RAW
  LOG(" -> Server Sends NUS TXD Error message: Out of Order!");
#endif
}; // onWrite

void NUS::server_NUS_Txd_Persistent_Settings(void) {
    // Send persistent stored values to Mobile Phone for correct Settings!
    uint8_t TXpacketBuffer[16] = { 0 };
    uint8_t iMax=0, iMin=0, iPerc=0, iDispl=0;
    // recalculate/convert the control settings for use on the Phone
    std::tie(iMax, iMin, iPerc, iDispl) = operations->getPhonePrefs();
    sprintf((char*)TXpacketBuffer, "!S%d;%d;%d;%d;", iMax, iMin, iPerc, iDispl);
    // send these persistent data to the Settings page on the smartphone
    server_NUS_Txd_Chr->notify(TXpacketBuffer, sizeof(TXpacketBuffer));
#ifdef DEBUG_NUS_SHOW_DATA_RAW
    LOG(" -> Server Sends NUS TXD Persistent settings to Phone: [%s]", (char*)TXpacketBuffer);
#endif
};

void NUS::server_setupNUS(NimBLEServer* pServer) {
    server_NordicUart_Service = pServer->createService(UUID_NUS_SERVICE);
    server_NUS_Rxd_Chr = server_NordicUart_Service->createCharacteristic(UUID_NUS_CHR_RXD, \
                         NIMBLE_PROPERTY::WRITE_NR | NIMBLE_PROPERTY::WRITE_ENC | NIMBLE_PROPERTY::READ_AUTHEN); // No Response !!
    server_NUS_Rxd_Chr->setCallbacks(new server_NUS_Rxd_Chr_callbacks(this)); 
    server_NUS_Txd_Chr = server_NordicUart_Service->createCharacteristic(UUID_NUS_CHR_TXD, \
                         NIMBLE_PROPERTY::READ | NIMBLE_PROPERTY::READ_ENC | NIMBLE_PROPERTY::READ_AUTHEN | NIMBLE_PROPERTY::NOTIFY);
    server_NUS_Txd_Chr->setCallbacks(new server_NUS_Txd_Callbacks(this)); //NIMBLE
    server_NordicUart_Service->start();
};
