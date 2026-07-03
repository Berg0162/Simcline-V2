#ifndef OPERATIONS_H_
#define OPERATIONS_H_

#include <Arduino.h>
#include <string>

#include <NimBLEAddress.h> 

#include <tuple>
using PhonePrefs = std::tuple<unsigned char, unsigned char, unsigned char, unsigned char>;
using std::get;

// Struct containing Device info to administer dis/connected devices
typedef struct
{
  NimBLEAddress PeerAddress;
  std::string PeerName;
  uint16_t conn_handle;
  bool IsConnected;
} Device_info_t;

class UTILS;

class OPS {
private:
    OPS();
    static OPS* instance;
    UTILS* utils;
    float grade;
    boolean gradeChanged;
    boolean enableNVS; 
    SemaphoreHandle_t xGradeChangeMutex;
    long RawgradeValue;
    uint16_t aRGVmin;
    uint16_t aRGVmax;
    uint8_t GradeChangeFactor;
    uint8_t OledDisplaySelection;
    int16_t EMA_TargetPositionFilter(int16_t current_value);
    float filterGrade(float currentGrade);
public:
    ~OPS();
    static OPS* getInstance();
    Device_info_t Trainer;
    Device_info_t Laptop;
    Device_info_t Smartphone;
    boolean enableUnknownDev;
    void setEnableUnknownDev(void);
    boolean isTrainerConnected(void);
    boolean isLaptopConnected(void);
    boolean isSmartphoneConnected(void);
    bool getNewGradeIfChanged(float& gradeOut);
    void setNewGrade(float grade);
    void setGradeChanged(boolean changed);
    void stepGrade(bool Up);
    void levelGrade(void);
    int16_t GetNewTargetPosition(float RoadGrade);
    void init(boolean storage=true);
    void storePhonePrefs(PhonePrefs phoneTuple);
    PhonePrefs getPhonePrefs(void);
}; // Class OPS

#endif