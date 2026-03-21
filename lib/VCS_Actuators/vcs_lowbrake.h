#ifndef VCS_LOWBRAKE_H
#define VCS_LOWBRAKE_H

#include <Arduino.h>
#include "vcs_pins.h"
#include "vcs_constants.h"

// Core initialization and loop functions
void initLowBrake();
void updateLowBrake(); 

// Interfaces for the State Machine
void forceBrakeEngagement(bool engage); 
bool isPhysicalBrakePressed(); 

#endif // VCS_LOWBRAKE_H