# <img src="./images/SC_logo.png" width="64" height="64" alt="SIMCLINE Icon"> &nbsp; SIMCLINE-V2 for Smart Trainers
# Simulation of Changing Road Inclination for Indoor Cycling<br>
<img src="./images/Simcline_2_0.jpg" width="300" height="300" ALIGN="left" alt="Mechanical SIMCLINE 2">
The SIMCLINE physically adjusts the bike position to mimic hilly roads, climbing and descending. This allows the rider to naturally change position on the bike, engage climbing muscles, and improve pedaling technique to become a more efficient and powerful climber.<br>
Without user intervention the SIMCLINE will replicate inclines and declines depicted in (online & offline) training programs (like <b>Zwift, Rouvy, VeloReality, myWhoosh</b> and many others) that adjust accordingly the resistance of the indoor trainer.<br>
The SIMCLINE auto connects at power up with a Smart trainer and let's relive the ascents and descents from favorite rides or routes while training indoors. The physical reach is: 20% maximum incline and -10% maximum decline. However, the reach that the rider is comfortable with can be adjusted! The SIMCLINE pairs directly to the Smart trainer and with your PC/Laptop/Tablet with (Zwift) training App for a connection that notifies the SIMCLINE to simulate autonomous the (change in) physical grade of the road during an indoor ride. During operation an OLED display shows the road grade in digits and in graphics. <br clear="left"> <br>
Notice that the description on how to build SIMCLINE consists of two parts:

+ <b>Mechanical SIMCLINE 2.0</b><br>
<img src="https://www.instructables.com/assets/img/instructables-logo-v2.png" width="32" height="48" align="left" alt="Instructables"> &nbsp; [SIMCLINE 2.0 Instructables](https://www.instructables.com/SIMCLINE-20-Easy-Simulation-of-Road-Incline/) 
<br clear="left">

+ <b>Present Simcline-V2 Library</b><br>
The Simcline-V2 library comes with an Android companion app for configuring and controlling its features. See: [Companion App](android/readme.md) <br>

# Simcline-V2 Library is optimised for ESP32 and NimBLE-Arduino 2!<br>
The <b>ESP32</b> family has a series of low-cost and low-power System on a Chip (SoC) microcontrollers developed by Espressif that include Wi-Fi and Bluetooth wireless capabilities and dual-core processor. See for an introduction: [Random Nerds Tutorials](https://randomnerdtutorials.com/getting-started-with-esp32/). Particularly the multiprocessing capabilities of the dual-core processor make the ESP32 a very attractive choice for the project! See: &nbsp;[FAQ #5](docs/Frequently_Asked_Questions.md#5)<br>
To benefit of the same formfactor (fit with the Mechanical SIMCLINE 2.0 component box!), the <b>Adafruit Feather ESP32 V2 plus Oled display</b> is still the preferred board and display for the project. See: [Adafruit Feather ESP32 V2](docs/Adafruit%20Feather%20ESP32-V2.md) for settings, wiring scheme and more.<br> 
Notice that other members of the ESP32 (particularly <b>ESP32S3</b>) will do the job perfectly, when the development boards come with extra flash and psram memory! See: &nbsp;[FAQ #7](docs/Frequently_Asked_Questions.md#7). &nbsp; For example SIMCLINE works successfully with the <b>Lilygo esp32s3 T-Display</b> board, however that Lilygo-board needs another size component box! See: [Lilygo esp32s3 T-Display](docs/LILYGO%20ESP32S3%20T-Display.md) for settings, wiring scheme and more.<br> 
Simcline-V2 library builds on the experience, knowledge and code of Simcline projects since <b>2020</b>. The original Simcline code has been revisited and redesigned completely. It was transformed to a C++ Object model that handles the many BLE services and hides most of the Simcline's internal operation. The Simcline community is very diverse when it comes to programming skills. The present Simcline-V2 library matches this much better! It also meets the urge for a greater variety in ESP32 boards and displays to work with.<br>

# The Simcline-V2 design separates completely:
+ presentation (display types/properties)
+ specific ESP32 board properties (pinout and initialization)
+ BLE operation<br>

To achieve this the design has implemented Abstract Base Classes for ESP32 board and Display. As a consequence the <b>novice programmer/user</b> only has to change config settings to select between <b>standard</b> board- and display-types that are supplied with Simcline-V2. However, a <b>proficient programmer/user</b> can implement Concrete Classes for ESP32-board-type and Display-type of his/her choice easily without interfering with the Simcline BLE operational code.<br>
Benefits of this Approach:
+ Scalability: Easily add new display types by creating new concrete classes implementing the IDisplay interface.
+ Maintainability: Changes to one display type do not affect others.
+ Flexibility: Change the display type used by the Presentation class without modifying its implementation.<br>

As a result the Arduino ino-files that the user will access, in the `Simcline-V2/examples` folder, are very concise in compare with the original code files. <br>

# Simcline-V2 has central configuration
All configuration settings have been gathered in one config directory `../documents/arduino/libaries/Simcline-V2/src/config` for <b>Board</b>-, <b>Debug</b>-, <b>Display</b>-, <b>NimBLE</b>- and <b>Simcline</b>-configurations. Check this out:  [Simcline-V2 Configuration](src/config/README.md)<br>

# NimBLE-Arduino
The Simcline project heavily leans on the <b>NimBLE-Arduino</b> library for Bluetooth handling, see: [H2Zero/NimBE-Arduino](https://github.com/h2zero/NimBLE-Arduino). <b>NimBLE-Arduino</b> is structured for compilation with Arduino and for use with ESP32! Simcline-V2 works only (!) with the latest version: <b>NimBLE-Arduino Version 2</b>!<br>

# FiTness Machine Service a Bluetooth Service Specification
From 2015 to 2017 the Sports and Fitness Working Group (SIG) designed a Bluetooth Service specification. This service exposes training-related data in the sports and fitness environment, which allows a Server (e.g., a fitness machine) to send training-related data to a Client. In Februari 2017 the service specification reached a stable version: [Fitness Machine Service 1.0](https://www.bluetooth.com/specifications/specs/fitness-machine-service-1-0/) when it was adopted by the Bluetooth SIG Board of Directors. Have a look at the document to appreciate the effort of all the contributors and the companies they represented!<br>
<b>FTMS</b> is an open (nonproprietary) protocol that is not owned by any particular company and not limited to a particular company's product. It can be compared in that respect with FE-C over ANT+, however <b>FTMS</b> is targeted to control fitness equipment over <b>Bluetooth</b>! Today <b>Bluetooth Smart FTMS</b> is the absolute industry standard for indoor trainers and Simcline-V2 is primarily targeted at <b>FTMS</b>! <br>

# Simcline-V2 is fully supporting:
|Trainer  |Supported protocols|
|-----------|-------------------------------------------------------------------------------------| 
|Elite |Bluetooth Smart FTMS on all 2020 smart trainers.|
|Gravat |Bluetooth Smart FTMS on all 2020 smart trainers|
|JetBlack |Bluetooth Smart FTMS with HRM.&nbsp;[FAQ #3](docs/Frequently_Asked_Questions.md#3)|
|Kinetic |Bluetooth Smart FTMS on all 2020 smart trainers.|
|Minoura |Bluetooth Smart FTMS on all 2020 smart trainers.|
|Saris |Bluetooth Smart FTMS on all 2020 smart trainers.|
|STAC |Bluetooth Smart FTMS on all 2020 smart trainers.|
|Tacx |Bluetooth Smart FTMS on all 2020 smart trainers.|
|Wahoo |Bluetooth Smart FTMS on all 2020 smart trainers <b>and</b> legacy Wahoo Bluetooth Smart Control.|
|Zwift Hub|Bluetooth Smart FTMS with HRM.&nbsp;[FAQ #3](docs/Frequently_Asked_Questions.md#3)|

# Man-In-The-Middle (MITM) software pattern<br>
<img src= "./images/FTMS_MITM.jpg" align="left" width="1000" height="500" alt="Man in the Middle"><br>
<b>Man-In-The-Middle</b> is a powerful software engineering pattern that is applied in many software designs. Unfortunately it is also known for a negative application in communication traffic: MITM is a common type of cybersecurity attack that allows attackers to eavesdrop on the communication between two targets.
We have applied the very principle: the Simcline is strategicly positioned in between the BLE communication of the Smart Trainer and the training App (like Zwift) running on the PC/Laptop, all communication traffic can be inspected in that MITM position, when it is passed on from one to the other, in both directions. When Zwift sends resistance information (like the road inclination) to the Smart trainer, this information can be intercepted and applied to determine the up/down positioning of the Simcline. <br>

# How to start?<br>
+ Install the [Arduino IDE 2](https://www.arduino.cc/en/software#experimental-software)
+ Install your [ESP32 board](https://randomnerdtutorials.com/installing-esp32-arduino-ide-2-0/) in the Arduino environment
+ Install [Adafruit OLED and GFX Libraries](https://makeabilitylab.github.io/physcomp/advancedio/oled-libraries.html)
+ Install the ESP32 NimBLE-Arduino library (<b>Version 2.#.#</b>), in <b>Arduino IDE</b> go to `Sketch menu` -> `Include Library` -> `Manage Libraries`, search for NimBLE-Arduino and install. [Ref](https://github.com/h2zero/NimBLE-Arduino#arduino-installation)
+ Install the Simcline-V2 library from this repository. Download as `.zip` and extract to `Arduino/libraries` folder, or <br>in <b>Arduino IDE</b> from `Sketch menu` -> `Include library` -> `Add .Zip library`<br>

# How to make it work?<br>
The requirements in this phase are simple: 
+ running Zwift, Rouvy or myWhoosh app or alike, 
+ working Feather ESP32-V2 (or other ESP32S3) development board and 
+ working Smart Trainer (see above support list)
+ running Simcline-V2 library and Smart-MITM application.<br>

# Testing is Knowing!<br>
I can understand and respect that you have some reserve: Is this really working in my situation? Better test if it is working, before buying all components and start building.
In the Simcline-V2 Library <b>Examples</b> section (see `..documents/arduino/libraries/Simcline-V2/examples`) you will find the appropriate applications for testing and running SIMCLINE that come with the library. The one you should start with in this stage is <b>Smart-MITM</b> It is developed with the only intention to allow you to check if the MITM solution is delivering in your specific situation. Within the Arduino IDE 2 select on the top bar menu: `File > Examples > Simcline-V2 > Smart-MITM`<br>

<b>What it does in short:</b><br>
<img src="./images/Feather_FTMS.jpg" align="middle" width="1000" height="500" alt="Simcline in the Middle"><br>
A working <b>MITM</b> implementation links a bike trainer (BLE Server FTMS) and a PC/Laptop (BLE Client running Zwift) with the Feather ESP32, like a <b>bridge</b> in between. The MITM bridge can pass on, control, filter and alter the interchanged trafic data! The <b>MITM</b> code is fully ignorant of mechanical or electronic components that drive the Simcline construction.<br>
```
It simply estabishes a virtual BLE bridge and allows you to ride the bike on the Smart Trainer and 
feel the resistance that comes with the route you have choosen, thanks to Zwift.
The experience should not differ from a normal direct one-to-one connection, Zwift - Smart Trainer!
```
All Bluetooth Smart FTMS indoor trainers expose your efforts on the bike in 2 additional BLE services: Cyling Power (CPS) and Speed & Cadence (CSC). These services are detected and applied by many training app's and are therefore an integral part of the present design of the MITM bridge. Training app's simply expect, when they connect to the Bluetooth Smart FTMS trainer, that the CPS and CSC services are available in one go! The Zwift pairing screen is a good example: it expects Power Souce (CPS), Resistance (FTMS) and Cadence (CSC) to be connected...
+ The client-side (Feather ESP32) scans for (a trainer) and connects with <b>FTMS, CPS and CSC</b> and collects cyling power, speed and cadence data like Zwift would do! The <b>Simcline Client</b> is doing just that at the left side of the "bridge"!
+ The Server-side (Feather ESP32) advertises and enables connection with training/cycling/game apps like Zwift and collects relevant resistance data, it simulates as if an active <b>FTMS</b> enabled trainer is connected to Zwift or alike! Notice that the Server-side also exposes active <b>CPS</b> and <b>CSC</b> services. The <b>Simcline Server</b> is doing just at the right side of the "bridge"!
+ The <b>Simcline MITM</b> code is connecting both sides at the same time: a full-blown working bridge<br clear="left">

# Load Smart-MITM application in Arduino IDE 2
Within the Arduino IDE 2 select on the top bar menu: `File > Examples > Simcline-V2 > Smart-MITM`<br>
The default Simcline-V2 configuration settings for Smart-MITM are: 
+ Display: <b>NODISPLAY</b>
+ ESP32 board: <b>YOUR_ESP32_BOARD</b>
+ NimBLE: <b>FTMS</b> and <b>CSC</b>
+ Debug:  <b>Defined</b>

Optionally:
+ Users with a <b>Adafruit Feather ESP32 V2</b> can setup their board and display now! See: [Setup Adafruit Feather ESP32 V2](docs/Adafruit%20Feather%20ESP32-V2.md)<br>
+ Users with a <b>Lilygo esp32s3 T-Display</b> can setup the board and display now! See: [Setup Lilygo esp32s3 T-Display](docs/LILYGO%20ESP32S3%20T-Display.md)<br>

Mandatory:
+ Users of <b>Legacy Wahoo KICKR</b> trainers (<b>NOT supporting FTMS</b>) must make changes now! See: [Setup for Legacy Wahoo KICKR](docs/Setup_Legacy_Wahoo_KICKR.md).

<b>Smart-MITM</b> is default in Debug mode: using Serial Monitor (logging on the PC-screen) to show you what is happening!<br>

When the settings are conforming your desired setup, it is time to run Smart-MITM!

# Compile and Upload Smart-MITM to your ESP32 board
A recipe for success: follow <b>ALWAYS</b> the instructions and procedure at the top of the respective program codes!
```
/* -----------------------------------------------------------------------------------------------------
 *             This code should work with all indoor cycling trainers that fully support,
 *        Fitness Machine Service, Cycling Power Service and Cycling Speed & Cadence Service
 * ------------------------------------------------------------------------------------------------------
 * NOTICE: that you need to have set first all config file settings in accordance with your specific setup!
 *         see: ../Documents/Arduino/libraries/FTMS-Simcline/src/config
 *
 *  The code links a BLE Server (a Peripheral to Zwift) and a BLE Client (a Central to the Trainer) with a bridge 
 *  in between, the ESP32 being man-in-the-middle (MITM). The ESP32 is an integral part of the Simcline design,
 *  that interprets the exchanged road grade and moves the front wheel up and down with the change in inclination.
 *  The ESP32-bridge can control, filter and alter the bi-directional interchanged data!
 *  The client-side (central) scans and connects with the Trainer relevant services: CPS and FTMS. It collects 
 *  all cyling data of the services and passes these on to the server-side....  
 *  The client-side supplies the Indoor Trainer with target and resistance control data.
 *  The server-side (peripheral) advertises and enables connection with cycling apps like Zwift and collects the app's  
 *  control commands, target and resistance data. It passes these on to the client-side....  
 *  The server-side supplies the app with the generated cycling data in return. 
 *  
 *  The client plus server (MITM) are transparent to the Indoor Trainer as well as to the training app Zwift or alike!
 *  
 *  Requirements: Zwift app or alike, ESP32 board (NO display required) and a supported Indoor Trainer
 *  0) Upload and Run this code on your ESP32 board
 *  1) Start the Serial Monitor to catch debugging info
 *  2) The code will do basic testing of electronic parts and settings
 *  3) Start/Power On the Indoor Trainer  
 *  4) Your ESP32 and Trainer will pair as reported in the output
 *  5) Start Zwift on your computer or tablet and wait....
 *  6) Search on the Zwift pairing screens for your ESP32 a.k.a. <SIM32>
 *  7) Pair: Power Source, Resistance and Cadence one after another with <SIM32>
 *  8) Optionally one can pair as well devices for heartrate and/or steering (Sterzo)
 *  9) Start the default Zwift ride or any ride you wish
 * 10) Make Serial Monitor output window visible on top of the Zwift window 
 * 11) Hop on the bike: do the work and feel resistance change with the road
 * 12) Inspect the info presented by Serial Monitor.....
 *  
 *   This device is identified with the name <SIM32>. You will see this only when connecting to Zwift on the 
 *   pairing screens! Notice: Zwift extends device names with additional numbers for identification!
 *  
*/ 
```
# Points of Interest
+ Be aware of undesirebly <b>autoconnect</b> of Zwift with your trainer using ANT+ or BLE Smart before <b>Smart-MITM</b> can establish a connection: always <b>start</b> Zwift <b>AFTER</b> Smart-MITM and trainer have connected successfully! The <b>Smart-MITM</b> code will than fail to connect, that does not help you getting representative results during the reconnaisance! Only one client can control at the same time: 2 captains on one ship is a recipe for disaster! See for more info [FAQ #1](docs/Frequently_Asked_Questions.md#1) and [FAQ #6](docs/Frequently_Asked_Questions.md#6)
+ Please write down the presented MAC/Device Addresses of a) your Smart trainer and b) your Desktop/Laptop with Zwift. These showup in the Serial Monitor output when running the Smart-MITM test code. This is for your own convenience since it helps you to identify later both devices by MAC Addresses! The Smart-MITM detects the Mac addresses and stores these in ESP32 NVS (Non-Volatile-Storage) for later use to unmistakingly establish a BLE connection with the targeted devices. See [FAQ #9](docs/Frequently_Asked_Questions.md#9)
+ When you see different road grade values in the Zwift window compared with the Simcline display or in the Serial Monitor output, this can have different reasons! See: [FAQ #8](docs/Frequently_Asked_Questions.md#8)

