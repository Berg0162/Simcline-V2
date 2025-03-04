/*
 * Lifter class: all basic up/down/brake actions 
 * Version #2 code changes
 * 10/01/2022 -> VL6180X timeout errors -> reset VL6180X separately and continue
 * 10/02/2022 -> More debug info, rework of settings, Single Shot is active, delay's deleted
 * 20/06/2024 -> Redesign xTaskControl, compatible with Simcline 2.0
 */
#include "Lifter.h"

// ----------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress DEBUG messages that help debugging...
// Uncomment general "#define DEBUG" to activate Debug messages from Class Lifter
#define DEBUG

// ------------------------------------------------------------------------------------------------
// Include these debug utility macros in all cases!
#include "../config/configDebug.h"

#ifdef DEBUG
//  Restrict activating one or more of the following << EXTRA >> debug directives --> process intensive 
//  The overhead can lead to spurious side effects and a loss of quality of service handling!!
//#define MOVEMENTDEBUG  // If defined allows for debugging Lifter actions in more detail
#endif

#include "../config/configBoard.h"

// Include the mechanical and logical configuration settings of the Simcline
#include "../config/configSimcline.h"

// NOTICE: COMPILER DIRECTIVE !!!!
// setup VL6180X Range Continuous or Single Shot, read the manual.... 
#define _RANGE_CONTINUOUS 0 // 1 = Range Continuous   0 = Single Shot

// Declare the running average filter for VL6180X Range measurements
// Filter is only used in the Lifter Class 
// Sampling is at about 10 Hz --> 10 VL6180X-RANGE-readings per second
#define _NUMBER_OF_RANGE_READINGS 10

// Define Lifter class Constructor
  Lifter::Lifter() {
	// Dynamically allocate the VL6180X sensor object
    	sensor = new VL6180X();
	// Dynamically allocate the MovingAverageFilter object
	movingAverageFilter_Range = new MovingAverageFilter(_NUMBER_OF_RANGE_READINGS);	
}

// Define Lifter class Destructor
   Lifter::~Lifter() {
   	// Free the dynamically allocated memory when the object is destroyed
    	if (sensor)
        	delete sensor;
	if (movingAverageFilter_Range)
		delete movingAverageFilter_Range;
}

void Lifter::Fill_Moving_Average_Filter(void)
{
  // fill the movingAverageFilter with current values
  // these blur operation when movement changes of direction
  for (int i = 0; i < _NUMBER_OF_RANGE_READINGS; i++) {
    delay(100); // Respect sample rate of 10 Hz
    _CurrentPosition = GetVL6180X_Range_Reading();
    } 
}

void Lifter::InitVL6180X(void)
{  
// setup VL6180X settings and operating mode
// Range Continuous or Single Shot, read the manual.... 
// Set scaling (after configureDefault = 1) of the VL6180X to approriate value 1, 2 or 3
// Only scaling factor #3 will work in our situation of 30+ cm range !!!!
#define _SCALING 3
  sensor->init();
  sensor->configureDefault();
  sensor->setScaling(_SCALING);
// Single shot operating mode of VL6180X is simplest and default
// The following is extra code critical for using Continuous mode !!!
#if _RANGE_CONTINUOUS
  // Reduce range max convergence time and the inter-measurement
  // -time to 30 ms and 50 ms, respectively, to allow 10 Hz
  // operation. Somewhat more power consumption but higher accuracy!
  sensor->writeReg(VL6180X::SYSRANGE__MAX_CONVERGENCE_TIME, 30);
  sensor->writeReg(VL6180X::SYSRANGE__INTERMEASUREMENT_PERIOD, 50);
  // stop continuous mode if already active
  sensor->stopContinuous();
  // in case stopContinuous() triggered a single-shot
  // measurement, wait for it to complete
  delay(300);
  // start range continuous mode with a period of 100 ms
  sensor->startRangeContinuous(100);
  LOG("VL6180X Range Continuous Mode Selected");
#else 
  LOG("VL6180X Single Shot Mode Selected");
#endif
  sensor->setTimeout(500);
}

boolean Lifter::Init(void)
{
  _actuatorOutPin1 = PIN_ACTUATOR_1;
  _actuatorOutPin2 = PIN_ACTUATOR_2;
  _BANDWIDTH = BANDWIDTH;
  _MINPOSITION = MINPOSITION;
  _MAXPOSITION = MAXPOSITION;
  LOG("Pin #1: %d  Pin #2: %d  BandWidth: %d  VL6180X MinPosition: %d VL6180X MaxPosition: %d", \
			_actuatorOutPin1, _actuatorOutPin2, _BANDWIDTH, _MINPOSITION, _MAXPOSITION);  
// Private variables for position control
  _IsBrakeOn = true;
  _IsMovingUp = false;
  _IsMovingDown = false;
// ------------version #2 Setup I2C and initialize VL6180X
  InitVL6180X(); 
// Test for a working sensor --> can we read position without a timeout!!
#if _RANGE_CONTINUOUS
  int16_t temp = sensor->readRangeContinuousMillimeters();
#else
  int16_t temp = sensor->readRangeSingleMillimeters();
#endif
  if (sensor->timeoutOccurred()) {
        LOG(">> ERROR << --> VL6180X reports TIMEOUT --> Not working!");
	return false; // failed
  }
// ----------- version #2
  // fill the movingAverageFilter with actual values instead of default zero's....
  // that blur operation in the early stages (of testing..)
  Fill_Moving_Average_Filter();
  _TargetPosition = _CurrentPosition; // Set default: No offset
  LOG("ToF VL6180X Initialized!");
  return true;
}

int16_t Lifter::GetVL6180X_Range_Reading()
{
#if _RANGE_CONTINUOUS
    int16_t temp = sensor->readRangeContinuousMillimeters();
#else
    int16_t temp = sensor->readRangeSingleMillimeters();
#endif
    if (sensor->timeoutOccurred()) 
        {
        LOG(">> ERROR << --> VL6180X reports TIMEOUT --> VL6180X is reset --> continue");
        // ------------version #2 handle timeout error state VL6180X
        brakeActuator();         // Stop any movement to avoid erroneous behavior of the Actuator
        InitVL6180X();           // Initialize ToF VL6180X again --> this resets timeout error state!
        return _CurrentPosition; // Do NOT use latest (temp) reading, it is not valid due to the timeout!!
        // ----------- version #2
        }
  return movingAverageFilter_Range->process(temp);  
} 

bool Lifter::TestBasicMotorFunctions()
{
  LOG("Testing VL6180X and motor functioning...");
  int16_t PresentPosition01 = GetVL6180X_Range_Reading();
  LOG("Start at position: %d", PresentPosition01);
  if (PresentPosition01 != (constrain(PresentPosition01, _MINPOSITION, _MAXPOSITION)) )
  { // VL6108X is out of Range ... ?
    LOG(">> ERROR << --> VL6108X Out of Range at start !!");
    return false; 
  }
  LOG("Moving UP ...");
  moveActuatorUp();
  delay(800); // Wait for some time
  brakeActuator();
  int16_t PresentPosition02 = (_CurrentPosition + _BANDWIDTH);
  if (PresentPosition02 != (constrain(PresentPosition02, _MINPOSITION, _MAXPOSITION)) )
  { // VL6108X is out of Range ... ?
    LOG(">> ERROR << -> VL6108X Out of Range");
    return false; 
  }
  if (!(PresentPosition02 < PresentPosition01))
    { 
    LOG(">> ERROR << -> VL6108X did not detect an UP movement");
    return false;
    }
  // VL6108X is properly working moving UP! ------------------------------------ 
  LOG("Moving Down ...");
  moveActuatorDown();
  delay(1600); // Wait some time (extra to "undo" the previous Up movement!!)
  brakeActuator();
  PresentPosition01 = (_CurrentPosition - _BANDWIDTH);
  if (PresentPosition01 != (constrain(PresentPosition01, _MINPOSITION, _MAXPOSITION)) )
  { // VL6108X is out of Range ... ?
    LOG(">> ERROR << -> VL6108X Out of Range");
    return false; 
  }
  if (!(PresentPosition01 > PresentPosition02))
    { 
    LOG(">> ERROR << -> VL6108X did not detect a DOWN movement");
    return false;
    }
  // AND VL6108X is properly moving DOWN ! --------------------------------------
  LOG("VL6180X and motor properly working ...");
  return true;
}


int Lifter::GetOffsetPosition()
{
  _CurrentPosition = GetVL6180X_Range_Reading();
  int16_t _PositionOffset = _TargetPosition - _CurrentPosition;
  if (sensor->timeoutOccurred()) 
    {
      LOG("VL6180X persists in TIMEOUT error state!");
      return 3;
    }
#ifdef MOVEMENTDEBUG
  LOG("Target: %d  Current %d  Offset: %d", _TargetPosition, _CurrentPosition, _PositionOffset);
#endif 
  if ( (_PositionOffset >= -_BANDWIDTH) && (_PositionOffset <= _BANDWIDTH) )
    { // postion = 0 + or - BANDWIDTH so don't move anymore!
#ifdef MOVEMENTDEBUG
    LOG(" offset = 0 (within bandwidth %1d) ", _BANDWIDTH);
#endif
    return 0; 
    }
  if ( _PositionOffset < 0 )
  {
#ifdef MOVEMENTDEBUG
   LOG(" offset < 0 ");
#endif
   return 1;    
  }
  else 
  {
#ifdef MOVEMENTDEBUG
   LOG(" offset > 0 ");
#endif
   return 2;     
  }
  // default --> error... stop!
#ifdef MOVEMENTDEBUG
  LOG(" BRAKE --> Offset comparison error!");
#endif
  return 0; 
}

void Lifter::SetTargetPosition(int16_t Tpos)
{
  xSemaphoreTake(xSemaphore, portMAX_DELAY); 
  _TargetPosition = Tpos;
  xSemaphoreGive(xSemaphore);
}

void Lifter::moveActuatorUp()
  { 
  // FORWARD
  if (_CurrentPosition <= (_MINPOSITION + _BANDWIDTH) )
    { // Stop further movement to avoid destruction...
    _IsMovingUp = false;
#ifdef MOVEMENTDEBUG
    LOG(" Stop MovingUp ");
#endif
    brakeActuator();
    return;
    }
    // DO NOT REPEATEDLY set the same motor direction 
    if (_IsMovingUp) {return;}
    
    digitalWrite(_actuatorOutPin1, LOW);                           
    digitalWrite(_actuatorOutPin2, HIGH);
    _IsMovingUp = true;
    _IsMovingDown = false;
    _IsBrakeOn = false;
#ifdef MOVEMENTDEBUG
    LOG(" Set MovingUp ");
#endif
  }

void Lifter::moveActuatorDown()
  { 
  // REVERSE
  if (_CurrentPosition >= (_MAXPOSITION - _BANDWIDTH) )
    { // Stop further movement to avoid destruction...
    _IsMovingDown = false;
#ifdef MOVEMENTDEBUG
    LOG(" Stop MovingDown ");
#endif
    brakeActuator();
    return;
    }
    // DO NOT REPEATEDLY set the same motor direction
    if (_IsMovingDown) {return;}
    
    // moving in the wrong direction or not moving at all
    digitalWrite(_actuatorOutPin1, HIGH);                           
    digitalWrite(_actuatorOutPin2, LOW);
    _IsMovingDown = true;
    _IsMovingUp = false;
    _IsBrakeOn = false;
#ifdef MOVEMENTDEBUG
    LOG(" Set MovingDown ");
#endif
  }

void Lifter::brakeActuator()
  { 
    // BRAKE
    // DO NOT REPEATEDLY stop the motor
    if (_IsBrakeOn) {return;}
    digitalWrite(_actuatorOutPin1, LOW);
    digitalWrite(_actuatorOutPin2, LOW);
    _IsBrakeOn = true;
    _IsMovingDown = false;
    _IsMovingUp = false; 
    // Consolidate present position
    Fill_Moving_Average_Filter();
#ifdef MOVEMENTDEBUG
    LOG(" Set Brake On ");
#endif
  }

void Lifter::xTaskControl(void) {
  // Check "continuously" the Actuator Position and move Motor Up/Down until target position is reached
  static int OnOffsetAction = 0;
  const TickType_t xDelay = 110 / portTICK_PERIOD_MS; // Block for 110ms < 10Hz sample rate of VL6180X
  while(1) {
    if(xSemaphoreTake(xSemaphore, portMAX_DELAY)) {
        // BLE channels can interrupt and consequently target position changes on-the-fly !!
        // We do not want changes in TargetPosition during one of the following action!!!
        OnOffsetAction = Lifter::GetOffsetPosition(); // calculate offset to target and determine action
        xSemaphoreGive(xSemaphore);
    }
    switch (OnOffsetAction)
            {
              case 0 :
                Lifter::brakeActuator();
#ifdef MOVEMENTDEBUG
                LOG(" -> Brake");
#endif
                break;
              case 1 :
                Lifter::moveActuatorUp();
#ifdef MOVEMENTDEBUG
                LOG(" -> Upward");
#endif
                break;
              case 2 :
                Lifter::moveActuatorDown();
#ifdef MOVEMENTDEBUG
                LOG(" -> Downward");
#endif
                break;
              case 3 :
                // Timeout --> OffsetPosition is undetermined --> do nothing and brake
                Lifter::brakeActuator();
#ifdef MOVEMENTDEBUG
                LOG(" -> Timeout");
#endif
                break;
            } // switch 
    vTaskDelay(xDelay);
  } // while
} // end


// Static defined member for wrapping non-static member xTaskControl
void Lifter::startTaskControl(void* _this) {
    static_cast<Lifter*>(_this)->xTaskControl();
}

void Lifter::StartControl(const BaseType_t xTaskCoreID) {
     xSemaphore = xSemaphoreCreateBinary();
     xTaskCreatePinnedToCore(this->startTaskControl, "xTaskControl", 4096, this, 10, &ControlTaskHandle, xTaskCoreID);
     xSemaphoreGive(xSemaphore);
}
