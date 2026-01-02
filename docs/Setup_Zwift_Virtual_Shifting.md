# Setup for Zwift Virtual Shifting (ZVS)

Virtual Shifting has rapidly become a dominant hardware trend for indoor cycling in 2025. While it currently complements—rather than fully replaces—traditional mechanical cassettes, its adoption is accelerating across both trainers and control devices.

## Two Zwift Virtual Shifting Implementations

Zwift Virtual Shifting exists in two distinct BLE service implementations, depending on trainer firmware generation:

1. **Current ZVS Service (2025 firmware)**  
   Modern trainers running the latest firmware expose a dedicated Bluetooth **Zwift Virtual Shifting service**, identified by the 16-bit UUID:  
   `0xFC82`

2. **Legacy ZVS Service (January 2024 firmware)**  
   Some older trainers received an initial Zwift Virtual Shifting implementation via a proprietary 128-bit BLE service, identified by the UUID:  
   `00000001-19ca-4651-86e5-fa29dcdd09d1`

Many trainers that were originally marketed as *“Zwift Ready”* never received the 2025 firmware update and therefore remain limited to the legacy ZVS service—or do not support Virtual Shifting at all.

> **Important:** Simcline-V2 supports both Zwift Virtual Shifting implementations, but you must configure the correct ZVS firmware version for your trainer. Always verify which ZVS service your trainer firmware provides.

> Notice: Trainer age alone is not sufficient — ZVS support depends entirely on the installed firmware.

# configNimBLE.h
Make configNimBLE.h settings to comply with your **ZVS Ready** trainer.

+ Open file, edit and save(!): `/documents/arduino/libraries/Simcline-V2/src/config/configNimBLE.h`

+ Set Simcline-V2 <b>NimBLE</b> configuration for use of <b>Virtual Shifting</b>:
```C++
// ----------------------------------------------------------------------------------------
// Uncomment to allow/enable the MITM transfer of FiTness Machine Service data from trainer 
// to training App directly. Bluetooth Smart FTMS: This is the industry standard for apps 
// controlling the trainer via Bluetooth Smart, and includes power and cadence baked in.
//		  NOTICE --> NOT all (legacy) trainers support FTMS!!!
//#define ENABLE_FTMS

// ----------------------------------------------------------------------------------------
// Uncomment to allow/enable the MITM transfer of Zwift Virtual Shifting (ZVS) data between 
// trainer and training App. ZVS is a proprietary Zwift BLE service used for virtual gear 
// shifting and trainer control instead of physically moving your chain across cogs.
//		  NOTICE --> Requires a trainer with firmware that explicitly supports ZVS!
#define ENABLE_ZVS
#if (defined(ENABLE_ZVS))
// Uncomment to allow/enable the Legacy ZVS Service for your trainer
//#define TRAINER_WITH_LEGACY_ZVS_SERVICE
#endif

// ----------------------------------------------------------------------------------------
// Uncomment to allow/enable the MITM transfer of Cycling Speed and Cadence data from trainer 
// to training App directly. Useful when trainer allows direct pairing of builtin CSC sensor, 
//		  NOTICE --> NOT all (legacy) trainers support CSC!!!
//#define ENABLE_CSC

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
<b>Warning</b>: When a new version of Simcline-V2 is installed in Arduino IDE 2 it will override <b>ALL</b> files of a previous version! If you have made modifications in a file that is part of `Simcline-V2` --> <b>Make</b> a <b>copy</b> of the file(s) in question <b>BEFORE</b> you <b>install</b> a new library version!
