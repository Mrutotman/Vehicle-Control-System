#ifndef VCS_COMMUTATION_H
#define VCS_COMMUTATION_H

#include <Arduino.h>
#include "../VCS_Config/vcs_pins.h"
#include "../VCS_Config/vcs_constants.h"

// Initializes the motor pins
void initCommutation();

// Safety and timing functions
void allPhasesOff();
void applyDeadtime();

// Commutates the motor with a specific PWM duty cycle (0-255)
void commutatePhase(uint8_t phase, uint8_t pwm_duty);

#endif // VCS_COMMUTATION_H