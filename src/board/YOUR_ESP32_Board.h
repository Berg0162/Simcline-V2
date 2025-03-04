#ifndef YOURESP32BOARD_H_
#define YOURESP32BOARD_H_

/*
Board Description....

For how Arduino IDE 2.x handles the specific board pin assignments, 
https://github.com/espressif/arduino-esp32/blob/master/variants/<board_name>/pins_arduino.h
*/

static const char* BOARD_NAME = "ESP32 Family Device";

#define PIN_ACTUATOR_1 	-1  // PIN to be connected to pin IN2 of the Adafruit DRV8871 Motor Driver board
#define PIN_ACTUATOR_2 	-1  // PIN to be connected to pin IN1 of the Adafruit DRV8871 Motor Driver board

// Optional Button 1 and 2 PIN connections
//#define PIN_BUTTON_1  -1  // PIN to be connected to Internal/External button 
//#define PIN_BUTTON_2  -1  // PIN to be connected to Internal/External button 

#define PIN_I2C_SCL    	SCL // Default in Arduino IDE -> Check for your board!
#define PIN_I2C_SDA    	SDA // Default in Arduino IDE -> Check for your board!

#include "IBoard.h"

class ESP32Board : public IBoard {
	private:

	public:
	ESP32Board() {}

	// Declaration of board-specific setup member
	void setup(void) override {

		// to be determined for specific board setup

  		// Setup the optional buttons on the appropriate pins
		#if defined(PIN_BUTTON_1) && defined(PIN_BUTTON_2)
  		pinMode(PIN_BUTTON_1, INPUT_PULLUP);
  		pinMode(PIN_BUTTON_2, INPUT_PULLUP);
		#endif

  		// Setup Actuator control pins
  		pinMode(PIN_ACTUATOR_1, OUTPUT);
  		pinMode(PIN_ACTUATOR_2, OUTPUT);
		// Set Actuator default to brake
  		digitalWrite(PIN_ACTUATOR_1, LOW);
  		digitalWrite(PIN_ACTUATOR_2, LOW);
	}
};

#endif // YOURESP32BOARD_H_
