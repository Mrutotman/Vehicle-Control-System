#ifndef VCS_HALL_H
#define VCS_HALL_H

#include <Arduino.h>
#include "../VCS_Config/vcs_pins.h"
#include "../VCS_Config/vcs_constants.h"

// Initializes pins and attaches hardware interrupts
void initHallSensors();

// Returns the current 3-bit Hall state (1-6)
uint8_t getHallState();

// Calculates and returns the low-pass filtered mechanical RPM
float getMechanicalRPM();

// Unified ISR for all Hall sensor transitions
void hallSensorISR();

#endif // VCS_HALL_H