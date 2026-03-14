#ifndef VCS_BRAKE_H
#define VCS_BRAKE_H

#include <Arduino.h>
#include "../VCS_Config/vcs_pins.h"
#include "../VCS_Config/vcs_constants.h"

// Initializes the brake PWM pin
void initBrake();

// Applies the electronic brake (0 = off, 100 = full brake)
void applyBrake(uint8_t percentage);

#endif // VCS_BRAKE_H