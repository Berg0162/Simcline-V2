# Legacy Wahoo KICKR
Today <b>Bluetooth Smart FTMS</b> is the absolute industry standard for indoor trainers and Simcline-V2 is primarily targeted at <b>FTMS</b>! However Simcline-V2 has builtin support for <b>Legacy Wahoo KICKR</b> trainers to meet the needs of early adapters of SIMCLINE! <br>
Pre-2020 Wahoo trainers use Cycling Power Service as a trainer control. Wahoo has added a custom characteristic: `A026E005-0A7D-4AB3-97FA-F1500F9FEB8B` to CPS to handle the Control Point characteristic. This function is builtin with Simcline-V2, you only have to activate it, when using legacy KICKR! Notice that FTMS and Wahoo CPS mutually exclude each other!<br>

# configNimBLE.h
Make configNimBLE.h settings to comply with your <b>Legacy Wahoo KICKR</b> trainer.

+ Open file, edit and save(!): `/documents/arduino/libraries/Simcline-V2/src/config/configNimBLE.h`

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
<b>Warning</b>: When a new version of Simcline-V2 is installed in Arduino IDE 2 it will override <b>ALL</b> files of a previous version! If you have made modifications in a file that is part of `Simcline-V2` --> <b>Make</b> a <b>copy</b> of the file(s) in question <b>BEFORE</b> you <b>install</b> a new library version!
