#ifndef CONFIG_NIMBLE_H_
#define CONFIG_NIMBLE_H_

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

// ----------------------------------------------------------------------------------------
// FTMS and Wahoo CPS (Control-Protocol-Service) mutually exclude each other! Be Aware!!!
#ifdef ENABLE_FTMS
#undef ENABLE_WAHOOCPS
#else
#ifdef ENABLE_WAHOOCPS
#undef ENABLE_HRM      // WahooCPS excludes the use of HRM baked in!!!
#endif
#endif

// ----------------------------------------------------------------------------------------
// Your FIXED hardware Laptop/Trainer/Smartphone BLE MAC- or DEVICE-Addresses 
// Enter address string here like the printed format like [00:01:02:03:04:05 0]
// LAPTOP Fixed Device Address --------Public Type (0) --------- Random Type (1)-----------
//#define LAPTOPADDRESS "00:01:02:03:04:05 0" // Example Mac string of Public Type
// TRAINER Fixed Device Address --------Public Type (0) --------- Random Type (1)----------
//#define TRAINERADDRESS "00:01:02:03:04:05 1"  // Example Mac string of Random Type
// ---------------------------------------------------------------------------------------

// NimBLE Client, Server MITM related settings -------------------------------------------
#define BLE_APPEARANCE_GENERIC_CYCLING   1152
#define THISDEVICENAME "SIM32" // Shortname 

// ---------------------------------------------------------------------------------------
// Training Apps set MTU after connection from 23 to 255, this allows for max data throughput!
const uint8_t MAX_PAYLOAD = 20;  // Max 20 (23-3) byte size for array DATA and strings
// Preferred Connection Parameters for max data throughput: Client-side (!) AND Server-side !
const uint16_t ConnectionMinInterval = 6; 	// Min interval (7.5ms)  
const uint16_t ConnectionMaxInterval = 12;	// Max interval (15ms)  
const uint16_t ConnectionLatency = 0;   	// No slave latency (immediate response)
const uint16_t ConnectionTimeout = 200; 	// Supervision timeout (2 seconds before disconnect)
const uint16_t ScanInterval = 160;		// Scan every 100ms (160 * 0.625ms)
const uint16_t ScanWindow = 160;		// Active scanning for 100ms (160 * 0.625ms)
const uint16_t ConnectionMTU = 255;     	// ATT (Attribute Protocol) Maximum Transmission Unit
const uint16_t AdvertiseMinInterval = 160;	// 100ms (160 * 0.625ms)
const uint16_t AdvertiseMaxInterval = 240;	// 150ms (240 * 0.625ms)

/* ---------------------------------------------------------------------------------------
* The Data field is dependent on the Bluetooth specification. In Bluetooth v4.0 and 4.1, 
* the maximum size of the Data field was 27 bytes. With Bluetooth v4.2, a new feature was 
* added to negotiate the Link Layer Data field length with Data Length Extension.
*/
#define EXTENDEDDATALEN
#ifdef EXTENDEDDATALEN
const uint16_t ExtendedDataLen = 0x00FB; // Link Layer Preferred Extended Data Len: 251
#endif

// ---------------------------------------------------------------------------------------
const bool indications = false;  //false as first argument to subscribe (!) to indications
const bool notifications = true; //true as first argument to subscribe to notifications

// ---------------------------------------------------------------------------------------
// Set Arduino IDE Tools Menu --> Events Run On: "Core 1"
// Set Arduino IDE Tools Menu --> Arduino Runs On: "Core 1"
// xTask constants to set xTasks to run on "Core 0" or "Core 1"
const BaseType_t xTaskCoreID0 = 0;
const BaseType_t xTaskCoreID1 = 1;

#endif // CONFIG_NIMBLE_H_
