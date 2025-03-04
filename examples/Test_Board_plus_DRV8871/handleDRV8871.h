
#ifdef MANUAL_INPUT_DATA
char input; // Type 'u' for up movement or 'd' for down movement or '0' (zero) for neutral position
#endif

void moveActuatorUp()
  { // FORWARD
  digitalWrite(PIN_ACTUATOR_1, LOW);                           
  digitalWrite(PIN_ACTUATOR_2, HIGH); 
  LOG("Move Up");
  }

void moveActuatorDown()
  { // REVERSE
  digitalWrite(PIN_ACTUATOR_1, HIGH);                           
  digitalWrite(PIN_ACTUATOR_2, LOW);
  LOG("Move Down");
  }
  
void brakeActuator()
  { // BRAKE
  digitalWrite(PIN_ACTUATOR_1, LOW);
  digitalWrite(PIN_ACTUATOR_2, LOW);
  LOG(" > Brakes On");
  }

void initDRV8871(void) {
#ifdef MANUAL_INPUT_DATA 
  LOG("Type 'u' for UP or 'd' for DOWN movement, and close with <ENTER> to confirm!");
#endif
  //  setup control pins and set brake by default
  pinMode(PIN_ACTUATOR_1, OUTPUT);
  pinMode(PIN_ACTUATOR_2, OUTPUT);
  brakeActuator();
}
void moveActuator(void) 
  { // TEST FUNCTION 
  moveActuatorUp();
  delay(1000);
  brakeActuator();
  delay(1000);
  moveActuatorDown();
  delay(1000);
  brakeActuator();
  delay(2000);
  }
