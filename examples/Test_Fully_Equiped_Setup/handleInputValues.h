
// -------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress DEBUG messages that help debugging...
// Comment out "#define MANUAL_INPUT_DATA" to activate: input of predetermined test values!
#define MANUAL_INPUT_DATA

// For testing purpose feed with some data manually -----------------------------------------------
#ifdef MANUAL_INPUT_DATA
char input; // Type 'u' for up movement or 'd' for down movement or '0' (zero) for neutral position
#else
uint8_t targetValuesCount = 0;
// Stroke = 30 --> Values between -10.0 and 20.0
//float TargetValuesArray[8] = { 0.0, -5.0, -10.0, 0.0, 10.0, 20.0, 15.0, 10.0 };
// Stroke = 20 --> Values between -5.0 and 15.0
float TargetValuesArray[8] = { 0.0, 5.0, 10.0, 15.0, 10.0, 5.0, 0.0, -5.0 };
#endif
int16_t targetPosition;
float grade = 0.0;
// For testing purpose only -----------------------------------------------------------------------

void getInputValue(void) {
// Get input and set testing target values -----------------------------------
#ifdef MANUAL_INPUT_DATA
    if(Serial.available()){
      input = Serial.read();
      switch (input)
      {
      case 117 : // u
        operations->stepGrade(true);
        LOG("Up");
        break;
      case 100 : // d
        operations->stepGrade(false);
        LOG("Down");
        break;
      case 48 :  // 0
        // Put Simcline in neutral: 0.0 flat road position
        operations->levelGrade();
        LOG("Level");
        break;
      default :
        return;
      }
    } else return; 
#else
    grade = TargetValuesArray[targetValuesCount]; 

    operations->setNewGrade(grade); 

    if(++targetValuesCount == 8) { targetValuesCount = 0; }  
#endif
}
