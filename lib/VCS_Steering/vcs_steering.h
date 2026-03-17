

#ifndef VCS_STEERING_H
#define VCS_STEERING_H

#include <Arduino.h>
#include "../VCS_Config/vcs_pins.h"
#include "../VCS_Config/vcs_constants.h"

// Initialize steering pins and PID variables
void initSteering();

// Calculates and applies steering motor PWM based on target position (0-1000)
void updateSteeringPID(uint16_t target_position, bool is_automatic);

// Reads the current mapped position from the potentiometer (0-1000)
uint16_t getMeasuredSteering();

#endif // VCS_STEERING_H