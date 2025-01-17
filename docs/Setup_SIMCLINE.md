
# Setup SIMCLINE following these steps:

# Completely independent of the Simcline-V2 library 
First test the ESP32 board of your choice with the display attached to it, after you have installed Arduino IDE 2 and the appropriate graphics libraries (like Adafruit GFX and/or TFT_eSPI). Notice that you have to install the original libraries and not the modified versions that product suppliers like to include! All graphics libraries come with code examples and test these. For TFT_eSPI you need to set your display in "User_Setup_Select.h" <b>first</b> to comply with the board/display type you use. Read and study the instructions that come with the sites of these libraries! Only when you have a working combination continue...

# Setup your board for Simcline-V2
The board and display to work with can be activated in Simcline-V2 with the right settings, see folder: `/Simcline-V2/src/config`<br>

+ Open file, edit and save(!): `/documents/arduino/libraries/Simcline-V2/src/config/configBoard.h`
```C++
// ------------------------------------------------------------------------------------------------
// Define the board here, that is part of your system setup
//#define LILYGO_T_DISPLAY_S3
#define ADAFRUIT_FEATHER_ESP32_V2
//#define YOUR_ESP32_BOARD
```
+ Open file, edit and save(!): `/documents/arduino/libraries/Simcline-V2/src/config/configDisplay.h`<br>

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
+ When you want another board or display than is supplied as standard with Simcline-V2, a proficient programmer/user can implement Concrete Classes for ESP32-board-type and Display-type of his/her choice easily without interfering with the Simcline BLE operational code. A dedicated program for creating "YOURDISPLAY" is offered in `/Simcline-V2/examples/Test_Board_plus_Display` to help you with the task!
+ <b>Warning</b>: When a new version of Simcline-V2 is installed in Arduino IDE 2 it will override <b>ALL</b> files of a previous version! If you have made modifications in a file that is part of `Simcline-V2` --> <b>Make</b> a <b>copy</b> of the file(s) in question <b>BEFORE</b> you <b>install</b> a new library version!

# Attach the TOF-sensor to your ESP32 board and test
First install the <b>VL6180X</b> library in Arduino IDE 2 and the example programs that come with it. Connect using the prescribed wiring scheme depending on the board that you earlier selected: 
[Adafruit Feather ESP32 V2](https://github.com/Berg0162/Simcline-V2/blob/main/docs/Adafruit%20Feather%20ESP32-V2.md) or [Lilygo esp32s3 T-Display](https://github.com/Berg0162/Simcline-V2/blob/main/docs/LILYGO%20ESP32S3%20T-Display.md) <br>
Test with the supplier code examples and finally run `/Simcline-V2/examples/Test_Board_plus_VL6180X` to test in the Simcline-V2 environment.

# Attach the Motor Driver board
No library needed for <b>DRV8871</b>, but the supplier will have example code supplied. 

# Attach the Actuator to the Motor Driver board and test 
Test the <b>DRV8871 plus Actuator</b> with `/Simcline-V2/examples/Test_Board_plus_DRV8871`.

# Setup and mount Simcline mechanically...
Actuator attached to the body

# Add the TOF-sensor to the mechanic setup (in the component box)
Check it can measure the distance correctly using `/Simcline-V2/examples/Test_Board_plus_VL6180X`<br>

Note down the provisional <b>MAXPOSITION</b> and <b>MINPOSITION</b> values of the TOF-sensor reaching the <b>top</b> position and <b>lowest</b> position!<br>

Open file, edit and save(!): `/documents/arduino/libraries/Simcline-V2/src/config/configSimcline.h`<br>

Insert <b>RGVMIN</b>, <b>RGVMAX</b>, <b>MINPOSITION</b> and <b>MAXPOSITION</b> values in <b>configSimcline.h</b> in accordance with your mechanical setup.

```C++
//----------- Global variable definitions for high level movement control -----------------------------------------------
// In theory the RawgradeValue varies between 0 (equals -200% grade) and 40000 (equals +200% grade)
// SIMCLINE is mechanically working between -10% and +20% --> 19000 and 22000

//------------------------------------------------- WARNING --------------------------------------------------------------
//------------ SET THESE TWO VALUES IN ACCORDANCE WITH THE MECHANICAL RANGE LIMITATIONS OF YOUR SIMCLINE !!! -------------
// Raw Grade Value Minimally (Mechanically: the lowest position of wheel axis)  19000 is equiv. of 10% downhill road grade
#define RGVMIN 19500 // -5%  // Always is RGVMIN < 20000 (flat road level)
// Raw Grade Value Maximally (Mechanically: the highest position of wheel axis) 22000 is equiv. of 20% uphill road grade
#define RGVMAX 22000 // 20%  // +20% // Always is RGVMAX > 20000 (flat road level)
//------------------------------------------------- WARNING --------------------------------------------------------------

// Correction for measuring plane difference and midth wheel axis position (1 cm offset is an MEASUREOFFSET of about 40)
#define MEASUREOFFSET 0

// -------------------------- WARNING ------------------------------------------------------------
// The following VL6180X sensor values are a 100% construction specific and
// should be experimentally determined, when the Actuator AND the VL6180X sensor are mounted!
// ------>>>> Test manually and use example/test sketches that go with the VL6180X sensor! <<<<---
// Microswitches should limit physically/mechanically the upper and lower position of the Actuator!
// The microswitches are mechanically controlled, and NOT by the software --> should be fail safe!
// Notice that unrestricted movement at the boundaries can damage the Actuator and/or construction!
// The following values are respected by the software and will (in normal cases!) never be exceeded!
#define MINPOSITION 10  // VL6180X highest value top microswitch activated to mechanically stop operation
#define MAXPOSITION 195 // VL6180X lowest value bottom microswitch activated to mechanically stop operation
```
Now you should have a fully equiped and operational SIMCLINE 2 (TOF-sensor/display/driver/Actuator) with hardware components tested separately!<br>
It is time to run `/Simcline-V2/examples/Test_Fully_Equiped_Setup`.<br>
During operation of this Full-Setup test-program you will have most probably to finetune the <b>MINPOSITION</b> and <b>MAXPOSITION</b>. Repeat, if necessary, by editing `/documents/arduino/libraries/Simcline-V2/src/config/configSimcline.h` ... Don't forget to save the file before (!) you run the Full-Setup test-program again, otherwise the new values will not be effective!!

# Open Simcline program, compile and run! 
SIMCLINE should recognise the TOF-sensor, motor driver board and test the actuator movements with help of the TOF-sensor. It will scan for your trainer and advertise its server-side to be connected to Zwift... from now on it will handle the Bluetooth MITM and translate changing road grades to actuator movements.<br>
See: [how road grades are handled FAQ #8](Frequently_Asked_Questions.md#8)<br>
See: [when devices have changed FAQ #9](Frequently_Asked_Questions.md#9)<br>


