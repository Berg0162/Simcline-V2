// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress DEBUG messages that help debugging...
// Uncomment general "#define DEBUG" to activate debug messages for this class
#define DEBUG
// Include these debug utility macros in all cases!
#include "config/configDebug.h"

#include <regex> // For regular expression validation

// Include the mechanical and logical configuration settings of the Simcline
#include "config/configSimcline.h"

#include "Utilities.h"

// NVS for internal storage of persistent data on the ESP32
#include <Preferences.h>
// Open Preferences with <name> namespace. Each application module, library, etc
// has to use a namespace <name> to prevent key name collisions.
// Note: Namespace <name> is limited to 15 chars.
#define NVSNAMESPACE "SIMprefs"
// Create an instance of class Preferences
Preferences* preferences = new Preferences();

// Initialize the static members
UTILS* UTILS::instance = nullptr;

UTILS::UTILS() { }

UTILS::~UTILS() { }

UTILS* UTILS::getInstance() {
    if (instance == nullptr) {
        instance = new UTILS();
    }
    return instance;
}

// NVS --------------------------------------------------

SimPrefs UTILS::getSIMprefsNVS(void) {
  SimPrefs simPrefs(RGVMAX, RGVMIN, 100, 2); // Set Default values
  // We will open storage in RW-mode (second parameter has to be false).
  if (!preferences->begin(NVSNAMESPACE, false)) {
    LOG(">>> NVS ERROR Open: <%s> Failed!", NVSNAMESPACE);
    return simPrefs;
  }
  // Get current variables from NVS NAMESPACE
  get<0>(simPrefs) = preferences->getUShort("iMax", RGVMAX);
  get<1>(simPrefs) = preferences->getUShort("iMin", RGVMIN);
  get<2>(simPrefs) = preferences->getUChar("iPerc", 100);
  get<3>(simPrefs) = preferences->getUChar("iDispl", 2);
  preferences->end();  // Close NVSNAMESPACE
  LOG("Get Simcline settings <%s> Max: %d Min: %d Perc.: %d Displ.: %d", NVSNAMESPACE, get<0>(simPrefs), \
                                                          get<1>(simPrefs), get<2>(simPrefs), get<3>(simPrefs));
  return simPrefs;
}

void UTILS::setSIMprefsNVS(SimPrefs simTuple) {
  // We will open storage in RW-mode (second parameter has to be false).
  if (!preferences->begin(NVSNAMESPACE, false)) {
    LOG(">>> NVS ERROR Open: <%s> Failed!", NVSNAMESPACE);
    return;
  }
  // Set new variables in NVS NAMESPACE
  preferences->putUShort("iMax", get<0>(simTuple));
  preferences->putUShort("iMin", get<1>(simTuple));
  preferences->putUChar("iPerc", get<2>(simTuple));
  preferences->putUChar("iDispl", get<3>(simTuple));
  preferences->end();  // Close NVSNAMESPACE
  LOG("Set Simcline settings <%s> Max: %d Min: %d Perc.: %d Displ.: %d", NVSNAMESPACE, get<0>(simTuple), \
                                                        get<1>(simTuple), get<2>(simTuple), get<3>(simTuple));
}

void UTILS::clearSIMprefsNVS(void) {
  // We will open storage in RW-mode (second parameter has to be false).
  if (!preferences->begin(NVSNAMESPACE, false)) {
    LOG(">>> NVS ERROR Open: <%s> Failed!", NVSNAMESPACE);
    return;
  }
  preferences->clear();  // Remove all preferences of NVSNAMESPACE
  LOG("NVS Namespace <%s> cleared!", NVSNAMESPACE);
  preferences->end();  // Close NVSNAMESPACE
}

void UTILS::setMacAddressNVS(const char* key, NimBLEAddress &address) {
  if(!preferences->begin(NVSNAMESPACE, false)) {
    LOG(">>> NVS ERROR Open: <%s> Failed!", NVSNAMESPACE);
    return;
  }
  preferences->putString(key, toString(address).c_str());  // Store as string
  preferences->end();  // Close NVSNAMESPACE
  LOG("Set NimBLE settings <%s> %s MAC Address: [%s]", NVSNAMESPACE, key, toString(address).c_str());
}

NimBLEAddress UTILS::getMacAddressNVS(const char* key) {
  if(!preferences->begin(NVSNAMESPACE, false)) {
    LOG(">>> NVS ERROR Open: <%s> Failed!", NVSNAMESPACE);
    return NimBLEAddress();  // Return a default empty address
  }
  String nvsAddress = preferences->getString(key, "");  // Retrieve stored String
  if(nvsAddress.length() == 0)
    nvsAddress = "00:00:00:00:00:00 0"; // Empty the first time!
  preferences->end(); // Close NVSNAMESPACE
  // Handle extended nvsAddress String, break up in parts and create NimBLEAddress
  std::string address; // Address part
  uint8_t type; // Type part
  NimBLEAddress macAddress; // Final NimBLEAddress
  if(parseMacAddressString(nvsAddress, address, type)) macAddress = NimBLEAddress(address.c_str(), type);
  else macAddress = NimBLEAddress(); // Empty string

  //NimBLEAddress address = NimBLEAddress(nvsAddress.c_str(), TYPE);

  LOG("Get NimBLE settings <%s> %s MAC Address: [%s]", NVSNAMESPACE, key, toString(macAddress).c_str());
  return macAddress;
}

bool UTILS::parseMacAddressString(const String& macAddress, std::string& address, uint8_t& type) {
    char addressBuffer[18]; // Buffer to store the MAC address (17 chars + null terminator)
    char empty[18] = "00:00:00:00:00:00";
    int tempType;
    bool result = true;

    // Use sscanf to parse the input string
    if(sscanf(macAddress.c_str(), "%17s %d", addressBuffer, &tempType) != 2) {
       LOG(">>> ERROR: Invalid Mac Address!");
       result = false;
     }
     // Validate the MAC address format using a regular expression
     std::regex macRegex("^([0-9A-Fa-f]{2}:){5}[0-9A-Fa-f]{2}$");
     if (!std::regex_match(addressBuffer, macRegex)) {
       LOG("Invalid MAC Address: malformed -> [%s]", addressBuffer);
       memcpy(addressBuffer, empty, 17);
       result = false;
     }
     if( !(tempType == 0 || tempType == 1) ) {
       LOG("Invalid MAC Address Type -> [%d]", tempType);
       tempType = 0;
       result = false;
     }
     type = static_cast<uint8_t>(tempType); // Convert the parsed type to uint8_t
     address = addressBuffer; // Assign the parsed address to the string
     return result;
}

// NVS --------------------------------------------------

// Uppercase alternative to NimBLEAddress <string>
std::string UTILS::toString(const NimBLEAddress& macAddress) {
  const uint8_t* macBytes = macAddress.getBase()->val; 
  const uint8_t macType = macAddress.getBase()->type;
  char stringMAC[20]; // 6*2 bytes + 5 colons + space + type + 1 null terminator = 20
  // Undo Little Endian machine-representation
  snprintf(stringMAC, sizeof(stringMAC), "%02X:%02X:%02X:%02X:%02X:%02X %1d", macBytes[5], macBytes[4], \
           macBytes[3], macBytes[2], macBytes[1], macBytes[0], macType); // byte by byte in HEX --> UPPERCASE
  return std::string(stringMAC);
}

/**
  * @brief Convert binary data to a formatted hexadecimal string.
  * @param [in] source The start of the binary data.
  * @param [in] length The length of the data to convert.
  * @return A formatted string representation of the data (e.g., "01 2A EF").
  */
std::string UTILS::toHexString(const uint8_t* source, uint8_t length) {
    if (length == 0) return "";  // Handle empty input
    std::string str;
    str.reserve(length * 3 - 1);  // Preallocate memory (length * 3 - 1 for spaces)
    char buffer[4];  // Buffer for "XX " (2 chars + space + null terminator)
    for (uint8_t i = 0; i < length; i++) {
        snprintf(buffer, sizeof(buffer), "%02X%s", source[i], (i < length - 1) ? " " : "");  
        str.append(buffer);
    }
    return str;
}

/**
  * @brief Overloaded function to convert a std::string to a formatted hexadecimal string.
  * @param [in] str The input string containing raw bytes.
  * @return A formatted hexadecimal string (e.g., "48 65 6C 6C 6F" for "Hello").
  */
std::string UTILS::toHexString(const std::string& str) {
      return UTILS::toHexString( reinterpret_cast<const uint8_t*>(str.data()), static_cast<uint8_t>(str.length()) );
}

void UTILS::logF(const char* format, ...) {
    static char buffer[256];  // Static buffer to prevent stack overflows
    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer) - 2, format, args);  // Leave space for \n and \0
    va_end(args);

    if (len > 0) {
        buffer[len] = '\n';   // Append newline
        buffer[len + 1] = '\0'; // Null-terminate
    }

    Serial.print(buffer);  // Send buffer in one atomic operation
    delay(2);              // Small delay to stabilize serial output
}

bool UTILS::isValidNimBLEAddress(const NimBLEAddress &address) {
    if(address.isNull()) {
      LOG(" >>> Invalid MAC Address -> Empty");
      return false;
    }
    if(address.getType() == 0 || address.getType() == 1)
      return true;
    else {
      LOG(" >>> Invalid MAC Address Type ");
      return false;
    }
};

bool UTILS::isValid(const std::string& macString) {
    // macString address must be 17 HEX characters with separator colons
    if (macString.length() != 17) {
        LOG(" >>> Invalid MAC Address Length ");
        return false;
    }
    // Check that each character is either a HEX digit or a separator colon
    for (int i = 0; i < macString.length(); i++) {
        if ((i + 1) % 3 == 0) {
            if (macString[i] != ':') {
                LOG(" >>> Invalid Separator Colon ");
                return false;
            }
        } else {
            if (!isxdigit(macString[i])) { // Is HEX?
                LOG(" >>> Invalid HEX Digit ");
                return false;
            }
        }
    }
    return true;
}

// Known gear ratios for gears 1 to 24
const std::vector<float> UTILS::knownRatios = {0.75, 0.87, 0.99, 1.11, 1.23, 1.38, 1.53, 1.68, 1.86, 2.04, 2.22, 2.40, 2.61, 2.82, \
                                              3.03, 3.24, 3.49, 3.74, 3.99, 4.24, 4.54, 4.84, 5.14, 5.49};

int UTILS::getGearNumberFromRatio(float gearRatio) {
    const float epsilon = 0.01; // tolerance for floating-point comparison
    for (size_t i = 0; i < knownRatios.size(); ++i) {
        if (std::fabs(gearRatio - knownRatios[i]) < epsilon) {
            return static_cast<int>(i + 1); // gears are 1-based
        }
    }
    return 0; // Not found
}

std::string UTILS::getHexString(const uint8_t* data, size_t length) {
  static char hexNumber[3];
  std::string hexString = "[";
  for (size_t index = 0; index < length; index++) {
    sprintf(hexNumber, "%02X", data[index]);
    hexString.append(hexNumber);
    if (index < (length - 1)) {
      hexString.append(" ");
    }
  }
  hexString.append("]");
  return hexString;
}

std::string UTILS::getHexString(const std::string& str) {
      return getHexString( reinterpret_cast<const uint8_t*>(str.data()), static_cast<uint8_t>(str.length()) );
}

std::string UTILS::getHexString(std::vector<uint8_t> data) {
  return getHexString(data.data(), data.size());
}

std::string UTILS::getHexString(std::vector<uint8_t>* data) {
  return getHexString(data->data(), data->size());
}

