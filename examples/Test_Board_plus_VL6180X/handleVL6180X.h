
// Include the VL6180X Time of Flight Sensor library
#include <VL6180X.h>
// Declaration of the VL6180X class instance
VL6180X sensor;
// Set scaling of the VL6180X to approriate value 1, 2 or 3
// Only scaling factor #3 will work in a Simcline setup of measuring distance equal or larger than 30 cm!
#define SCALING 3
// setup VL6180X Range Continuous or Range Single Shot, read the manual.... 
#define RANGE_CONTINUOUS 0 // 1 = Range Continuous   0 = Range Single Shot
uint16_t CurrentPosition;

// Include Moving Average Filter
#include <lifter/MovingAverageFilter.h>
// Declare the running average filter for VL6180X Range measurements
// Filter is only used in the Lifter Class 
// Sampling is at about 10 Hz --> 10 VL6180X-RANGE-readings per second
#define NUMBER_OF_RANGE_READINGS 10
// Instantiate MovingAverageFilter class
MovingAverageFilter movingAverage(NUMBER_OF_RANGE_READINGS);

int16_t GetVL6180X_Range_Reading(void);
void Fill_Moving_Average_Filter(void);
void InitVL6180X(void);

void Fill_Moving_Average_Filter(void) {
  // fill the movingAverageFilter with current values
  // these blur operation when movement changes of direction
  for (int i = 0; i < NUMBER_OF_RANGE_READINGS; i++) {
    delay(100); // Respect sample rate of 10 Hz
    CurrentPosition = GetVL6180X_Range_Reading();
    } 
}

void InitVL6180X(void) {  
  // setup VL6180X settings and operating mode
  sensor.init();
  LOG("ToF Sensor VL6180X Initialized!");
  sensor.configureDefault();
  sensor.setScaling(SCALING);
  LOG(" Scaling: %dx\n", sensor.getScaling());
  // Single shot operating mode of VL6180X is simplest and default
  // The following is extra code critical for using Continuous mode !!!
#if RANGE_CONTINUOUS
  // Reduce range max convergence time and the inter-measurement
  // -time to 30 ms and 50 ms, respectively, to allow 10 Hz
  // operation. Somewhat more power consumption but higher accuracy!
  sensor.writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
  sensor.writeReg(VL6180X::SYSRANGE__INTERMEASUREMENT_PERIOD, 50);
  // stop continuous mode if already active
  sensor.stopContinuous();
  // in case stopContinuous() triggered a single-shot
  // measurement, wait for it to complete
  delay(300);
  // start range continuous mode with a period of 100 ms
  sensor.startRangeContinuous(100);
  LOG("VL6180X Range Continuous Mode Selected");
#else 
  LOG("VL6180X Range Single Shot Mode Selected");
#endif
  sensor.setTimeout(500);
}

int16_t GetVL6180X_Range_Reading(void) {
#if RANGE_CONTINUOUS
    int16_t temp = sensor.readRangeContinuousMillimeters();
#else
    int16_t temp = sensor.readRangeSingleMillimeters();
#endif
    if (sensor.timeoutOccurred()) 
        {
        LOG(">> ERROR << --> VL6180X reports TIMEOUT --> VL6180X is reset --> continue");
        // Handle timeout error state VL6180X
        InitVL6180X();           // Initialize ToF VL6180X again --> this resets timeout error state!
        return CurrentPosition;  // Do NOT use latest (temp) reading, it is not valid due to the timeout!!
        }
  return movingAverage.process(temp);  
} 

