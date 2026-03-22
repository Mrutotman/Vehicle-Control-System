#include "vcs_hallsensor.h"

// Volatile variables are required for data shared with Interrupt Service Routines (ISRs)
volatile uint32_t hall_pulse_count = 0;
uint32_t last_calc_time = 0;
float current_rpm = 0.0f;

// The ISR: Runs every time Phase U changes state (RISING or FALLING)
// REMOVED IRAM_ATTR: Not required/supported on Nano 33 BLE (Mbed OS)
void handleHallInterrupt() {
    hall_pulse_count++;
}

void initHallSensors() {
    // E-bike Hall sensors are usually open-collector. PULLUP is required 
    // to read the distinct square wave without floating noise.
    pinMode(PIN_HALL_U, INPUT_PULLUP);
    
    // Attach interrupt to Phase U using the correct Mbed OS mapping method.
    attachInterrupt(digitalPinToInterrupt(PIN_HALL_U), handleHallInterrupt, CHANGE);
    
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

        // 2. Math for RPM (Corrected for Single-Phase Tap)
        // If we tap ONE phase wire and trigger on CHANGE, we get 2 pulses per electrical rev.
        // One mechanical revolution = MOTOR_POLE_PAIRS * 2 Hall transitions.
        float pulses_per_rev = (float)MOTOR_POLE_PAIRS * 2.0f;
        
        // Apply Gear Reduction if your e-bike hub has internal planetary gears
        pulses_per_rev *= GEAR_REDUCTION; 
        
        // RPM = (Pulses / Pulses_Per_Rev) * (60,000ms / Elapsed_ms)
        if (pulses > 0) {
            current_rpm = ((float)pulses / pulses_per_rev) * (60000.0f / (float)elapsed);
        } else {
            current_rpm = 0.0f;
        }

        last_calc_time = now;
    }
}

float getMeasuredRPM() {
    // Linked directly to the v1.3 constant we defined earlier
    #if SIMULATION_MODE
        return getSimulatedRPM();
    #else
        return current_rpm;
    #endif
}