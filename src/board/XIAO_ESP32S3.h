#ifndef XIAOESP32S3_H_
#define XIAOESP32S3_H_

/* 
Seeed Studio XIAO Series are diminutive development boards, sharing a similar hardware structure, 
where the size is literally thumb-sized. The code name "XIAO" here represents its half feature "Tiny", 
and the other half will be "Puissant". Seeed Studio XIAO ESP32S3, CPU: ESP32-S3R8, Xtensa LX7 dual-core, 
32-bit processor that operates at up to 240 MHz and with On-chip 8M PSRAM & 8MB Flash. 
See: https://wiki.seeedstudio.com/xiao_esp32s3_getting_started/
For how Arduino IDE 2.x handles the specific board pin assignments, 
https://github.com/espressif/arduino-esp32/blob/master/variants/XIAO_ESP32S3/pins_arduino.h
*/

static const char* BOARD_NAME = "Seeed Studio XIAO ESP32S3";

#define PIN_ACTUATOR_1 	A0   // GPIO1 Connected to pin IN2 of the Adafruit DRV8871 Motor Driver board
#define PIN_ACTUATOR_2 	A1   // GPIO2 Connected to pin IN1 of the Adafruit DRV8871 Motor Driver board

// Optional Button 1 and 2 PIN connections
//#define PIN_BUTTON_1  A2   // GPIO3 External button 
//#define PIN_BUTTON_2  A3   // GPIO4 External button 

// I2C definitions and PIN connections
#define PIN_I2C_SCL    	SCL  // GPIO6
#define PIN_I2C_SDA    	SDA  // GPIO5 

// SPI definitions and PIN connections
#define PIN_SPI_MOSI 	MOSI // GPIO9
#define PIN_SPI_MISO 	MISO // GPIO8
#define PIN_SPI_CLK  	SCK  // GPIO7
#define PIN_SPI_CS   	44   // GPIO44
#define PIN_SPI_DC	43   // GPIO43
#define PIN_SPI_RES	-1   // NOT connected to a GPIO

#include "IBoard.h"

class ESP32Board : public IBoard {
	private:

	public:
	ESP32Board() {}

	// Declaration of board-specific setup member
	void setup(void) {

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

#endif // XIAOESP32S3_H_
