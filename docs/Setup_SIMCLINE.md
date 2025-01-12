
# Setup SIMCLINE following these steps:

# Completely independent of the Simcline-V2 library 
First test the ESP32 board of your choice with the display attached to it, after you have installed Arduino IDE 2 and the appropriate graphics libraries (like Adafruit GFX and/or TFT_eSPI). Notice that you have to install the original libraries and not the modified versions that product suppliers like to include! All graphics libraries come with code examples and test these. For TFT_eSPI you need to set your display in "User_Setup_Select.h" <b>first</b> to comply with the board/display type you use. Read and study the instructions that come with the sites of these libraries! Only when you have a working combination continue...

# Setup your board for Simcline-V2
The board and display to work with can be activated in Simcline-V2 with the right settings, see folder: <b>/Simcline-V2/src/config</b><br>
+ Open file, edit and save(!): <b>/documents/arduino/libraries/Simcline-V2/src/config/configBoard.h</b>
```C++
// ------------------------------------------------------------------------------------------------
// Define the board here, that is part of your system setup
//#define LILYGO_T_DISPLAY_S3
#define ADAFRUIT_FEATHER_ESP32_V2
//#define YOUR_ESP32_BOARD
```
+ Open file, edit and save(!): <b>/documents/arduino/libraries/Simcline-V2/src/config/configDisplay.h</b><br>
```C++
// ------------------------------------------------------------------------------------------------
// Define the display, that is part of your system setup
//#define NODISPLAY
#define OLEDSSD1306_128x64
//#define LILYGO_T_DISPLAY_S3
//#define YOURDISPLAY
//#define SERIALDISPLAY
// ------------------------------------------------------------------------------------------------
```
+ When you want another board or display than is supplied as standard with Simcline-V2, a proficient programmer/user can implement Concrete Classes for ESP32-board-type and Display-type of his/her choice easily without interfering with the Simcline BLE operational code. A dedicated program for creating "YOURDISPLAY" is offered in /Simcline-V2/examples/Test_Board_plus_Display to help you with the task!

# Attach the TOF-sensor to your ESP32 board and test
First install the <b>VL6180X</b> library in Arduino IDE 2 and the example programs that come with it. Connect using the prescribed wiring scheme depending on the board that you earlier selected. Test with supplier code examples and the Simcline test samples in the Arduino folder.



5) Attach the Motor Driver board and test. No library needed, but the supplier will have example code supplied. Use the Simcline test samples in the Arduino folder.



6) Attach the Actuator to the Motor Driver board and test the Actuator....



7) Setup and mount Simcline mechanically...



8) Add the TOF-sensor to the mechanic setup and check it can measure the distance correctly. Repeat 3) for testing. Note down the MAX and MIN values of the TOF-sensor reaching the the top position and lowest position!



9) Insert the settings that you have noted down before in the Simcline Diagnostics Test code. Upload and run to see if all components work together properly.



10) Insert the settings that you have noted down before in the FTMS_Simcline code, compile and upload! Simcline should recognise the TOF-sensor, motor driver board and test the actuator movements + TOF-sensor readings. If successful, it will scan for the trainer (+ connect) and advertise/connect with the Laptop (Zwift).

