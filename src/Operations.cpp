#include "Operations.h"

// ----------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress DEBUG messages that help debugging...
// Uncomment general "#define DEBUG" to activate Debug messages for this class
#define DEBUG
// Include these debug utility macros in all cases!
#include "config/configDebug.h"

#ifdef DEBUG
//  Restrict activating one or more of the following << EXTRA >> debug directives --> process intensive
//  The overhead can lead to spurious side effects and a loss of quality of service handling!!
// ------------------------------------------------------------------------------------------------
//#define SIMPERCENTAGESET          // Show Simcline settings for RoadGrade and RawgradeValue
//#define MOVEMENTDEBUG             // Show Simcline settings for RoadGrade, RawgradeValue and TargetPosition
#endif

// -----------------------------------------------------------------------------------------------
// Include all NimBLE constants
#include "config/configNimBLE.h"
/** Connection handle not present --> default NimBLE */
#define BLE_HS_CONN_HANDLE_NONE 0xffff

// ------------------------------------------------------------------------------------------------
// Include the mechanical and logical configuration settings of the Simcline
#include "config/configSimcline.h"

// Raw Grade Value Minimally (Mechanically: the lowest position of wheel axis)  19000 is equiv. of 10% downhill road grade
// These values are derived from the RGVMIN and RGVMAX settings in "config/configSimcline.h"
#define RGVMIN_GRADE (20000 - RGVMIN) / 100  // Notice: positive value of the Minimal downhill grade!
#define RGVMAX_GRADE (RGVMAX - 20000) / 100  // Notice: positive value of the Maximal uphill grade!

#include "Utilities.h"

// Initialize the static members
OPS* OPS::instance = nullptr;

// ---------------------------------------------------------------------------------------
// Exponential EMA ALPHA filter definition
// Used to filter sequence of actuator target positions --> minimize consecutive small up/down movements
// Should be between low (10-40) is maximal and high (50-90) is minimal filtering
// Uncomment "#define EMA_ALPHA" to activate
//#define EMA_ALPHA 60    // Value is in percentage 0-99.

//----------------------------------------------------------------------------------------
// After Zwift has paired with your trainer it sends a first road grade value (randomly between 2.0 and 5.0) 
// to the trainer, to wake-up/activate the resistance mechanism. You will feel the resistance while you were 
// expecting a flat road for a start. This road grade value remains constant, until you have actually started 
// with the ride of your choice! Simcline, being a MITM, will lift your front wheel accordingly .. unless ..
// Uncomment "#define FILTERINITIALGRADES" to filter these initial values and to level Simcline at startup!
#define FILTERINITIALGRADES

// New Settings values have arrived --> parse, set values and store persistently
void OPS::storePhonePrefs(PhonePrefs phoneTuple) {
  // set Ascent Grade Limit to aRGVmax
  get<0>(phoneTuple) = constrain(get<0>(phoneTuple), 0, RGVMAX_GRADE);
  aRGVmax = map(get<0>(phoneTuple), 0, RGVMAX_GRADE, 20000, RGVMAX);
  // set Descent Grade Limit to aRGVmin
  get<1>(phoneTuple) = constrain(get<1>(phoneTuple), 0, RGVMIN_GRADE);  // Notice: positive value!
  aRGVmin = map(get<1>(phoneTuple), RGVMIN_GRADE, 0, RGVMIN, 20000);
  // set Road Grade Change Factor
  get<2>(phoneTuple) = constrain(get<2>(phoneTuple), 0, 200);
  GradeChangeFactor = get<2>(phoneTuple);
  // set OledDisplaySelection
  OledDisplaySelection = get<3>(phoneTuple);
  if (enableNVS) {
    SimPrefs simPrefs(aRGVmax, aRGVmin, GradeChangeFactor, OledDisplaySelection);
    utils->setSIMprefsNVS(simPrefs);
  }
}

PhonePrefs OPS::getPhonePrefs(void) {
  PhonePrefs phonePrefs(RGVMAX_GRADE, RGVMIN_GRADE, 100, 2); // Set default phone values
  // Constrain aRGVm** within limits
  aRGVmax = constrain(aRGVmax, 20000, RGVMAX);
  aRGVmin = constrain(aRGVmin, RGVMIN, 20000);
  GradeChangeFactor = constrain(GradeChangeFactor, 0, 200);
  // set aRGVmax to Ascent Grade Limit in whole number
  get<0>(phonePrefs) = (uint8_t)((aRGVmax - 20000) / 100);
  // set aRGVmin to Descent Grade Limit in whole number
  get<1>(phonePrefs) = (uint8_t)((20000 - aRGVmin) / 100);
  // set GradeChangeFactor to Road Grade Change Factor
  get<2>(phonePrefs) = GradeChangeFactor;
  get<3>(phonePrefs) = OledDisplaySelection;
  return phonePrefs;
}

// ---------------------------------------------------------------------------------
#ifdef EMA_ALPHA
int16_t OPS::EMA_TargetPositionFilter(int16_t current_value) {
  static int16_t exponential_average = current_value;
  exponential_average = int16_t((EMA_ALPHA * (uint32_t)current_value + (100 - EMA_ALPHA) * (uint32_t)exponential_average) / 100);
  return exponential_average;
}
#endif

OPS::OPS() {
  enableUnknownDev = false;
  utils = UTILS::getInstance();
}

OPS::~OPS() { }

OPS* OPS::getInstance() {
    if (instance == nullptr) {
        instance = new OPS();
    }
    return instance;
}

void OPS::setEnableUnknownDev(void) {
#if defined (TRAINERADDRESS) && defined (LAPTOPADDRESS)
  LOG("--> Only defined devices are allowed to connect!");
#else
  enableUnknownDev = true; 
  LOG("--> Unknown device is allowed to connect!"); 
#endif
}

void OPS::setGradeChanged(boolean changed) {
  xSemaphoreTake(xGradeChangeMutex, portMAX_DELAY);
  gradeChanged = changed;
  xSemaphoreGive(xGradeChangeMutex);
}

void OPS::stepGrade(bool Up) {
  // Keep values within the safe range
  if (Up) {
    if (grade++ > 0) grade = constrain(grade, 0, RGVMAX_GRADE);
  } else {
    if (grade-- < 0) grade = -constrain(-grade, 0, RGVMIN_GRADE);
  }
  setGradeChanged(true);
}

void OPS::levelGrade(void) {
  RawgradeValue = 20000;                       // Level value RawGrade
  grade = float(RawgradeValue - 20000) / 100;  // Level value grade
  OPS::setGradeChanged(true);
}

int16_t OPS::GetNewTargetPosition(float RoadGrade) {
  RawgradeValue = (long)(RoadGrade * 100) + 20000;
  // Take into account the measuring offset when calculating targetPosition!
  RawgradeValue = RawgradeValue - MEASUREOFFSET;
  // Test for Maximally en Minimally Allowed Raw Grade Values ----------------------------------------
  if (RawgradeValue < aRGVmin) {
    RawgradeValue = aRGVmin;  // Do not allow lower values than aRGVmin !!
  }
  if (RawgradeValue > aRGVmax) {
    RawgradeValue = aRGVmax;  // Do not allow values to exceed aRGVmax !!
  }
  // --------------------------------------------------------------------------------------------------
#ifdef SIMPERCENTAGESET
  LOG("Set Simcline to Percentage: %03.1f%% RawgradeValue: %05d\n", RoadGrade, RawgradeValue);
#endif
  // Handle mechanical movement i.e. wheel position in accordance with Road Inclination
  // Map RawgradeValue ranging from 0 to 40.000 on the
  // TargetPosition (between MINPOSITION and MAXPOSITION) of the Lifter
  // Notice 22000 is equivalent to +20% incline and 19000 to -10% incline
  RawgradeValue = constrain(RawgradeValue, RGVMIN, RGVMAX);  // Keep values within the safe range
  int16_t TargetPosition = map(RawgradeValue, RGVMIN, RGVMAX, MAXPOSITION, MINPOSITION);
  // EMA filter for smoothing quickly fluctuating Target Position values..
#ifdef EMA_ALPHA
  TargetPosition = OPS::EMA_TargetPositionFilter(TargetPosition);
#endif
#ifdef MOVEMENTDEBUG
  LOG("RawgradeValue: %05d Grade percent: %03.1f%% TargetPosition: %03d", RawgradeValue, RoadGrade, TargetPosition);
#endif
  return TargetPosition;
}

void OPS::init(boolean storage) {
  enableNVS = storage;  // Default Enable Access to Non-Volatile Storage of Simcline settings
  // SIMprefs data -------------------------------------------------------------------------------------
  // Besides what is mechanically possible there are also limits in what is physically pleasant
  // Keep the following aRGVMin and aRGVMax values within the limits of the mechanically feasible values !!!
  // DEFAULT Minimally Allowed Raw Grade Value that should not be exceeded
  aRGVmin = RGVMIN;
  // DEFAULT Maximally Allowed Raw Grade Value that should not be exceeded
  aRGVmax = RGVMAX;
  GradeChangeFactor = 100;  // 100% means no effect (default), 50% means halved road grade values 
  // For backward compatibility for TFT Display selection 1 (Cycling data) or 2 (Road Grade)
  OledDisplaySelection = 2;  // default Road Grade to show
  // SIMprefs data -------------------------------------------------------------------------------------
  if (enableNVS) {
    std::tie(aRGVmax, aRGVmin, GradeChangeFactor, OledDisplaySelection) = utils->getSIMprefsNVS();
  } else LOG("Simcline settings have Default Values!");
  gradeChanged = false;
  // Set default values for road grade and status
  grade = 0.0;
  // Default value for a flat road equals 0% grade or a RGV of 20000
  RawgradeValue = 20000;
  // Create mutex for road grade (changed) protected exchange
  xGradeChangeMutex = xSemaphoreCreateMutex();
  xSemaphoreGive(xGradeChangeMutex);

  // Assign connectable devices default values (defined in header file or from NVS)
  std::string address; // Address part
  uint8_t type; // Type part
#ifdef TRAINERADDRESS
  LOG("Defined Trainer MAC Address: [%s]", TRAINERADDRESS); 
  // Handle defined Address String, break up in parts and create NimBLEAddress
  if( utils->parseMacAddressString(TRAINERADDRESS, address, type) ) Trainer.PeerAddress = NimBLEAddress(address.c_str(), type);
  else Trainer.PeerAddress = NimBLEAddress(); // Empty string
#else
  Trainer.PeerAddress = utils->getMacAddressNVS("Trainer");
#endif
  Trainer.PeerName = "MyTrainer";
  Trainer.IsConnected = false;
  Trainer.conn_handle = BLE_HS_CONN_HANDLE_NONE;

#ifdef LAPTOPADDRESS
  LOG("Defined Laptop MAC Address: [%s]", LAPTOPADDRESS); 
  // Handle defined Address String, break up in parts and create NimBLEAddress
  if( utils->parseMacAddressString(LAPTOPADDRESS, address, type) ) Laptop.PeerAddress = NimBLEAddress(address.c_str(), type);
  else Laptop.PeerAddress = NimBLEAddress(); // Empty string
#else
  Laptop.PeerAddress = utils->getMacAddressNVS("Laptop");
#endif
  Laptop.PeerName = "MyLaptop";
  Laptop.IsConnected = false;
  Laptop.conn_handle = BLE_HS_CONN_HANDLE_NONE;

  Smartphone.PeerAddress = NimBLEAddress();
  Smartphone.PeerName = "MyPhone";
  Smartphone.IsConnected = false;
  Smartphone.conn_handle = BLE_HS_CONN_HANDLE_NONE;
}

boolean OPS::isGradeChanged(void) {
  return gradeChanged;
};

float OPS::getNewGrade(void) {
  OPS::setGradeChanged(false);
  return grade;
};

#ifdef FILTERINITIALGRADES
/* Zwift starts with a wake up stream of random grades between 2.0 and 5.0
*  Legacy Wahoo trainers receive a stream of equal grade values 
*  FTMS trainers receive a stream of 2 unequal grade values 
*  filterGrade keeps the starting grade value at level -> NO LIFT at startup */
float OPS::filterGrade(float currentGrade) {
  // Static variables to retain state across function calls
  static float initialGrade = 0.0f;
  static bool isInitialized = false;
  static bool passValues = false;
  static uint8_t cnt = 0;

  if (!isInitialized) {
    initialGrade = currentGrade;
    isInitialized = true;
    return 0.0f; // Ignore grade during initialization
  }
  if (!passValues) {
    if (currentGrade != initialGrade) { // Ignore equal values from the start
      cnt++; // Count unequal values
      if (cnt > 3) passValues = true; // Set passValues after 3 unequal readings
    }
    return 0.0f; // Ignore grades until passValues == true
  }
  return currentGrade; // Pass on
};
#endif

void OPS::setNewGrade(float roadGrade) {
#ifdef FILTERINITIALGRADES
  if(isLaptopConnected()) // Filter only when Zwift is connected
    grade = filterGrade(roadGrade);
  else grade = roadGrade;
#else
  grade = roadGrade;
#endif
  // Take into account the allowed Increase Percentage of the inclination
  // 100% has no effect, 50% means road grade value is divided by 2
  grade = grade * GradeChangeFactor/100;
  OPS::setGradeChanged(true);
};

boolean OPS::isTrainerConnected(void) {
  return Trainer.IsConnected;
};

boolean OPS::isLaptopConnected(void) {
  return Laptop.IsConnected;
};

boolean OPS::isSmartphoneConnected(void) {
  return Smartphone.IsConnected;
};

