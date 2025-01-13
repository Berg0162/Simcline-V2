# configBoard.h
Simcline-V2 <b>Board</b> configuration options:
```C++
// Define the board here, that is part of your system setup
//#define LILYGO_T_DISPLAY_S3
//#define ADAFRUIT_FEATHER_ESP32_V2
#define YOUR_ESP32_BOARD
```
# configDisplay.h
Simcline-V2 <b>Display</b> configuration options:
```C++
// Define the display, that is part of your system setup
#define NODISPLAY
//#define OLEDSSD1306_128x64
//#define LILYGO_T_DISPLAY_S3
//#define YOURDISPLAY
//#define SERIALDISPLAY
```

# configNimBLE.h
Simcline-V2 <b>NimBLE</b> configuration options:
```C++
// ----------------------------------------------------------------------------------------
// Uncomment to allow/enable the MITM transfer of FiTness Machine Service data from trainer 
// to training App directly. Bluetooth Smart FTMS: This is the industry standard for apps 
// controlling the trainer via Bluetooth Smart, and includes power and cadence baked in.
//		  NOTICE --> NOT all (legacy) trainers support FTMS!!!
#define ENABLE_FTMS

// ----------------------------------------------------------------------------------------
// Uncomment to allow/enable the MITM transfer of Cycling Speed and Cadence data from trainer 
// to training App directly. Useful when trainer allows direct pairing of builtin CSC sensor, 
//		  NOTICE --> NOT all (legacy) trainers support CSC!!!
#define ENABLE_CSC

// ----------------------------------------------------------------------------------------
// Uncomment to allow/enable the MITM transfer of Heart Rate Measurement data from trainer 
// to training App directly. Useful when trainer allows direct pairing of your HRM band, 
//		  NOTICE --> ONLY Zwift Hub and Jetblack trainers support HRM baked in!!!
//#define ENABLE_HRM

// ----------------------------------------------------------------------------------------
// Uncomment to allow/enable the MITM transfer of Wahoo Trainer Control data from trainer 
// to training App directly. 
// Bluetooth Smart Wahoo Trainer Control: This is Wahoo’s (legacy) proprietary method of 
// controlling trainers, and includes speed/cadence data – still supported by Zwift.
//		  NOTICE --> Most pre-2020 Wahoo trainers support this instead of FTMS!!!
//#define ENABLE_WAHOOCPS
```

# configSimcline.h
Simcline-V2 <b>Mechanical SIMCLINE 2.0</b> configuration options:
```C++
//------------- Range Control definitions for high level movement control -----------------------------------------------
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

// -------------------------- WARNING ------------------------------------------------------------
// Operational boundaries of the VL6180X sensor are used/set in class Lifter after calling its "init".
// A safe measuring range of at least 30 cm of total movement is recommended for the VL6180X sensor setting!
//
// Bandwidth is used in the code to take measuring errors and a safe margin into account when reaching
// the above defined max or min positions of the construction! The software does painstakingly respect
// these and is independent of the appropriate working of the microswitches when reaching the boundaries!
// These microswitches are a SECOND line of defence against out of range and potentially damaging movement!
#define BANDWIDTH 4 
```

# configDebug.h
Simcline-V2 has a fine-grained scheme for allowing debug-messages during operation. Every /src/<name>.cpp file has its own <b>#define Debug</b> at the beginning of the code that can be (un)commented (switched on or off) discretionary. However, when one reaches the state that debugging has no longer a purpose, you want to get rid of all the overhead (processor load) and optimize for speed. This config file allows you to switch OFF in one place all Simcline-V2 debugging messages with one <b>master switch</b><br>
Simcline-V2 <b>Debug</b> configuration options:
```C++
// ------------------------------------------------------------------------------------------------
// COMPILER DIRECTIVE to allow/suppress ALL DEBUG messages that help to debug code sections
// Uncomment general "#define GLOBAL_DEBUG" to activate the Simcline-V2 debug master switch...
#define GLOBAL_DEBUG

```
