/* ==============================================================================
 * MODULE:        VCS_HallSensor
 * RESPONSIBILITY: RPM and Odometry calculation via parallel Hall tapping.
 * DESCRIPTION:   Uses Hardware Interrupts to count transitions on the motor's
 * Hall Phase wires. Calculates RPM based on pole pairs.
 * ============================================================================== */

#include "vcs_hallsensor.h"

// Volatile variables are required for data shared with Interrupt Service Routines (ISRs)
volatile uint32_t hall_pulse_count = 0;
uint32_t last_calc_time = 0;
float current_rpm = 0.0f;

// The ISR: This runs every time Phase U changes state
void IRAM_ATTR handleHallInterrupt() {
    hall_pulse_count++;
}

void initHallSensors() {
    pinMode(PIN_HALL_U, INPUT);
    
    // Attach interrupt to Phase U. 
    // CHANGE: triggers on any state change to maximize resolution
    attachInterrupt(digitalRead(PIN_HALL_U), handleHallInterrupt, CHANGE);
    
    last_calc_time = millis();
}

void updateHallCalculations() {
    uint32_t now = millis();
    uint32_t elapsed = now - last_calc_time;

    // Calculate RPM every 100ms for a balance of responsiveness and stability
    if (elapsed >= 100) {
        // 1. Atomically read and reset pulse count
        noInterrupts(); // Disable interrupts briefly to prevent data corruption
        uint32_t pulses = hall_pulse_count;
        hall_pulse_count = 0;
        interrupts();

        // 2. Math for RPM
        // One mechanical revolution = MOTOR_POLE_PAIRS * 6 Hall transitions
        float pulses_per_rev = (float)MOTOR_POLE_PAIRS * 6.0f;
        
        // RPM = (Pulses / Pulses_Per_Rev) * (60,000ms / Elapsed_ms)
        if (pulses > 0) {
            current_rpm = ((float)pulses / pulses_per_rev) * (60000.0f / (float)elapsed);
        } else {
            current_rpm = 0;
        }

        last_calc_time = now;
    }
}

float getMeasuredRPM() {
    #if SIMULATION_ENABLED
        return getSimulatedRPM();
    #else
        return current_rpm;
    #endif
}