#ifndef NIMBLE_MITM_H_
#define NIMBLE_MITM_H_

#include <Arduino.h>

// Include all NimBLE constants
#include "config/configNimBLE.h"
#include <NimBLEDevice.h>

class MITM {
private:
    // Constructor 
    MITM();
    static MITM* instance; // Singleton class instance
public:
    ~MITM();
    // Other members
    static MITM* getInstance();  // Singleton access method
    void start(void);
}; // Class MITM

#endif
