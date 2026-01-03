#ifndef COMPONENTS_UTILS_H_
#define COMPONENTS_UTILS_H_

#include <Arduino.h>
#include <string>
#include <cstring>
#include <tuple>
#include <stdarg.h>  // Required for variable argument functions

using SimPrefs = std::tuple<unsigned short, unsigned short, unsigned char, unsigned char>;
using std::get;

#include <NimBLEAddress.h> 

class UTILS {
private:
  UTILS();
  static UTILS* instance;
public:
  ~UTILS();
  static UTILS* getInstance();
  SimPrefs getSIMprefsNVS(void);
  void setSIMprefsNVS(SimPrefs simTuple);
  void clearSIMprefsNVS(void);
  void setMacAddressNVS(const char* key, NimBLEAddress &address);
  NimBLEAddress getMacAddressNVS(const char* key);
  bool parseMacAddressString(const String& macAddress, std::string& address, uint8_t& type);
  std::string toString(const NimBLEAddress& macAddress);
  std::string toHexString(const uint8_t* source, uint8_t length);
  std::string toHexString(const std::string& str);
  bool isValid(const std::string& macString);
  bool isValidNimBLEAddress(const NimBLEAddress &address);
  void logF(const char* format, ...);

  std::string getHexString(const uint8_t* data, size_t length);
  std::string getHexString(const std::string& str);
  std::string getHexString(std::vector<uint8_t> data);
  std::string getHexString(std::vector<uint8_t>* data);

  int getGearNumberFromRatio(float gearRatio);
  static const std::vector<float> knownRatios;
};

#endif // COMPONENTS_UTILS_H_