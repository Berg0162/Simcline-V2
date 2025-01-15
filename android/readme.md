# Simcline Android Companion App

The SIMCLINE Companion App (for Android smartphones) can be paired with SIMCLINE (i.c. ESP32 board) instead of the cycling App (Zwift)! During SIMCLINE startup when the App is <b>not</b> activated yet and after a ride when the App is <b>disconnected</b>. The Companion App allows for the dynamic setting of some variables like Ascent Grade Limit (between 0-20%), Descent Grade Limit (between 0-10%), Road Grade Change Factor (between 0-100%) and for simple manual control/testing of the Up and Down movement.

This folder contains files for the Android companion app for the Simcline-V2 library.

## Files
- `Simcline_v2_5_2.aia`: The project file for [MIT App Inventor](https://appinventor.mit.edu/). Use this to modify or rebuild the app.
- `Simcline_v2_5_2.apk`: The pre-built Android app. Install this on your Android device to configure and simple control the SIMCLINE.

The Android app is built with BluetoothLE version: `edu.mit.appinventor.ble-20240822.aix`

## Instructions
- To install the `.apk`:
  1. Transfer the file to your Android device.
  2. Enable installation from unknown sources in your device settings.
  3. Open the file to install the app.
- To modify the app, import the `.aia` file into [MIT App Inventor](https://appinventor.mit.edu/).


