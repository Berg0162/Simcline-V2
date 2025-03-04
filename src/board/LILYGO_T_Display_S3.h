#ifndef LILYGOTDISPLAYS3_H_
#define LILYGOTDISPLAYS3_H_

/*
LilyGo T-Display-S3 is a development board whose main control chip is ESP32-S3. 
It is equipped with a 1.9-inch LCD color screen, two programmable buttons and
Flash: 16MB plus PSRAM: 8MB. See: https://www.lilygo.cc/products/t-display-s3

**************************************************************************************************
Select in board drop down -> ESP32S3 Dev Module and set Menu -> Tools to the correct settings !!
**************************************************************************************************

**************************************************************************************************
>>>> DO NOT select in board drop down -> lilygo_t_display_s3 to let Arduino IDE 2.x handle it <<<<
**************************************************************************************************

Board pin assignments in the list below handle physical GPIO pin assignments, since Arduino IDE
default labelling is NOT used !!
*/

static const char* BOARD_NAME = "LilyGo T-Display S3";

#define PIN_BACKLIGHT 	  38 // LCD_BL 		// GPIO38

// Warning I/O Pins can have identical board position but different I/O Pin declarations
#define PIN_ACTUATOR_1 	  43 // GPIO43 -> connected to pin IN2 of the Adafruit DRV8871 Motor Driver board
#define PIN_ACTUATOR_2 	  44 // GPIO44 -> connected to pin IN1 of the Adafruit DRV8871 Motor Driver board

#define PIN_POWER_ON      15 // LCD_POWER_ON 	// GPIO15

// LilyGo T-display has 2 user buttons on the board
#define PIN_BUTTON_1      0  // BUTTON_1 	// GPIO0  Internal button 1
#define PIN_BUTTON_2      14 // BUTTON_2 	// GPIO14 Internal button 2

#define PIN_BAT_VOLT      4  // BAT_VOLT 	// GPIO 4

#define PIN_I2C_SCL       17 // SCL 		// GPIO 17
#define PIN_I2C_SDA       18 // SDA 		// GPIO 18

#include "IBoard.h"

class ESP32Board : public IBoard {
	private:

	public:
	ESP32Board() {}

	// Declaration of board-specific setup function
	void setup(void) override { 
	
  		// Pin_Power_On needs to be set to HIGH in order to boot without USB connection
  		pinMode(PIN_POWER_ON, OUTPUT);     // to boot with battery...
  		digitalWrite(PIN_POWER_ON, HIGH);  // and/or power from 5v rail instead of USB

  		// Setup the buttons on the appropriate pins
  		pinMode(PIN_BUTTON_1, INPUT_PULLUP);
  		pinMode(PIN_BUTTON_2, INPUT_PULLUP);

  		// Setup Actuator control pins
  		pinMode(PIN_ACTUATOR_1, OUTPUT);
  		pinMode(PIN_ACTUATOR_2, OUTPUT);
		// Set Actuator default to brake
  		digitalWrite(PIN_ACTUATOR_1, LOW);
  		digitalWrite(PIN_ACTUATOR_2, LOW);
	}
}; 

#endif // LILYGOTDISPLAYS3_H_
