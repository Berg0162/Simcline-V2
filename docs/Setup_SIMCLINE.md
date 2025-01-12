
Follow these steps:



1) Test the board with display attached to it, after you have installed the appropriate graphics libraries (like Adafruit GFX and/or TFT_eSPI). Notice that you have to install the original libraries and not the modified versions that product suppliers like to include! All graphics graphics libraries come with code examples. For TFT_eSPI you need to set "your user settings" to comply with the board type you use. Read and study the instructions that come with the sites of these libraries!



2) Test BLE connection with FTMS_Client code (Trainer) and FTMS_Server code (Laptop + Zwift) to see if things are working with your hardware and equipment! Note down the Mac Addresses of your Trainer and Laptop (Zwift). Test the FTMS_Bride code to see if with your equipment MITM is working properly.



3) Download relevant code and install the Simcline critical libraries.



4) Attach the TOF-sensor and test. First load the library that comes with it. Test with supplier code examples and the Simcline test samples in the Arduino folder.



5) Attach the Motor Driver board and test. No library needed, but the supplier will have example code supplied. Use the Simcline test samples in the Arduino folder.



6) Attach the Actuator to the Motor Driver board and test the Actuator....



7) Setup and mount Simcline mechanically...



8) Add the TOF-sensor to the mechanic setup and check it can measure the distance correctly. Repeat 3) for testing. Note down the MAX and MIN values of the TOF-sensor reaching the the top position and lowest position!



9) Insert the settings that you have noted down before in the Simcline Diagnostics Test code. Upload and run to see if all components work together properly.



10) Insert the settings that you have noted down before in the FTMS_Simcline code, compile and upload! Simcline should recognise the TOF-sensor, motor driver board and test the actuator movements + TOF-sensor readings. If successful, it will scan for the trainer (+ connect) and advertise/connect with the Laptop (Zwift).

