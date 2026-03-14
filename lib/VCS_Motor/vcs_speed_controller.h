#ifndef VCS_SPEED_CONTROLLER_H
#define VCS_SPEED_CONTROLLER_H

#include <Arduino.h>
#include "../VCS_Config/vcs_constants.h"

// Resets the PI controller's internal state (integrator)
void initSpeedController();

// Calculates the new PWM duty cycle (0-255) based on target and measured RPM
uint8_t updateSpeedController(float target_rpm, float measured_rpm);

#endif // VCS_SPEED_CONTROLLER_H