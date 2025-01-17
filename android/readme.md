# Simcline Android Companion App

The SIMCLINE Companion App (for Android smartphones) can be paired with SIMCLINE, over a <b>secured and authenticated</b> connection, instead of the cycling App (Zwift)! During SIMCLINE startup when the cycling App is <b>not</b> activated yet and after a ride when the cycling App is <b>disconnected</b>. The Companion App allows for the dynamic setting of some preferences like Ascent Grade Limit (between 0-20%), Descent Grade Limit (between 0-10%), Road Grade Change Factor (between 0-100%) and for simple manual control/testing of the Up and Down movement.
The preferences will be stored in ESP32 NVS (Non-Volatile-Storage) when you send them to the SIMCLINE. Every following bootup will reactivate your preferences!

This folder contains files for the Android companion app for the Simcline-V2 library.

## Files
- `Simcline_v2_5_2.aia`: The project file for [MIT App Inventor](https://appinventor.mit.edu/). Use this to modify or rebuild the app.
- `Simcline_v2_5_2.apk`: The pre-built Android app. Install this on your Android device to configure and simple control the SIMCLINE.

The Android app is built with BluetoothLE version: `edu.mit.appinventor.ble-20240822.aix`

## Instructions
- To install the `.apk`:
  1. Transfer the file to your Android device.
  2. Enable installation from <b>unknown sources</b> in your device settings.
  3. <b>BLE Location</b> must be enabled in advance or when requested.
  4. Open the file to install the app.
- To modify the app, import the `.aia` file into [MIT App Inventor](https://appinventor.mit.edu/).

## Test Simcline and Smartphone together
A dedicated program for testing the functioning of Simcline and Smartphone, over a <b>secured and authenticated</b> connection, is available at: `/Simcline-V2/examples/Test_Simcline_App_Connection` to help you with the task! Notice that the security pin when the two pair is: `123456`
