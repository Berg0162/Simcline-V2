#ifndef CONFIGDISPLAY_H_
#define CONFIGDISPLAY_H_

// ------------------------------------------------------------------------------------------------
// Define the display, that is part of your system setup
//#define NODISPLAY
#define OLEDSSD1306_128x64
//#define LILYGO_T_DISPLAY_S3
//#define YOURDISPLAY
//#define SERIALDISPLAY
// ------------------------------------------------------------------------------------------------

// ------------------------------------------------------------------------------------------------
#if defined(USELOCALDISPLAYFILE)  // For restricted use only (!) with "Test_Board_plus_Display.ino"
// ------------------------------------------------------------------------------------------------
// Apply the local "YOURDisplay.h" file
#include "YOURDisplay.h"  
   	 
// ------------------------------------------------------------------------------------------------
#elif defined(NODISPLAY)
// ------------------------------------------------------------------------------------------------
// Include when NO display is connected to the ESP32 board
#include <display/NODisplay.h>

// ------------------------------------------------------------------------------------------------
#elif defined(LILYGO_T_DISPLAY_S3) // When defined it comes with its own display -> NO choice!!
// ------------------------------------------------------------------------------------------------
// Include integrated T-Display of TTGO board
#include <display/TTGOTDisplay.h>

// ------------------------------------------------------------------------------------------------
#elif defined(OLEDSSD1306_128x64)
// ------------------------------------------------------------------------------------------------
// Include SSD1306 Oled display over SPI or I2C connected to the board
#include <display/OLEDDisplay.h>

// ------------------------------------------------------------------------------------------------
#elif defined(YOURDISPLAY)
// ------------------------------------------------------------------------------------------------
// Include your specific display connected to the ESP32 board
#include <display/YOURDisplay.h>

// ------------------------------------------------------------------------------------------------
#elif defined(SERIALDISPLAY)
// ------------------------------------------------------------------------------------------------
// Include SERIALdisplay configured to redirect all output to Serial Monitor
#include <display/SERIALDisplay.h>

// ------------------------------------------------------------------------------------------------
#else 				   // No selection made at all
// ------------------------------------------------------------------------------------------------
#error "Please define a display first"
#endif

#endif // CONFIGDISPLAY_H_