# Legacy Tacx Smart Trainers with (ANT+) FE-C over BLE
Today <b>Bluetooth Smart FTMS</b> is the absolute industry standard for indoor trainers and Simcline-V2 is primarily targeted at <b>FTMS</b>! However only since 2020 most trainers have builtin support for FTMS. Simcline-V2 has also builtin support for **Tacx Smart Trainers with (ANT+) FE-C over BLE** to meet the needs of early adapters of SIMCLINE!<br>

The original ANT+ FE-C protocol details the bi-directional message communication of trainer data (speed, power, etcetera) from trainer to the controller and from the controller to the trainer commands or settings of targeted power, trainer resistance (grade) and calibration controls.
See [ThisisAnt](http://www.thisisant.com/) for all conceivable documentation, tools, implementations, ANT+ based products, etcetera!
Until about 2015 ANT+ FE-C protocol was the dominant standard for training equipment.<br>

Since about 2015 the Tacx Company produced Tacx Smart Trainers that comply **ANT+ FE-C** and **Bluetooth**. Tacx implemented a proprietary BLE-protocol then called: _(ANT+) FE-C over BLE_. Tacx designed this proprietary BLE-protocol because at that time an open standard on BLE for trainers was completely lacking. 
You think this applies to your legacy Tacx trainer? Always check first if your legacy Tacx trainer has support for the (ANT+) FE-C protocol over BLE, since NOT all legacy Tacx Smart Trainers that are labelled _smart_ are equally _smart_!

This Tacx FE-C functionality is now builtin with Simcline-V2, you only have to **activate** it, when using a legacy Tacx smart trainer! Notice that FTMS and legacy Tacx FE-C mutually exclude each other within Simcline-V2!<br>

# configNimBLE.h
Make configNimBLE.h settings to comply with your <b>Legacy Tacx FE-C</b> smart trainer.

+ Open file, edit and save(!): `/documents/arduino/libraries/Simcline-V2/src/config/configNimBLE.h`

+ Set Simcline-V2 <b>NimBLE</b> configuration for use of <b>Legacy Tacx FE-C</b>:
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
// Smart (Bluetooth) Wahoo Trainer Control: This is Wahoo’s (legacy) proprietary method of 
// controlling Wahoo trainers, and includes speed/cadence data – still supported by Zwift a.o.
//		  NOTICE --> Most pre-2020 Wahoo trainers support this instead of FTMS!!!
//#define ENABLE_WAHOOCPS

// ----------------------------------------------------------------------------------------
// Uncomment to allow/enable the MITM transfer of Tacx FE-C (Fitness Equipment Cycling) data 
// from trainer to training App directly.
// FE-C ANT+ over Bluetooth: This is Tacx’s (legacy) proprietary method of controlling Tacx 
// trainers, and includes power and speed/cadence data – still supported by Zwift a.o.
//		  NOTICE --> Most pre-2020 Tacx trainers support this instead of FTMS!!! 
#define ENABLE_TACXFEC

```
<b>Warning</b>: When a new version of Simcline-V2 is installed in Arduino IDE 2 it will override <b>ALL</b> files of a previous version! If you have made modifications in a file that is part of `Simcline-V2` --> <b>Make</b> a <b>copy</b> of the file(s) in question <b>BEFORE</b> you <b>install</b> a new library version!
