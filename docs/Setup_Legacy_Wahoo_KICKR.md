

# configNimBLE.h
Make configNimBLE.h settings to comply with your <b>Legacy Wahoo KICKR</b> trainer.

+ Open file, edit and save(!): /documents/arduino/libraries/Simcline-V2/src/config/configNimBLE.h

+ Set Simcline-V2 <b>NimBLE</b> configuration for use of <b>Legacy Wahoo KICKR</b>:
```C++
// ----------------------------------------------------------------------------------------
// Uncomment to allow/enable the MITM transfer of FiTness Machine Service data from trainer 
// to training App directly. Bluetooth Smart FTMS: This is the industry standard for apps 
// controlling the trainer via Bluetooth Smart, and includes power and cadence baked in.
//		  NOTICE --> NOT all (legacy) trainers support FTMS!!!
//#define ENABLE_FTMS

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
#define ENABLE_WAHOOCPS
```
