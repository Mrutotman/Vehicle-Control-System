#ifndef VCS_HALLSENSOR_H
#define VCS_HALLSENSOR_H

#include <Arduino.h>
#include "vcs_pins.h"
#include "vcs_constants.h"

// Core functions
void initHallSensors();
void updateHallCalculations(); // Called in the main loop

// Getters for other modules
float getMeasuredRPM();
uint32_t getTotalOdometer(); // Optional: pulses counted since boot

#endif // VCS_HALLSENSOR_H