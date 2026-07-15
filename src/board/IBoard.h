#ifndef IBOARD_H_
#define IBOARD_H_

#include <Esp.h>

class IBoard {
  public:
    virtual ~IBoard() = default;

    virtual void setup() = 0;

    void initUSB(long rate) {
#ifdef DEBUG
	// Initialize USB connection to the computer to catch debug messages
        Serial.setRxBufferSize(256); // Increase RX buffer size
        Serial.begin(rate);

        while (!Serial) delay(10); 
        Serial.flush();
        delay(800); // Give Serial I/O time to settle
#endif
    };

#ifdef DEBUG 
    void scanForDevices(void) {
  	byte error, address;
  	uint8_t nDevices = 0;
  	for(address = 1; address < 127; address++ ) {
    		// The scanner uses the return value of the Wire.endTransmisstion to see if
    		// a device did acknowledge to the address.
    		Wire.beginTransmission(address);
    		error = Wire.endTransmission();
    		if(error == 0) {
      			LOG("I2C device at address 0x%X", address);
      			nDevices++;
    		} else if(error == 4) {
      			LOG("Unknown error at address 0x%X", address);
    		}    
  	}
  	if(nDevices == 0) Serial.println("No I2C devices found!");
    };
#endif

    void initWire(bool test) {
  	// Set I2C SDA and SCL for TOF sensor (VL6180X)
  	if( Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL) ) LOG("Wire (I2C) Initialized!");
	else LOG("Initialize Wire (I2C) Failed!");
#ifdef DEBUG
	if(test) scanForDevices();
#endif
    };

    void info() { 
#ifdef DEBUG   
  	Serial.print("ESP-IDF version "); Serial.println(esp_get_idf_version());
  	Serial.print("Arduino Core version "); Serial.print(ESP_ARDUINO_VERSION_MAJOR); Serial.print(".");
 	Serial.print(ESP_ARDUINO_VERSION_MINOR); Serial.print("."); Serial.println(ESP_ARDUINO_VERSION_PATCH);
  	Serial.print("Model "); Serial.print(ESP.getChipModel()); Serial.print(" revision "); Serial.println(ESP.getChipRevision());
  	Serial.print("Cores "); Serial.println(ESP.getChipCores());
	Serial.print("CPU   "); Serial.print(ESP.getCpuFreqMHz()); Serial.println(" MHz");
  	Serial.print("Flash "); Serial.print(ESP.getFlashChipSpeed()/1000000); Serial.println(" MHz");
  	Serial.print("PSRAM "); Serial.print(ESP.getPsramSize()/1000000); Serial.println(" MB");
  	Serial.print("Flash "); Serial.print(ESP.getFlashChipSize()/1000000); Serial.println(" MB");
#endif
    };
};

#endif // IBOARD_H_
