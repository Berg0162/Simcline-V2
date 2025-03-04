#ifndef LIFTER_H_
#define LIFTER_H_

#include <Arduino.h>
#include <VL6180X.h>
#include "MovingAverageFilter.h"

class Lifter {

private:
  VL6180X* sensor;
  MovingAverageFilter* movingAverageFilter_Range;
  bool _IsBrakeOn;
  bool _IsMovingUp;
  bool _IsMovingDown;
  int _actuatorOutPin1;
  int _actuatorOutPin2;
  int16_t _TargetPosition;
  int16_t _CurrentPosition;
  int _BANDWIDTH;
  int _MINPOSITION;
  int _MAXPOSITION;
  void InitVL6180X(void);
  void Fill_Moving_Average_Filter(void);
  int16_t GetVL6180X_Range_Reading();
  SemaphoreHandle_t xSemaphore;
  TaskHandle_t ControlTaskHandle;
  static void startTaskControl(void* _this);
  void xTaskControl(void);
  
public:
  Lifter();
  ~Lifter();
  boolean Init(void);
  void SetTargetPosition(int16_t Tpos);
  bool TestBasicMotorFunctions();
  int GetOffsetPosition();
  void moveActuatorUp();
  void moveActuatorDown();
  void brakeActuator();

  void StartControl(const BaseType_t);
};
#endif
