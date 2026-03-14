/* ==============================================================================
 * MODULE:        VCS_Speed_Controller
 * RESPONSIBILITY: Speed PI control loop (1 kHz)[cite: 34].
 * DESCRIPTION:   The mathematical "brain" of the motor. It compares the Target 
 * RPM (from the ANS) against the Measured RPM (from VCS_Hall) 
 * to calculate the error. It uses Proportional (Kp) and Integral (Ki) 
 * gains to output a precise 0-255 PWM duty cycle. Includes 
 * anti-windup logic to freeze the integrator if saturated.
 * EXECUTION:     Must be executed exactly at 1 kHz (every 1000us) for the 
 * integral math to remain accurate[cite: 258].
 * ============================================================================== */

#include "vcs_speed_controller.h"

// Internal PI state
static float u_I = 0.0f; // Integral accumulator: u_I(k-1)

// Sample time calculation based on defined frequency
const float Ts = 1.0f / FREQ_SPEED_CTRL_HZ; // 1.0 / 1000 = 0.001 seconds

void initSpeedController() {
    u_I = 0.0f; // Reset integrator on startup or state change
}

uint8_t updateSpeedController(float target_rpm, float measured_rpm) {
    // 1. Calculate error: e(k) = w_ref(k) - w_meas(k)
    float e_k = target_rpm - measured_rpm;

    // 2. Proportional term: u_p(k) = Kp * e(k)
    float u_p = SPEED_KP * e_k;

    // 3. Tentative Integral term: u_I(k) = u_I(k-1) + Ki * e(k) * Ts
    float u_I_next = u_I + (SPEED_KI * e_k * Ts);

    // 4. Total output: u(k) = u_p(k) + u_I(k)
    float u_k = u_p + u_I_next;

    uint8_t final_duty_cycle = 0;

    // 5. Anti-windup and Saturation limits
    if (u_k > MAX_PWM_DUTY) {
        final_duty_cycle = MAX_PWM_DUTY;
        // Integrator u_I is NOT updated (frozen) to prevent windup
    } 
    else if (u_k < MIN_PWM_DUTY) {
        final_duty_cycle = MIN_PWM_DUTY;
        // Integrator u_I is NOT updated (frozen) to prevent windup
    } 
    else {
        // Within limits: apply calculated duty cycle and update integrator
        final_duty_cycle = (uint8_t)u_k;
        u_I = u_I_next; // Save u_I(k) for the next cycle
    }

    return final_duty_cycle;
}