#ifndef LIFTER_H_
#define LIFTER_H_

#include <Arduino.h>
#include <VL6180X.h>

// Option to select one of two filters (only one should be active)
// Comment/uncomment to choose the filter
#define SMA_FILTER		// simple moving average (SMA) filter
//#define EMA_FILTER		// exponential moving average (EMA) filter

#if defined(SMA_FILTER)
  #include "SMA_Filter.h"
  #undef EMA_FILTER
#elif defined(EMA_FILTER)
  #include "EMA_Filter.h"
#else
  #error "Lifter: No filter selected! Define either SMA_FILTER or EMA_FILTER."
#endif

class Lifter {
private:
  VL6180X* sensor;
#ifdef SMA_FILTER  
  SMA_Filter* SMAFilter;
#endif
#ifdef EMA_FILTER
EMA_Filter* EMAFilter;
#endif

  bool isBrakeOn;
  bool isMovingUp;
  bool isMovingDown;
  int16_t targetPosition;
  int16_t currentPosition;
  void initVL6180X(void);
  void primeFilter(uint8_t maxCount);
  int getOffsetPosition();
  SemaphoreHandle_t xSemaphore;
  TaskHandle_t ControlTaskHandle;
  static void startTaskControl(void* _this);
  void xTaskControl(void);
  
public:
  Lifter();
  ~Lifter();
  bool Init(void);
  int16_t getVL6180XRangeReading(void);
  void SetTargetPosition(int16_t Tpos);
  bool TestBasicMotorFunctions();
  void moveActuatorUp();
  void moveActuatorDown();
  void brakeActuator();
  bool StartControl(const BaseType_t);
};
#endif
