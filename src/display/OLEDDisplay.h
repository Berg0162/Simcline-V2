#ifndef OLEDDISPLAY_H_
#define OLEDDISPLAY_H_

#include "IDisplay.h"

/* 
--------------------------------------------------------------------

     Oled 1306 128*64 monochrome SPI and I2C versions supported!!

--------------------------------------------------------------------
*/ 

// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow a SSD1306 Oled Display to be connected over SPI or I2C...
// Uncomment general "#define SPI_CONNECT" to activate a connection over SPI, default is I2C connection
//#define SPI_CONNECT

#ifdef SPI_CONNECT
#include <SPI.h>
static const char* IDISPLAY = "OLED SSD1306 128*64 SPI";
#else 
// Notice that Wire.h (I2C) is initialized in configBoard.h for use with the ToF sensor!!
static const char* IDISPLAY = "OLED SSD1306 128*64 I2C";
#define OLED_RESET -1               // No reset pin on this OLED display
#define OLED_I2C_ADDRESS 0x3C       // I2C Address of OLED display
#endif

#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
// Additional splash screen bitmap and icon(s) for Oled
#include "Adafruit_SSD1306_Icons.h" // needs to be in the display directory

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels

#ifdef SPI_CONNECT
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &SPI, PIN_SPI_DC, PIN_SPI_RES, PIN_SPI_CS); 
#else
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#endif

class OLEDDisplay : public IDisplay {
  private:
  bool fullClear = true;

  public:
    OLEDDisplay() {}

    void initDisplay() override {
#ifdef SPI_CONNECT
        if(!display.begin(SSD1306_SWITCHCAPVCC)) 
#else
	if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDRESS))
#endif
	{
            Serial.println("SSD1306 allocation failed");
            for(;;); // Hang
        } else  {
            // Show initial display buffer contents with the screen ADAFRUIT Splash screen
            // the library initializes this (edit the splash.h in the library).
            display.display();
            delay(500); // Pause for some time
        }
        // Ready to show our own SIMCLINE splash screen
        display.clearDisplay(); // clean the oled screen
        display.setTextColor(SSD1306_WHITE);
        display.drawBitmap(24, 0, Mountain_bw_79x64, 79, 64, 1);
        display.display();
    }

    void setDisplayBrightness(bool updown) override {
        // Implementation for OLED display
        // Only 2 levels: false to enable full brightness, true for a lower brightness
        display.dim(updown); 
    }

    void ShowIconsOnTopBar(bool isTrainer, bool isLaptop, bool isPhone) override {  
        // Implementation for OLED display
        static bool blinkState = true;
        // Show BLE icon during BLE advertising and/or BLE scanning 
        if(!isTrainer || !(isLaptop ^ isPhone)) { 
            // Blink BLE icon when not all devices are connected
            if(blinkState) display.drawBitmap(0, 0, bluetooth_icon16x16, 16, 16, SSD1306_WHITE);
            else display.drawBitmap(0, 0, bluetooth_icon16x16, 16, 16, SSD1306_INVERSE);
            blinkState = !blinkState;
        }
        // Show Icons on Top Bar
        if (isTrainer) display.drawBitmap(112, 0, power_icon16x16, 16, 16, SSD1306_WHITE);
        else display.drawBitmap(112, 0, power_icon16x16, 16, 16, SSD1306_BLACK); // Clear this Icon field
        if (isLaptop) display.drawBitmap(0, 0, zwift_icon16x16, 16, 16, SSD1306_WHITE);
        if (isPhone) display.drawBitmap(0, 0, mobile_icon16x16, 16, 16, SSD1306_WHITE);
        display.display();
    }

    void ShowMessageWindow(const String& Line1, const String& Line2, const String& Line3, uint16_t Pause) override {
        // Implementation for OLED display
        // Clear and set Oled to display 3 line messages -> centered
        int pos = 1;
        display.clearDisplay();
        display.setTextColor(SSD1306_WHITE);
        display.setTextSize(2);  // Large characters 11 pixels wide
        if (Line1) {
            pos = round( (127 - (12 * Line1.length())) / 2 );
            display.setCursor(pos, 2); 
            display.print(Line1);
        }
        if (Line2) {
            pos = round( (127 - (12 * Line2.length())) / 2 );
            display.setCursor(pos, 22);
            display.print(Line2);
        }
        if (Line3) {
            pos = round( (127 - (12 * Line3.length())) / 2 );
            display.setCursor(pos, 44);
            display.print(Line3);
        }
        display.drawRect(0,0,127,63,SSD1306_WHITE); // draw window
        display.display();
        if(Pause > 0) vTaskDelay(Pause/portTICK_PERIOD_MS); // Pause indicated time in ms
        fullClear = true; // Signal full display clear!
    }

    void ShowRoadGrade(float gradePerc) override {
        // Implementation for OLED display
        // Clear most of the display to avoid flicker !
        if(fullClear) { // Clear full display area only when strictly needed!
           display.clearDisplay();
           fullClear = false;
        } else { // Clear only the area where RoadGrade dynamic content is displayed!
           display.fillRect(0, 16, 128, 64, SSD1306_BLACK);
        }
        // The following calculations give more "weight" to lower grade values
        // (like: 1.2% or 0.5%), these will occur more often in practice and are not well
        // displayable at 128*64! --> 64 * 64 = 4096 and this value should not be
        // exceeded (4096/20) = 204.8
        int pos = 64 - int(sqrt(abs(204 * gradePerc))); // cast to int to get rid of decimals only now!
        if (gradePerc > 0) {
            display.fillTriangle( 1, 63, 127, 63, 127, pos, SSD1306_INVERSE);
        } else {
            display.fillTriangle( 127, 63, 1, 63, 0, pos, SSD1306_INVERSE);
        }
        if(pos < 17) fullClear = true; // Triangle reaches Icons on Top Bar!
        // Draw the baseline to smooth small decimal values and show flat road case
        display.drawFastHLine(1, 63, 127, SSD1306_WHITE);
        // Draw the road grade (percentage)
        display.setTextColor(SSD1306_INVERSE);
        char tmp[7];
        dtostrf(gradePerc, 5, 1, tmp); // show sign only if negative
        display.setCursor(10, 22); // 6 -> 22
        display.setTextSize(3);
        display.print(tmp);
        display.setCursor(102, 30); //10 -> 30
        display.setTextSize(2);
        display.print("%");

        display.display();
    }
}; // OLEDDisplay class

// Create the Display object of type OLEDDisplay
OLEDDisplay* pDisplay = new OLEDDisplay();

#endif // OLEDDISPLAY_H_
