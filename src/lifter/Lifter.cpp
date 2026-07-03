#include <VL6180X.h>

/*
 * Lifter class: handles all basic up/down/brake/control actions 
 * Redesigned xTaskControl, compatible with Simcline-V2
 */
#include "Lifter.h"

// ----------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress DEBUG messages that help debugging...
// Uncomment general "#define DEBUG" to activate Debug messages from Class Lifter
#define DEBUG

// ------------------------------------------------------------------------------------------------
// Include these debug utility macros in all cases!
//#include "../config/configDebug.h"
#include "config/configDebug.h"

#ifdef DEBUG
//  Restrict activating one or more of the following << EXTRA >> debug directives --> process intensive 
//  The overhead can lead to spurious side effects and a loss of quality of service handling!!
//#define MOVEMENTDEBUG  // If defined allows for debugging Lifter actions in more detail
#endif

//#include "../config/configBoard.h"
#include "config/configBoard.h"

// Include the mechanical and logical configuration settings of the Simcline
//#include "../config/configSimcline.h"
#include "config/configSimcline.h"

/* Select setup VL6180X Range Continuous or Range Single Shot, read the fine manual for details
 *                https://www.pololu.com/file/0J961/VL6180X.pdf
 * In the present code OPTIONAL Range Continuous Mode is setup for 10Hz sampling -> every 100ms a reading 
 * is available and quicker (<100ms) readings will have to wait for a sample to be ready!
 * In DEFAULT Range Single Shot Mode, it takes less time for a sample to be ready for reading! 
 * The range convergence time is variable and depends on target distance/reflectance */
// Uncomment for selection of Range Continuous Mode, otherwise default is Single Shot Mode
//#define RANGE_CONTINUOUS

// Filter requires a specific number of VL6180X readings before it's stable -> PRIME_COUNT
// If sampling rate is at about 10 Hz -> time to prime filter is about (PRIME_COUNT * 100) ms
// PRIME_COUNT should be in all modes between 5 and 10
#define PRIME_COUNT 7

#if defined(SMA_FILTER)
// SMA_DATA_POINT_WINDOW should be between 5 and 10
#define SMA_DATA_POINT_WINDOW 7
#endif

#if defined(EMA_FILTER)
// EMA_ALPHA should be between low (10-40) is maximal and high (50-90) is minimal filtering
#define EMA_ALPHA 35
#endif 

// Lifter class Constructor
Lifter::Lifter() {
    // Heap allocate the VL6180X sensor object
    sensor = new VL6180X();
#if defined(SMA_FILTER)
    // Heap allocate the SMAFilter object
    SMAFilter = new SMA_Filter(SMA_DATA_POINT_WINDOW);
#endif
#if defined(EMA_FILTER)
    // Heap allocate the EMAFilter object
    EMAFilter = new EMA_Filter(EMA_ALPHA);
#endif
    // Default actuator states
    isBrakeOn = true;
    isMovingUp = false;
    isMovingDown = false;
    // Default position values
    targetPosition = 0;
    currentPosition = 0;	
}

// Lifter class Destructor
Lifter::~Lifter() {
   	// Free the dynamically allocated memory when the object is destroyed
    if (sensor)
        	delete sensor;
#if defined(SMA_FILTER)
	  if (SMAFilter)
		      delete SMAFilter;
#endif
#if defined(EMA_FILTER)
    if (EMAFilter)
          delete EMAFilter;
#endif
}

void Lifter::primeFilter(uint8_t maxCount) {
  // Prime the Filter in stable position with readings and set currentPosition
#ifdef MOVEMENTDEBUG
  std::string logBuffer; // Accumulate output before logging
#endif
  for (uint8_t i = 0; i < maxCount; i++) {
#if defined(RANGE_CONTINUOUS)
  delay(100); // Respect 10 Hz sample rate of VL6180X
#else // Single Shot Mode
  delay(50); // Wait for 50ms
#endif
    currentPosition = getVL6180XRangeReading();
#ifdef MOVEMENTDEBUG
    logBuffer += std::to_string(currentPosition) + " | ";
#endif
    } 
#ifdef MOVEMENTDEBUG
  LOG("Lifter: Primed filter: %s", logBuffer.c_str()); // Send final log buffer
#endif
}

void Lifter::initVL6180X(void) {  
  sensor->init();
  sensor->configureDefault();
  // Set scaling factor to 3 for long range, SIMCLINE needs max. 300 millimeters
  // Raw range values are in units of 3 mm
  sensor->setScaling(3);  // Readings are filtered to increase resolution
// Single Shot operating mode of VL6180X is simplest and default
// The following is extra code critical for using Continuous mode !!!
#if defined(RANGE_CONTINUOUS)
  // Reduce range max convergence time and the inter-measurement
  // -time to 30 ms and 50 ms, respectively, to allow 10 Hz
  // operation. Somewhat more power consumption but higher accuracy!
  sensor->writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
  sensor->writeReg(VL6180X::SYSRANGE__INTERMEASUREMENT_PERIOD, 50);
  // stop continuous mode if already active
  sensor->stopContinuous();
  // in case stopContinuous() triggered a single-shot measurement, wait for it to complete
  delay(300);
  // start range continuous mode with a period of 100 ms
  sensor->startRangeContinuous(100);
  LOG("Lifter: VL6180X Range Continuous Mode Selected");
#else
  LOG("Lifter: VL6180X Single Shot Mode Selected");
#endif
  sensor->setTimeout(500);
}

bool Lifter::Init(void) {
  LOG("Lifter: Pin #1: %d | Pin #2: %d | Bandwidth: %d | MinPos: %d | MaxPos: %d", \
              PIN_ACTUATOR_1, PIN_ACTUATOR_2, BANDWIDTH, MINPOSITION, MAXPOSITION);
  // Initialize VL6180X
  initVL6180X();
  // Test if VL6180X is working
  int retries = 3;
  bool sensorOK = false;
  int16_t tempPosition;
  while (retries-- > 0) {
#if defined(RANGE_CONTINUOUS)
      tempPosition = sensor->readRangeContinuousMillimeters();
#else
      tempPosition = sensor->readRangeSingleMillimeters();
#endif
      if (!sensor->timeoutOccurred()) {
          sensorOK = true;
          break;
      }
      LOG("Lifter: VL6180X Timeout, retrying... (%d retries left)", retries);
      delay(100);
  } // while

  if (!sensorOK) {
      LOG("Lifter: >> ERROR << VL6180X failed after retries. Check wiring and power!");
      return false;
  }

  // Change default zero values in filter to tempPosition
#if defined(SMA_FILTER)
  SMAFilter->presetFilter(static_cast<float>(tempPosition));
  LOG("Lifter: SMA Filter Activated!");
#endif
#if defined(EMA_FILTER)
  EMAFilter->presetFilter(static_cast<float>(tempPosition));
  LOG("Lifter: EMA Filter Activated!");
#endif

  // Prime the Filter with stable readings and set currentPosition
  primeFilter(PRIME_COUNT);

  // Set Target position equal to Current Position
  targetPosition = currentPosition;

  LOG("Lifter: ToF VL6180X Initialized!");
  return true;
}

int16_t Lifter::getVL6180XRangeReading(void) {
    static int timeoutCount = 0;    // Track consecutive timeouts
    const int timeoutThreshold = 3; // Number of timeouts before resetting sensor 
    int16_t tempPosition;  
#if defined(RANGE_CONTINUOUS)
    tempPosition = sensor->readRangeContinuousMillimeters();
#else
    tempPosition = sensor->readRangeSingleMillimeters();
#endif
    // Handle timeout error
    if (sensor->timeoutOccurred()) {
        timeoutCount++;
        LOG("Lifter: >> ERROR << VL6180X Timeout! Attempt %d/%d", timeoutCount, timeoutThreshold);
        // Reset the sensor only if timeouts keep happening
        if (timeoutCount >= timeoutThreshold) {
            LOG("Lifter: Too many timeouts! Resetting VL6180X...");
            brakeActuator();  // Stop movement
            initVL6180X();    // Reset ToF sensor
            timeoutCount = 0; // Reset timeout counter
        }
        return currentPosition; // Return last known good position
    }
    // Reset timeout counter if we get a valid reading
    timeoutCount = 0;
    // Sanity check: Filter out extreme values
    if (tempPosition < MINPOSITION || tempPosition > MAXPOSITION) {
        LOG("Lifter: >> WARNING << VL6180X reported out-of-range value: %d (valid: %d-%d)", \
                                                          tempPosition, MINPOSITION, MAXPOSITION);
        return currentPosition;  // Ignore bad reading
    }
#if defined(SMA_FILTER)
    // Process valid reading through the SMA filter and return
    return static_cast<uint16_t>( std::round(SMAFilter->getFilteredValue(static_cast<float>(tempPosition))) );
#endif
#if defined(EMA_FILTER)
    // Process valid reading through the EMA filter and return
    return static_cast<uint16_t>( std::round(EMAFilter->getFilteredValue(static_cast<float>(tempPosition))) );
#endif
    // No Filter and return
    return tempPosition; 
}
 
bool Lifter::TestBasicMotorFunctions() {
    uint8_t moveCount = 0;
    int16_t upPos;
    uint32_t startTime;
    uint32_t endTime;
 
    // Prime the Filter to get a stable starting position
    primeFilter(PRIME_COUNT);
    LOG("Lifter: Start position: %d", currentPosition);
    if (currentPosition < MINPOSITION || currentPosition > MAXPOSITION) {
      LOG("Lifter: >> ERROR << VL6180X Out of Range at start!!");
      return false;
    }
    // Movement detection: 3 times in a row in limited time!
    upPos = currentPosition;
    LOG("Lifter: Moving UP...");
    moveActuatorUp();
    delay(250); // Give actuator time to respond
    startTime = millis(); // Get the start time
    while ((millis()-startTime) < 3000) { // Max timeout of 3 sec
        currentPosition = getVL6180XRangeReading();
        if (currentPosition <= (MINPOSITION + BANDWIDTH)) { // Check we do not reach the ceiling
            LOG("Lifter: >> WARNING << Reached MINPOSITION limit: %d", currentPosition);
            brakeActuator();
            return false;
        }
        if (currentPosition < upPos) {  // Confirm an upward movement
            upPos = currentPosition;
            moveCount++;
        } else moveCount = 0; // Reset!
        if (moveCount >= 3) {
          endTime = millis();
          break;  // Exit early if consistent UP movements detected
        }
#if defined(RANGE_CONTINUOUS)
        delay(100); // Respect 10 Hz sample rate of VL6180X
#else // Single Shot Mode
        delay(50); // Wait for 50ms
#endif
    } // while
    brakeActuator();
    if (moveCount < 3) {
        LOG("Lifter: >> ERROR << VL6180X did not detect an UP movement!");
        return false;
    }
    LOG("Lifter: 3 Consecutive UP movements detected in: %d ms", (endTime-startTime));
    return true;
}

int Lifter::getOffsetPosition() {
    // Read the current position
    int16_t tempPosition = getVL6180XRangeReading();
    // Check for timeout BEFORE using the measurement
    if (sensor->timeoutOccurred()) {
        return 3;  // Timeout Error state
    }
    currentPosition = tempPosition;  // Now safe to update
    int16_t _PositionOffset = targetPosition - currentPosition;
#ifdef MOVEMENTDEBUG
    LOG("Lifter: Target: %d  Current: %d  Offset: %d", targetPosition, currentPosition, _PositionOffset);
#endif 
    // If within the bandwidth range, stop movement
    if (abs(_PositionOffset) <= BANDWIDTH) {
#ifdef MOVEMENTDEBUG
        LOG("Lifter: Offset within bandwidth [%d]", BANDWIDTH);
#endif
        return 0;  // Position is within the acceptable range
    }
    // Determine movement direction
    if (_PositionOffset < 0)  {
#ifdef MOVEMENTDEBUG
        LOG("Lifter: Offset < 0");
#endif
        return 1;  // Move UP
    }
#ifdef MOVEMENTDEBUG
    LOG("Lifter: Offset > 0");
#endif
    return 2;  // Move Down
}


void Lifter::SetTargetPosition(int16_t Tpos) {
  xSemaphoreTake(xSemaphore, portMAX_DELAY); 
  targetPosition = Tpos;
  xSemaphoreGive(xSemaphore);
}

void Lifter::moveActuatorUp() { 
  if (currentPosition <= (MINPOSITION + BANDWIDTH) ) { 
    // Stop further movement to avoid destruction...
    isMovingUp = false;
    brakeActuator();
    return;
  }
  // DO NOT REPEATEDLY set the same motor direction 
  if (isMovingUp) return;
  // Moving in the opposite direction or not moving at all    
  digitalWrite(PIN_ACTUATOR_1, LOW);                           
  digitalWrite(PIN_ACTUATOR_2, HIGH);
  isMovingUp = true;
  isMovingDown = false;
  isBrakeOn = false;
#ifdef MOVEMENTDEBUG
  LOG("Lifter: Set MovingUp ");
#endif
}

void Lifter::moveActuatorDown() { 
  if (currentPosition >= (MAXPOSITION - BANDWIDTH) ) {
    // Stop further movement to avoid destruction...
    isMovingDown = false;
    brakeActuator();
    return;
  }
  // DO NOT REPEATEDLY set the same motor direction
  if (isMovingDown) return;   
  // Moving in the opposite direction or not moving at all
  digitalWrite(PIN_ACTUATOR_1, HIGH);                           
  digitalWrite(PIN_ACTUATOR_2, LOW);
  isMovingDown = true;
  isMovingUp = false;
  isBrakeOn = false;
#ifdef MOVEMENTDEBUG
  LOG("Lifter: Set MovingDown ");
#endif
}

void Lifter::brakeActuator() { 
  // DO NOT REPEATEDLY stop the motor
  if (isBrakeOn) return;
  digitalWrite(PIN_ACTUATOR_1, LOW);
  digitalWrite(PIN_ACTUATOR_2, LOW);
  isBrakeOn = true;
  isMovingDown = false;
  isMovingUp = false; 
#ifdef MOVEMENTDEBUG
  LOG("Lifter: Set Brake On ");
#endif
}

void Lifter::xTaskControl(void) {
  // Check the Actuator Position and move Motor Up/Down until target position is reached
  static int OnOffsetAction = 0;
#if defined(RANGE_CONTINUOUS)
  const TickType_t xDelay = 100 / portTICK_PERIOD_MS; // Block for 100ms to respect sample rate of VL6180X
#else // Single Shot Mode
  const TickType_t xDelay = 50 / portTICK_PERIOD_MS; // Block for 50ms
#endif

  while(1) {
    if(xSemaphoreTake(xSemaphore, portMAX_DELAY)) {
      // BLE channels can interrupt and consequently target position changes on-the-fly !!
      // We do not want changes in TargetPosition during getOffsetPosition()!!!
      OnOffsetAction = getOffsetPosition(); // Calculate offset to target and determine action
      xSemaphoreGive(xSemaphore);
    }
    switch (OnOffsetAction) {
      case 0 :
        brakeActuator();
        break;
      case 1 :
        moveActuatorUp();
        break;
      case 2 :
        moveActuatorDown();
        break;
      case 3:  // Timeout occurred, handle error
        LOG("Lifter: >> ERROR << Sensor timeout!");
        brakeActuator();
        initVL6180X();  // Reinitialize ToF sensor
        break;
      default:  // OffsetPosition is undetermined --> do nothing and brake
        LOG("Lifter: >> ERROR << Unknown offset status!");
        brakeActuator();
        break;
    } // switch 
    vTaskDelay(xDelay);
  } // while
} // end


// Static defined member for wrapping non-static member xTaskControl
void Lifter::startTaskControl(void* _this) {
    static_cast<Lifter*>(_this)->xTaskControl();
}

bool Lifter::StartControl(const BaseType_t xTaskCoreID) {
    BaseType_t xReturned;
    xSemaphore = xSemaphoreCreateBinary();
    if (xSemaphore == NULL) {
      LOG("Lifter: >> ERROR << Unable to create xSemaphore!");
      return false;
    }
    xReturned = xTaskCreatePinnedToCore(this->startTaskControl, \
                                  "xTaskControl", 4096, this, 10, &ControlTaskHandle, xTaskCoreID);
    if( xReturned != pdPASS ) {
      LOG("Lifter: >> ERROR << Unable to create xTaskControl!");
      return false;
    }
     xSemaphoreGive(xSemaphore);
     return true;
}
