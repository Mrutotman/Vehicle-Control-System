#ifndef VCS_THROTTLE_H
#define VCS_THROTTLE_H

#include <Arduino.h>
#include "vcs_pins.h"
#include "vcs_constants.h"
#include <QuickPID.h>

// Variables exposed for telemetry and state machine
extern uint16_t current_throttle_adc;
extern uint8_t current_pwm_duty;

void initThrottle();
void updateThrottle(float measured_rpm, float target_rpm);

#endif // VCS_THROTTLE_H