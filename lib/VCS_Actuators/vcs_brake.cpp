/* ==============================================================================
 * MODULE:        VCS_Brake
 * RESPONSIBILITY: Electronic Brake Actuation.
 * DESCRIPTION:   Manages the physical braking mechanism. It maps a percentage 
 * value (0-100%) into a PWM signal that controls the brake 
 * solenoid or motor. On system boot or during an E-Stop, this 
 * module defaults to 100% (Full Engagement) for safety.
 * HW RESOURCES:  PWM Output (PA7).
 * ============================================================================== */

#include "vcs_brake.h"

void initBrake() {
    pinMode(PIN_ESTOP_BRAKE, OUTPUT);
    
    // Engage brake fully on boot for safety
    applyBrake(100); 
}

void applyBrake(uint8_t percentage) {
    // Convert 0-100% command to 0-255 PWM duty cycle
    percentage = constrain(percentage, COMM_BRAKE_MIN, COMM_BRAKE_MAX);
    uint8_t pwm_val = map(percentage, 0, 100, MIN_PWM_DUTY, MAX_PWM_DUTY);
    
    analogWrite(PIN_ESTOP_BRAKE, pwm_val);
}