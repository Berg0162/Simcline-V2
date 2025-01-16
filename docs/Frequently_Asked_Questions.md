
# [1] 
<b>Who is in control?</b><br>
+ When a training app (like Zwift on your laptop) has connected to your trainer using the FTMS protocol: is it possible to connect multiple devices via FTMS? As FTMS enables control of a physical device there can only be one <b>“controller”</b> to avoid safety issues. This means that you will not be able to connect multiple devices directly to the indoor bike trainer or treadmill using FTMS. If the trainer does not appear in an app’s (e.g. Zwift's) device list (on the Zwift pairing screen) it generally means the trainer is (still) connected to another controlling app or device. It is virtually impossible to connect the trainer to Zwift using FTMS, have a nice indoor ride and at the same time to connect for example the Simcline to the trainer or Zwift for simulating road incline... This means that other active cycling App's (installed on your telephone and/or tablet) are in competition with your Zwift App on the laptop to connect with the trainer! Be carefull with devices with active cycling Apps near your setup during the connection phase! Notice that modern smartphones have very powerful BLE/WIFI chips that have an indoor BLE reach of 6-10 meters. If in doubt: turn Flight Mode On!

# [2]
<b>What about ANT+ (FE-C) and FTMS at the same time?</b><br>
+ When a training app (like Zwift) has connected to your trainer using the ANT+ protocol: is it possible to connect other devices via FTMS?<br>
Since this ANT+ connection enables control of the physical device (trainer) there can NOT be connected another <b>“controller”</b> at the same time over FTMS to avoid safety issues. Only one (1) controlling app is allowed to connect and drive the Trainer at any time. You know, 2 captains on one ship is a recipe for disaster!
If this case, unfortunately and undesirebly, happens with your equipment setup, the controlling Client-side code will not connect or disconnect with an error message! So keep these worlds separated! If you intend to use devices with BLE and FTMS: mechanically disconnect the ANT+ dongle to avoid your controller App (like Zwift) to (auto)connect over ANT+.

# [3]
<b>What about Zwift Hub and Jetblack users?</b><br>
+ There is an excellent review available [DCRainmaker Zwift Hub review](https://www.dcrainmaker.com/2022/10/zwift-hub-smart-trainer-in-depth-review-the-best-bang-for-your-buck.html), that describes o.a. a special goodie that comes with the Zwift Hub and Jetblack:
>**– Protocol Compatibility:** ANT+ FE-C, ANT+ Power, Bluetooth Smart Trainer Control, Bluetooth Smart Power (everything you need)<br>
**– Unique Party Trick: Can rebroadcast your heart rate sensor within a single channel, ideal for Apple TV Zwift users (who are Bluetooth channel limited)**<br>
**– App Compatibility:** Every app out there basically (Zwift, TrainerRoad, Rouvy, RGT, The Sufferfest, Kinomap, etc…)<br>

Simcline-V2 **fully** supports this proprietary Zwift function for your heart rate sensor connection. The code supports the same features as you would have had with only Zwift App connected to Zwift Hub or JetBlack trainer! Use your heart rate band the way you are used too before the Simcline-V2 came in between your Zwift Hub/Jetblack trainer and the Zwift App! It should be fully transparent with respect to this feature! Please test yourself!

# [4]
<b>How to cleanup Zwift devices from the past?</b><br>
+ Zwift can sometimes hang onto the wrong info, such as trainers or sensors that were paired to the game in the past. Zwift uses Mac Addresses from previous connections to identify devices. So when device names change Zwift hangs on to the unique Mac Addresses rather than the names that you see in the pairing screens! This can be rather confusing and lead to misunderstandings when you connect devices having only their original names shown and not the actual names....<br>
Check out the steps below.<br>
For <b>PC/Mac</b> to reset all the Zwift stored devices on a PC or Mac, complete these steps:
- Close Zwift
- On your desktop, open Documents
- Double-click Zwift
- Delete or Edit <knowndevices.xml><br>

Next time you go for a Zwift ride:
- Launch Zwift
- Pair your devices
 
# [5]
<b>How is Dual Processor used with ESP32?</b><br>
+ One of the advantages of the ESP32 platform is the fact that the ESP32 WROOM processor has two cores. This makes it possible to precisely balance the load of a program over 2 processor cores. With the Simcline this is particular usefull for the motor control of the actuator. During operation Zwift sends from time to time new settings, and one of these is the grade value (road inclination in degrees). The program translates the grade to a level that should be reached by the actuator to simulate exactly the road grade that was received from Zwift. However, the actuator can only be switched to <b>move up</b>, <b>move down</b> or <b>stop</b>. After having set the actuator to move (up or down), the program has to check continuously if the actuator has reached the desired level by reading its position with the help of the Time-Of-Flight sensor and act accordingly. Meanwhile the trainer sends your cycling data and the Zwift app has to confirm the receipt of these data. The data sent by Zwift has also to be tranferred to the trainer and also the trainer has to confirm the receipt. Being a MITM means handling a lot of BLE traffic and it does not allow for mistakes!
The load of the Simcline program itself, the BLE handling and the critical control of the actuator is balanced over 2 processor cores on the ESP32 platform. To avoid conflicts during variable updates (i.c. TargetPosition) a Binary Semaphore scheme is applied to protect <b>task shared variables</b> during an update.<br clear="left">

# [6]
<b>Why is my trainer variably successful in connecting with the SIMCLINE?</b><br>
+ In most cases this behavior can be attributed to <b>NOT</b> follow the critical sequence for starting and connecting of Trainer, SIMCLINE and Zwift.<br>

0) The Start or Initial situation of <b>ALL</b> parties involved is: Laptop/Zwift, Simcline and Trainer are Powered <b>OFF</b>!
1) Trainer Power-ON --> Trainer needs some time (4 seconds?) to settle and start advertising!
2) Power-ON or Reset Simcline (ESP32 board) --> Simcline needs some time to start, test motor functions and start scanning for Trainer!
3) Wait for Simcline and Trainer to connect! 9 Out of 10 times this is immediately successful. If this is NOT the case then Reset Simcline first and wait again for a Simcline-Trainer connection! If this is NOT successful: Read [FAQ #1](https://github.com/Berg0162/Simcline-V2/blob/main/docs/Frequently_Asked_Questions.md#1), take appropriate measures and than go back to "Start" after <b>ALL</b> components have been Powered <b>OFF</b> first.
4) Laptop/Zwift Power-ON --> ONLY when Simcline and Trainer have been connected successfully!
5) Wait and wait for the Zwift pairing window to pop-up
6) Click orange POWER SOURCE button and select in the list with devices: ESP32#### (Simcline) --> Close
7) Click orange RESISTANCE button and select in the list with devices: ESP32#### (Simcline) --> Close
8) Click orange CADENCE button and select in the list with devices: ESP32#### (Simcline) --> Close
9) Click orange OK! button, when all the selected devices are conforming your choices and are indicated as <b>CONNECTED</b>!

# [7]
<b>Will Simcline-V2 work out-of-the-box with my ESP32S3 development board?</b><br>
+ Yes, in most cases, when we assume your ESP32S3 has enough Flash and PSRAM memory on board. Particularly when you <b>restrict</b> (!) your attention initially to the <b>Smart-MITM</b> application, to check out if the concept is working with your hardware.<br>
If you want finally a full blown working SIMCLINE setup, you have to <b>complement</b> the concrete ESP32board class in the file: `/documents/arduino/libraries/Simcline-V2/src/board/YOUR_ESP32_Board.h`. It needs minimally definitions for the PIN assignments: TOF-sensor (I2C), DRV8871 Motor Driver board (Actuator) and optionally for 2 Buttons. Possibly you have to insert code for specific board initialization (not shown).<br>
```C++
#define PIN_ACTUATOR_1 	-1  // PIN to be connected to pin IN2 of the Adafruit DRV8871 Motor Driver board
#define PIN_ACTUATOR_2 	-1  // PIN to be connected to pin IN1 of the Adafruit DRV8871 Motor Driver board

// Optional Button 1 and 2 PIN connections
//#define PIN_BUTTON_1  -1  // PIN to be connected to Internal/External button 
//#define PIN_BUTTON_2  -1  // PIN to be connected to Internal/External button 

#define PIN_I2C_SCL    	SCL // Default in Arduino IDE -> Check for your board!
#define PIN_I2C_SDA    	SDA // Default in Arduino IDE -> Check for your board!
```
For how Arduino IDE 2.x handles the specific board pin assignments, search for your board in: [this reference](https://github.com/espressif/arduino-esp32/blob/master/variants)

# [8]
<b>How are Zwift, Trainer and Simcline handling (changes in) road grade during your ride!</b><br>
1) The trainer calculates the resistance, among others using road grade information. That's why Zwift sends modified road grades to the trainer to set resistance. Simple as that! However, resistance (i.e. road grade) is also dependent of Zwift's famous difficulty setting. With the default difficulty setting of 50%, Zwift sends only half the value of the road grade to the trainer. Only when the Zwift App difficulty setting is 100%, Zwift will send unmodified road grades to the trainer (WYSIWYG). As a consequence the Simcline will lift accordingly and show the road grade values on the display! If you observe deviations when riding long and steady climbs, like Alpe-de-Huez or Mt-Ventoux, check the Zwift App "difficulty setting" that is most likely the cause! Most Zwifters have set the Zwift difficulty at 100%, because they want a "road-realistic" experience when training and racing indoors with Zwift.
2) Trainers do NOT react momentarily, but delayed on changes in road grade, they need to process incoming info and the (direct) drive mechanism has to be settled accordingly before you will feel it as a change in resistance. The delay between the initial command (by Zwift) and the feel on the bike is 1 to 1.5 seconds. To accommodate for this delay, the Zwift app sends commands 1 to 1.5 second <b>earlier</b> (!) than what you observe on your screen (slope of the road you see and the displayed road grade value on the Zwift window). This explains temporarily deviating values on the Simcline display in comparison what you see on the Zwift window, after all Simcline is a MITM and is the first to receive changes in road grade, to react (moving the lift mechanism) and to show the new value on the display. When riding rolling hills in rapid succession, like Zwift's Titans Grove, this is most visible! Notice that the Simcline's responsiveness is benefiting of the feature!
3) Zwift handles downhill roads differently! When the road grade values become negative, during a downhill section of the road, Zwift will consequently send only half of the road grade value to the trainer (i.e Simcline). Modern trainers with direct drive are capable to accelerate proportionally with negative road grades. So when the Zwift window shows a -8.0 grade value it has sent only -4.0 road grade to the trainer. The rationale behind this is to minimize effortless riding, after all you get on an indoor bike to do some work! Simcline will act accordingly and display only 50% of the Zwift road grade value on downhill sections!  
4) After Zwift has paired with your trainer it sends a first road grade value (randomly between 2.0 and 5.0) to the trainer, to wake-up/activate the resistance mechanism. You will feel the resistance while you were expecting a flat road for a start. Simcline, being a MITM, will lift your front wheel accordingly! This road grade value remains constant, until you have actually <b>started</b> with the ride of your choice!
5) The Simcline code has its own setting that influences how it will react on and display the road grade value sent by Zwift. The Grade-Change-Factor functionality was originally designed and implemented to allow a Simcline user to attenuate the actuator movements in case of a noisy and/or slow actuator. Notice that the original (unmodified) Zwift grade value is passed on to the trainer, so the trainer's resistance is NOT affected by the Grade-Change-Factor setting! Default code setting for the Grade-Change-Factor is 100% and it can be modified dynamically using the Simcline app.

In conclusion when you see different road grade values in the Zwift window compared with the Simcline display, this can have different reasons! If you consider this a problem: check first Zwift difficulty setting and Grade-Change-Factor of the Simcline, these can be modified to what you prefer! Like it or not, the other causes are Zwift features and part of the design! Cycling Apps like Rouvy and MyWhoosh handle road grades more or less comparably.

# [9]
<b>How to (later) manage (changed) Device MAC Addresses?</b> <br>
When you run <b>Smart-MITM</b>, it is instructed to detect and store in ESP32 NVS (Non-Volatile-Storage) the MAC addresses for Trainer and Laptop. These stored MAC addresses are used by <b>Smart-Simcline</b> to unmistakenly connect to the right client and server! If you want to check or eventually change the MAC addresses in NVS (because of changing devices) you will find in `Simcline-V2/examples` a tool <b>Test_Board_plus_NVS</b> that will help with the task! Notice that MAC addresses have to be a) <b>correct</b> and b) in <b>right format</b> to be resolved at connection-time!<br>
The second option with permanent result, is to insert <b>defines</b> for Laptop and Trainer in the `configNimBLE.h`.
+ Open file, edit and save(!): `/documents/arduino/libraries/Simcline-V2/src/config/configNimBLE.h`<br>
Look for the following section:
```C++
// ----------------------------------------------------------------------------------------
// Your FIXED hardware Laptop/Trainer/Smartphone BLE MAC- or DEVICE-Addresses 
// Enter address string here like the printed format like [00:01:02:03:04:05 0]
// LAPTOP Fixed Device Address --------Public Type (0) --------- Random Type (1)-----------
//#define LAPTOPADDRESS "00:01:02:03:04:05 0" // Example Mac string of Public Type

// TRAINER Fixed Device Address --------Public Type (0) --------- Random Type (1)----------
//#define TRAINERADDRESS "00:01:02:03:04:05 0"  // Example Mac string of Public Type
```
+ Uncommented `#define LAPTOPADDRESS` and `#define TRAINERADDRESS` are <b>overruling</b> whatever values are stored within NVS !

